#!/bin/bash

LLVM_PATH="${1}/bin"

export CLANG="$LLVM_PATH/clang"
export CLANGFORM="$LLVM_PATH/clang-format"
export OPT="$LLVM_PATH/opt"
export LINKER="$LLVM_PATH/llvm-link"
export DIS="$LLVM_PATH/llvm-dis"

export SCOPEFIND="$LLVM_PATH/../lib/scope-finder.so"

export BUILD="${2}"

export PRA="$BUILD/PtrRangeAnalysis/libLLVMPtrRangeAnalysis.so"
export AI="$BUILD/AliasInstrumentation/libLLVMAliasInstrumentation.so"
export DPLA="$BUILD/DepBasedParallelLoopAnalysis/libParallelLoopAnalysis.so"
export DLM="$BUILD/DivergentLoopMetadata/libDivergentLoopMetadata.so"
export CP="$BUILD/CanParallelize/libCanParallelize.so"
export PLM="$BUILD/ParallelLoopMetadata/libParallelLoopMetadata.so"
export WAI="$BUILD/ArrayInference/libLLVMArrayInference.so"
export CDA="$BUILD/ControlDivergenceAnalysis/libControlDivergenceAnalysis.so"
export ST="$BUILD/ScopeTree/libLLVMScopeTree.so"
export WTM="$BUILD/libLLVMTaskFinder.so"
export PTRA="$BUILD/PtrAccessType/libLLVMPtrAccessTypeAnalysis.so" 

export XCL="-Xclang -load -Xclang"
export FLAGS="-mem2reg -tbaa -scoped-noalias -basicaa -functionattrs
-gvn -loop-rotate
-instcombine -licm"
export FLAGSAI="-mem2reg -loop-rotate"

rm result.bc result2.bc

$CLANGFORM -style="{BasedOnStyle: llvm, IndentWidth: 2}" -i "${3}"

$CLANG -Xclang -load -Xclang $SCOPEFIND -Xclang -add-plugin -Xclang
-find-scope -g -O0 -c -fsyntax-only "${3}"

$CLANG $OMP -g -S -emit-llvm "${3}" -o result.bc

$OPT -load $ST -instnamer -mem2reg -scopeTree result.bc

$OPT -load $PTRA -load $ST -load $WTM -load $WAI -instnamer -mem2reg
-loop-simplify -writeInFile -Run-Mode=true \
         -RUNTIME_COST="${4}" "${5}" "${6}" -S result.bc -o result2.bc

$CLANGFORM -style="{BasedOnStyle: llvm, IndentWidth: 2}" -i ${3}
