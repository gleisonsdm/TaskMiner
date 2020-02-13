#!/bin/bash
set -e 

#You need wget, tar, unzip, cmake and a toolchain to use this script
#Put this on a root folder, than it will download all tarballs required and build
#After running, you will have something like
#
#./
#├── build.sh
#├── cfe-3.7.0.src.tar.xz
#├── TaskMiner
#├── TaskMiner-Compiler-master.zip
#├── llvm
#├── llvm-3.7.0.src.tar.xz
#└── llvm-build

#Number of threads to build
MAKE_THREADS=8

#Clang and LLVM versions
LLVM_VER="3.7.0"
CLANG_VER="3.7.0"

#TaskMiner root path plus Clang and LLVM source folders
ROOT_FOLDER=`pwd`
TKMN_PATH="${ROOT_FOLDER}/TaskMiner"
LLVM_SRC="${ROOT_FOLDER}/llvm"
CLANG_SRC="${LLVM_SRC}/tools/clang"
LLVM_OUTPUT_DIR="${ROOT_FOLDER}/llvm-build"

#Tarball names
LLVM_SRC_FILE="llvm-${LLVM_VER}.src.tar.xz"
CLANG_SRC_FILE="cfe-${CLANG_VER}.src.tar.xz"
TKMN_SRC_FILE="TaskMiner"

#Tarball websites
LLVM_SRC_ADDR="http://llvm.org/releases/${LLVM_VER}/${LLVM_SRC_FILE}"
CLANG_SRC_ADDR="http://llvm.org/releases/${CLANG_VER}/${CLANG_SRC_FILE}"
TKMN_SRC_ADDR="https://github.com/gleisonsdm/TaskMiner.git"

#Download LLVM, Clang and TaskMiner source tarballs if not already downloaded
if [ ! -f "${LLVM_SRC_FILE}" ]; then
    wget "${LLVM_SRC_ADDR}"
fi

if [ ! -f "${CLANG_SRC_FILE}" ]; then
    wget "${CLANG_SRC_ADDR}"
fi

#If downloaded tarballs were not extracted, then extract
if [ ! -d "${TKMN_PATH}" ]; then
    git clone "${TKMN_SRC_ADDR}"
fi

if [ ! -d "${LLVM_SRC}" ]; then
    mkdir "${LLVM_SRC}"
    tar -Jxf "llvm-${LLVM_VER}.src.tar.xz" -C "${LLVM_SRC}" --strip 1

    #Apply TaskMiner patch into LLVM source
    cd "${LLVM_SRC}"
    patch -p1 < "${TKMN_PATH}/src/ArrayInference/llvm-patch.diff"
    cd "${ROOT_FOLDER}"
fi

if [ ! -d "${CLANG_SRC}" ]; then
    mkdir "${CLANG_SRC}"
    tar -Jxf "cfe-${CLANG_VER}.src.tar.xz" -C "${CLANG_SRC}" --strip 1 #extract clang tarball to llvm/tools folder
fi

#Create output folder for LLVM if not already created
if [ ! -d "${LLVM_OUTPUT_DIR}" ]; then
    mkdir ${LLVM_OUTPUT_DIR}
fi

#Setup LLVM+Clang and scope-finder plugin if not already setup
EXTRA_FOLDER="${LLVM_SRC}/tools/clang/tools/extra"

if [ ! -f "${EXTRA_FOLDER}" ]; then
    mkdir -p "${EXTRA_FOLDER}"
    echo "add_subdirectory(scope-finder)" > ${EXTRA_FOLDER}/CMakeLists.txt
    cp -rf ${TKMN_PATH}/src/ScopeFinder/scope-finder ${EXTRA_FOLDER}/.
fi

#Create setup with cmake if not already created
cd ${LLVM_OUTPUT_DIR}

if [ ! -f "Makefile" ]; then
    CXX=g++-5 cmake -DCMAKE_BUILD_TYPE=debug -DBUILD_SHARED_LIBS=ON ${LLVM_SRC}
fi

#Prebuild clang to workaround TaskMiner build problems
make clang -j${MAKE_THREADS} 

#Build LLVM then go back to root folder
make -j${MAKE_THREADS}
cd ${ROOT_FOLDER}

#Create lib folder of TaskMiner if not already created
if [ ! -d "${TKMN_PATH}/lib" ]; then
    mkdir ${TKMN_PATH}/lib 
fi

#Navigate to TaskMiner output directory, run if theres no makefile, and then build TaskMiner
cd ${TKMN_PATH}/lib
if [ ! -f "Makefile" ]; then
    CXX=g++-5 cmake -DLLVM_DIR=${LLVM_OUTPUT_DIR}/share/llvm/cmake ../src
fi
make -j${MAKE_THREADS}

#Go back to root folder
cd ${ROOT_FOLDER}
