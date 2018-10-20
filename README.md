# TaskMiner - A Source to Source Compiler for Parallelizing Irregular C/C++ Programs with Code Annotation

[Project Webpage](http://cuda.dcc.ufmg.br/taskminer/)

[Code Repository](https://github.com/gleisonsdm/TaskMiner/)

## Introduction

### Motivation

Nowadays, the high performance of computers and smartphones is one of the most important aspects to users. On the other hand, the demands of information are growing day by day and, consequently, the programs need to process larger sets of data. The problem is that people would like to process more data using less time, and the industry cannot improve more the hardware as before. One of the most promises solutions to solve this kind of problem is use more CPU cores, trying to spend less time processing different pieces of data at the same time.

Unfortunately, a large portion of the apps and programs that people use each day was not structurized to use all the processing power available. People have powerful equipments, but they cannot obtain a large speedup, due the major part of the programers are not friendly with the required specificities to process data in parallel. 

Recently, programmers have an important ally, some metalanguages as OpenACC and OpenMP. These metalanguages can simplify the steps to write high performance programs, as they provide some level of abstraction, and helps to structurize the correct execution order of functions in programs. Even though this kind of abstraction, programmers still needing to define the parallel regions, and their data dependences, directive-based models don't find data dependences, they just organize them.

These types of programming models have his shortcomings, as is necessary to use specific libraries to write programs that sectorize the data and process them concurrently. And, even if the programmer is adapted to explore the computer especificities and its aspects, identify the data dependencies in the program is a hard task, subject to errors. 

### Implementation

We have developed TaskMiner as a tool to automate the performance of these tasks. Through the implementation of a static analysis that derives memory access bounds from source code, it infers the size of memory regions in C and C++ programs. With these bounds, it is capable of inserting data copy directives in the original source code. These directives provide a compatible compiler with information on which data must be used in each portion of the program. Given the source code of a program as input, our tool can then provide the user with a modified version containing directives with a mapping of regions that are attractive to parallelize, proper their memory bounds specified, all without any further intervention from the user, effectively freeing developers from the burdensome task of manual code modification. 

We implemented TaskMiner as a collection of compiler modules, or passes, for the LLVM compiler infrastructure, whose code is available in this repository.

## Functionality

### Compiler Directive Standards

Compiler directive-oriented programming standards are some of the newest developments in features for parallel programming. These standards aim to simplify the creation of parallel programs by providing an interface for programmers to indicate specific regions in source code to be run as parallel. Parallel execution has application in several different hardware settings, such as multiple processors in a multicore architecture, or offloading to a separate device in a heterogeneous system. Compilers that support these standards can check for the presence of directives (also known as pragmas) in the source code, and generate parallel code for the specific regions annotated, so they can be run on a specified target device. TaskMiner currently supports OpenMP standard, but it can easily be extended to support others. You can read more on the subject in the links below:

[OpenMP](http://openmp.org/openmp-faq.html)

In order to use this standard to offload execution to accelerators, it is necessary to compile the modified source code with a compiler that supports the given directive standard (OpenMP 4.0). In our internal testing environment, we use gcc-6 compiler for OpenMP support. You can find out more about it in the following link:

[GCC 5+](https://gcc.gnu.org/wiki/openmp) - Starting from version 5.0, GCC provides support for computation offloading through OpenMP 4.0 directives.

[Portland Group](http://www.pgroup.com/index.htm) - There are other compilers that provide support for OpenMP 4.0, either as fully-supported features or as experimental implementations. Below is a small list of such compilers:

[OpenMP Clang](http://openmp.llvm.org/) - The OpenMP runtime Clang implementation has been officially moved to an LLVM subproject. Currently supports offloading to accelerators using OpenMP 4.0 directives.

[Pathscale](http://www.pathscale.com/) - Pathscale's EKOPath compiler suite supposedly supports offloading with OpenMP 4.0+, as well as other annotation standards.

Note that, since most implementations are premiliminary and tend to change considerably, the annotation syntax inserted by TaskMiner, while standard compliant, might not be fully compatible with each compiler's implementation. If you attempt to use a compiler that provides support for these standards but does not compile the annotation format TaskMiner uses, we would appreciate knowing about it!

## Installation

The project is structured as a set of dynamically loaded libraries/passes for LLVM that can be built separately from the main compiler. However, an existing LLVM build (compiled using cmake) is necessary to build our code. 

You can download and build it by downloading [LLVM](http://llvm.org/releases/3.7.0/llvm-3.7.0.src.tar.xz) and [Clang](http://llvm.org/releases/3.7.0/cfe-3.7.0.src.tar.xz), then:

    Extract their contents to llvm and llvm/tools/clang.

    Download the TaskMiner source and apply the patch "llvm-patch.diff" to your LLVM source directory, that is located in 'taskminer/ArrayInference/llvm-patch.diff'.

    After applying the diff, we can move on to compiling a fresh LLVM+Clang 3.7 build. To do so, you can follow these outlines:

    	MAKEFLAG="-j8"
      
     	LLVM_SRC=<path-to-llvm-source-folder>
    	REPO=<path-to-taskminer-repository>

    	#We will build a debug version of LLVM+Clang under ${LLVM_SRC}/../llvm-build
    	mkdir ${LLVM_SRC}/../llvm-build
    	cd ${LLVM_SRC}/../llvm-build

    	#Setup clang plugins to be compiled alongside LLVM and Clang
    	${REPO}/src/ScopeFinder/setup.sh

    	#Create build setup for LLVM+Clang using CMake
    	cmake -DCMAKE_BUILD_TYPE=debug -DBUILD_SHARED_LIBS=ON ${LLVM_SRC}
    	
    	#Compile LLVM+Clang (this will likely take a while)
    	make ${MAKEFLAG}
    	cd -

    After you get a fresh LLVM build under ${LLVM_BUILD_DIR}, the following commands can be used to build TaskMiner:

    	LLVM_BUILD_DIR=<path-to-llvm-build-folder> 	
    	REPO=<path-to-repository>

     	# Build the shared libraries under ${REPO}/lib, assumming an existing LLVM
     	# build under ${LLVM_BUILD_DIR}
     	mkdir ${REPO}/lib
     	cd ${REPO}/lib
     	cmake -DLLVM_DIR=${LLVM_BUILD_DIR}/share/llvm/cmake ../src/
     	make
    	cd -


## How to run a code

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

- TaskMiner/lib : A reference to the location of the DawnCC libraries (.so files). 

- Source Code : The input file that will be used to run the analyses. 

- op1 => An integer that defines the acceptable runtime cost.
  
- op2 => Flag "-debug-only=print-tasks": Flag used to print tasks, if necessary (Optional).

- op3 => Flag "-stats": Flag necessary to debug the analyze (Optional).
    
