#!/bin/bash

# -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- 

trap 'echo "Killing build_exec.sh script" ; exit' INT TERM

# -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- 

function cleanup() {
  rm -f *.bc
  rm -f *.rbc ;
  rm -f *.ibc ;
  rm -f *.o ;
  if [[ $ANNOTATE -eq 1 ]]; then
    # delete the _AI.c only when we're annotating the benchmark
    rm -f *_AI.c
    rm -f *_AI.h
    rm -f *c_scope.dot
    rm -f taskminer*.out
  fi
  rm -f report.txt
  # rm -f *.exe ;
  # rm -f table.csv
  # rm -f prof.dat ;
  # rm -f feat.dat ;
}

function cleanup_all() {
  rm -f *.bc
  rm -f *.rbc 
  rm -f *.ibc
  rm -f *.o
  rm -f *.exe
  rm -f *.txt
}

function unset_vars() {
  unset COMPILER ;
  unset STDIN ;
  unset STDOUT ;
  unset RUN_OPTIONS ;

  unset CPU2006 ;

}

function set_vars(){
  source info.sh
  COMP_FLAGS=$CXXFLAGS
  export COMP_FLAGS

  # Let's set the variables that are unset

  # sometimes we need to use clang++
  [[ -n $COMPILER ]] || COMPILER=clang ;

  # We can specify STDIN to something other than /dev/stdin
  [[ -n $STDIN ]] || STDIN=/dev/stdin ;
  # And STDOUT default is /dev/null. 
  [[ -n $STDOUT ]] || STDOUT=/dev/null ;
  # But if we set DEBUG=1, than we ignore the previous definition of STDOUT
  if [[ $DEBUG == 1 ]]; then
    STDOUT=/dev/stdout ;
  fi

  if [[ $(pwd) =~ "cpu2006" ]]; then
    echo "Setting CPU2006=1"
    CPU2006=1
  fi

  # Common files used by comp.sh and instrument.sh
  if [[ -n $CPU2006 && $CPU2006 -eq 1 && $(uname -s) == "Linux" ]]; then
    rbc_name="$bench_name.linux"
  else
    rbc_name="$bench_name.llvm"
  fi

  lnk_name="$bench_name.rbc"
  prf_name="$bench_name.ibc"
  obj_name="$bench_name.o"
  exe_name="$bench_name.exe"

  # options for exe name
  if [[ -n $INSTRUMENT && $INSTRUMENT -eq 1 ]]; then
    exe_name=INS_$exe_name
  fi
  
  if [[ -n $ASAN && $ASAN -eq 1 ]]; then
    exe_name=ASAN_$exe_name
  fi
  
  if [[ $SSA -eq 0 ]]; then
    exe_name=NO_SSA_$exe_name ;
  fi

  # if we're on osx, we must use `gtimeout` instead of `timeout`
  # `gtimeout` can be download from homebrew
  TIMEOUT=timeout
  if [[ $(uname -s) == "Darwin" ]]; then
    TIMEOUT=gtimeout
  fi
}

# -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- 

function walk() {

  if [[ $# == 0 ]]; then
    echo "Error, you must specify the directories this script must compile"
    echo 'ex: walk $( ls -d */ )'
    exit ;
  else
    dirs=("$@") ;
  fi

  parent_dir=$(pwd) ;

  for dir in "${dirs[@]}"; do
    cd "$parent_dir"/"$dir" ;

    # # Let's remove all _AI.c files before
    rm -f tmp.c

    d=$(basename $(pwd))
    echo "Sourcing info.sh from $d" ;
    
    set_vars ;
    cleanup ;

    if [[ $ANNOTATE -eq 1 ]]; then
      annotate ;
      if [[ $? -ne 0 ]]; then
        echo 
        echo "###############"
        echo
        continue ;
      fi
    fi

    if [[ $COMPILE -eq 1 ]]; then
      compile ;
      if [[ $? -ne 0 ]]; then
        echo 
        echo "###############"
        echo
        continue ;
      fi
    fi

    execute ;

    unset_vars ;
    
    echo 
    echo "###############"
    echo

    cd "$parent_dir"

  done
}

# -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- 

source "vars.sh"
source "benchs.sh"
source "comp.sh"
source "exec.sh"

if [[ -n $CLEAN && $CLEAN -eq 1 ]]; then
  echo "REMOVING ALL TEMP FILES!"
  
  for bench in "${benchs[@]}"; do
    cd $BENCHSDIR
    echo "Removing from $bench" ;
    cd $bench ;
    $bench ;
    echo "" ;
  done

  exit 0
fi

if [[ -n $PIN && $PIN -eq 1 ]]; then
  # replace the function `execute`
  source "exec_pin.sh"
fi

if [[ -n $ANNOTATE && $ANNOTATE -eq 1 ]]; then
  # Create the _AI.c files
  source "annotate.sh"
fi

if [[ -n $TASKMINER && $TASKMINER -eq 1 ]]; then
  # replace the comp function
  source "comp_taskminer.sh"
fi

rm -f run.txt
touch run.txt

if [[ "$#" -ne 0 ]]; then
  # check if the input is a file
  if [[ -f "$@" ]]; then
    echo "Reading input file..."
    # Read the content into "${benchs[@]}" array
    IFS=$'\n' read -d '' -r -a benchs < "$@"
  else
    benchs=( "$@" )
  fi

  walk "${benchs[@]}" ;

else
  for bench in "${benchs[@]}"; do
    cd $BENCHSDIR
    echo "Starting $bench" ;
    cd $bench ;
    $bench ;
  done
fi

cd $BASEDIR ;

source "parallel.sh"
source "collect.sh"


