# TaskMiner - Automatic Annotation of C with OpenMP

## Description

TaskMiner is a tool that annotates C
programs with OpenMP directives to support task parallelism.
Details are available in the paper [Automatic Identification of Tasks in
Structured Programs](http://homepages.dcc.ufmg.br/~fernando/publications/papers/PACT18.pdf). This tool is implemented in C++, on top of LLVM 3.7, and it is
publicly available through an [on-line interface](http://cuda.dcc.ufmg.br/taskminer/).

## Installating

### Automatic Installating

This project contains scripts build Taskminer on unix systems. It was designed to install this compiler on Ubuntu. It is possible to use those scripts on different operating systems in case the user adapts them.

#### Warning

This project contains scripts to install software and update your system. Please, take a look at the bash scripts before running it, case you wanna customize the installation process.

### How to install TaskMiner using the scripts

Firstly, define the TaskMiner path into your system. Notice that you need to replace <path to taskminer>
with the directory you want to install TaskMiner. 

```sh
export TASKMINER_DEST=<path of taskminer>
```

After that, type the following commands:

```sh
git clone https://github.com/gleisonsdm/TaskMiner.git
cd TaskMiner/TaskMiner-Installer
cp install.sh TASKMINER_DEST/.
cp execute_tests.sh TASKMINER_DEST/.
cd ../../
rm -r TaskMiner
cd TASKMINER_DEST
sudo bash install.sh
```

#### How to use TaskMiner test framework to annotate programs:


Firstly, define the TaskMiner path into your system. Notice that you need to replace <path to taskminer>
with the directory you want to install TaskMiner. 

```sh
export TASKMINER_DEST=<path of taskminer>
```

After that, type the following commands:

```sh
cd TASKMINER_DEST
sudo bash execute_tests.sh
```

### Manual Installating

The project is structured as a set of dynamically loaded libraries/passes for
LLVM.
These libraries can be built separately from the main compiler.
However, an existing LLVM build (compiled using cmake) is necessary to build
our code. 


1. Extract [LLVM 3.7](http://llvm.org/releases/3.7.0/llvm-3.7.0.src.tar.xz) into
llvm

2. Extract [Clang](http://llvm.org/releases/3.7.0/cfe-3.7.0.src.tar.xz) into
llvm/tools/clang.

3. Download the [TaskMiner](https://github.com/gleisonsdm/TaskMiner) source code.

4. Apply the patch "llvm-patch.diff" (located at '/src/ArrayInference/llvm-patch.diff') to your LLVM source directory.

        patch -p1 < "${TASKMINER_PATH}/src/ArrayInference/llvm-patch.diff"

5. Compile  a fresh LLVM+Clang 3.7 build:

    	MAKEFLAG="-j8"
      
     	LLVM_SRC=<path-to-llvm-source-folder>
    	REPO=<path-to-taskminer-repository>

    	#Build a debug version of LLVM+Clang under ${LLVM_SRC}/../llvm-build
    	mkdir ${LLVM_SRC}/../llvm-build
    	cd ${LLVM_SRC}/../llvm-build

    	#Setup clang plugins to be compiled alongside LLVM and Clang
    	${REPO}/src/ScopeFinder/setup.sh ${LLVM_SRC}

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

We have written a [script](https://github.com/gleisonsdm/TaskMiner/blob/master/src/run.sh) to run TaskMiner.
To use it, adjust its flags to your environment, and then do:
 
  	./run.sh <path-to-llvm-build-bin-folder> <TaskMiner/lib> <Source_Code> <op1>

The following parameters are customizable:

* path-to-llvm-build-bin-folder: location of the llvm-3.7 binaries. 
* TaskMiner/lib: location of the TaskMiner libraries (.so files). 
* Source-Code: input files that will be analyzed. 
* op1: integer specifying the acceptable runtime cost.
