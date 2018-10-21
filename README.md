# TaskMiner - Automatic Annotation of C Programs with OpenMP

## Description

TaskMiner is a tool that annotates C
programs with OpenMP directives to support task parallelism.
Details are available in the paper [Automatic Identification of Tasks in
Structured Programs](http://homepages.dcc.ufmg.br/~fernando/publications/papers/PACT18.pdf). This tool is implemented in C++, on top of LLVM 3.7, and it is
publicly available through an [on-line interface](http://cuda.dcc.ufmg.br/taskminer/).

## Installating

The project is structured as a set of dynamically loaded libraries/passes for
LLVM.
These libraries can be built separately from the main compiler.
However, an existing LLVM build (compiled using cmake) is necessary to build
our code. 

1. Extract [LLVM 3.7](http://llvm.org/releases/3.7.0/llvm-3.7.0.src.tar.xz) into
llvm

2. Extract [Clang](http://llvm.org/releases/3.7.0/cfe-3.7.0.src.tar.xz) into
llvm/tools/clang.

3. Download the TaskMiner source code.

4. Apply the patch "llvm-patch.diff" (located at 'taskminer/ArrayInference/llvm-patch.diff') to your LLVM source directory.

5. Compile  a fresh LLVM+Clang 3.7 build:

    	MAKEFLAG="-j8"
      
     	LLVM_SRC=<path-to-llvm-source-folder>
    	REPO=<path-to-taskminer-repository>

    	#Build a debug version of LLVM+Clang under ${LLVM_SRC}/../llvm-build
    	mkdir ${LLVM_SRC}/../llvm-build
    	cd ${LLVM_SRC}/../llvm-build

    	#Setup clang plugins to be compiled alongside LLVM and Clang
    	${REPO}/src/ScopeFinder/setup.sh

    	#Create build setup for LLVM+Clang using CMake
    	cmake -DCMAKE_BUILD_TYPE=debug -DBUILD_SHARED_LIBS=ON ${LLVM_SRC}
    	
    	#Compile LLVM+Clang (this will likely take a while)
    	make ${MAKEFLAG}
    	cd -

6. Build TaskMiner:

    	LLVM_BUILD_DIR=<path-to-llvm-build-folder> 	
    	REPO=<path-to-repository>

     	# Build the shared libraries under ${REPO}/lib
     	# build under ${LLVM_BUILD_DIR}
     	mkdir ${REPO}/lib
     	cd ${REPO}/lib
     	cmake -DLLVM_DIR=${LLVM_BUILD_DIR}/share/llvm/cmake ../src/
     	make
    	cd -


## Running

To run TaskMiner, you can run the run.sh bash script, passing as arguments the directory which llvm-build and TaskMiner are located and a directory containing source files to be processed. Arguments can be passed in command line to change behaviour of the script.
    
    ./run.sh -d <root folder> -src <folder with files to be processed> 

Or you can run TaskMiner by copying and pasting the text below into a shell script file. You will have to change text between pointy brackets, e.g., *< like this >* to adapt the script to your environment.

 	LLVM_PATH="<root folder>/llvm-build/bin"

 	export CLANG="$LLVM_PATH/clang"
 	export CLANGFORM="$LLVM_PATH/clang-format"
 	export OPT="$LLVM_PATH/opt"
	export LINKER="$LLVM_PATH/llvm-link"
	export DIS="$LLVM_PATH/llvm-dis"

	export SCOPEFIND="$LLVM_PATH/../lib/scope-finder.so"

 	export BUILD=< TaskMiner/lib >

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

	export XCL="-Xclang -load -Xclang"
	export FLAGS="-mem2reg -tbaa -scoped-noalias -basicaa -functionattrs -gvn -loop-rotate
 	-instcombine -licm"
 	export FLAGSAI="-mem2reg -loop-rotate"

 	rm result.bc result2.bc

 	$CLANGFORM -style="{BasedOnStyle: llvm, IndentWidth: 2}" -i < Source Code File(s) (.c/.cc/.cpp)>

 	$CLANG -Xclang -load -Xclang $SCOPEFIND -Xclang -add-plugin -Xclang -find-scope -g -O0 -c -fsyntax-only < Source Code File(s) (.c/.cc/.cpp)>

 	$CLANG $OMP -g -S -emit-llvm < Source Code > -o result.bc 

 	$OPT -load $ST -instnamer -mem2reg -scopeTree result.bc 

 	$OPT -load $ST -load $WTM -load $WAI -instnamer -mem2reg -loop-simplify -writeInFile -Run-Mode=true \
             -RUNTIME_COST=<op1> <op2> <op3> -S result.bc -o result2.bc

 	$CLANGFORM -style="{BasedOnStyle: llvm, IndentWidth: 2}" -i < Source Code Files (.c/.cc/.cpp) >

Below, a summary of each part where it is necessary to change text:

- path-to-llvm-build-bin-folder : A reference to the location of the llvm-3.7 binaries. 

- TaskMiner/lib : A reference to the location of the TaskMiner libraries (.so files). 

- Source Code : The input file that will be used to run the analyses. 

- op1 => An integer that defines the acceptable runtime cost.
  
- op2 => Flag "-debug-only=print-tasks": Flag used to print tasks, if necessary (Optional).

- op3 => Flag "-stats": Flag necessary to debug the analyze (Optional).
    
