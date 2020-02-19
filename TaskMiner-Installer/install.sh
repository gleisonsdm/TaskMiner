#!/bin/bash
set -e 

# Create a directory to install TaskMiner and its test framework
THIS_INSTALL=$(pwd)
VERIFICATION=$(pwd | grep "TaskMiner-Installer")
if [ "${VERIFICATION}" == "" ]; then 
  if [ ! -d "../../TaskMiner_Dir" ]; then
    mkdir ../../TaskMiner_Dir
  fi
  cd ../../TaskMiner_Dir
  if [ ! -d "TaskMiner" ]; then
    cp -r ../TaskMiner .
  fi
  if [ ! -f "install.sh" ]; then
    cp -r ../TaskMiner/TaskMiner-Installer/* .
  fi
  CURR_SCRIPT=$(pwd)
  ((exec "sudo ${CURR_SCRIPT}/install.sh") ; (exit 0))
fi
if [ -d "../TaskMiner" ]; then
  rm -r "../TaskMiner"
fi

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

# Install Dependences
apt-get update
apt-get install apt-utils -y
apt-get upgrade -y
apt-get install libexpat1 -y
apt-get install -y make vim cmake git wget zip unzip
apt-get install -y gcc-6 g++-6
apt-get install -y libomp-dev
apt-get install -y python
apt-get install -y parallel

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

#------------------------------------------------------------------------------
#  Scripts to install the test Framework
#------------------------------------------------------------------------------

if [ ! -d "${ROOT_FOLDER}/tf" ]; then
  git clone -b taskminer https://github.com/guilhermeleobas/tf.git
fi

if [ ! -d "${ROOT_FOLDER}/Benchmarks" ]; then
  git clone https://github.com/lac-dcc/Benchmarks.git
fi

cd "${ROOT_FOLDER}/tf"
sed -i "s,\$HOME\/lge\/llvm-3.7-src\/build-debug,${ROOT_FOLDER}\/llvm-build,g" runAnalyzesTest.sh
sed -i "s,\$HOME\/lge\/taskminer\/build-debug,${ROOT_FOLDER}\/TaskMiner/lib,g" runAnalyzesTest.sh

sed -i "s,echo \"\$file_name\,broken\" >> \/home\/gleison\/tf\/report.csv,MARK1_REPLACE,g" annotate.sh
sed -i "s,MARK1_REPLACE,filename=\$(readlink -f \$file_name)\nMARK2_REPLACE,g" annotate.sh
sed -i "s,MARK2_REPLACE,filename=\"\${filename\/\/*Benchmarks\/}\"\nMARK3_REPLACE,g" annotate.sh
sed -i "s,MARK3_REPLACE,echo \"\$filename\,broken\" >> ${ROOT_FOLDER}\/tf\/report.csv,g" annotate.sh

sed -i "s,echo \"\$file_name\,annotated\" >> \/home\/gleison\/tf\/report.csv,MARK1_REPLACE,g" annotate.sh
sed -i "s,MARK1_REPLACE,filename=\$(readlink -f \$file_name)\nMARK2_REPLACE,g" annotate.sh
sed -i "s,MARK2_REPLACE,filename=\"\${filename\/\/*Benchmarks\/}\"\nMARK3_REPLACE,g" annotate.sh
sed -i "s,MARK3_REPLACE,echo \"\$filename\,annotated\" >> ${ROOT_FOLDER}\/tf\/report.csv,g" annotate.sh

sed -i "s,CLANG=\"\/home\/brenocfg\/Work\/llvm-3.7\/test\/bin\/clang\",CLANG=\"${ROOT_FOLDER}\/llvm-build\/bin\/clang\",g" scopetest.sh
sed -i "s,PLUGIN=\"\/home\/brenocfg\/Work\/llvm-3.7\/test\/lib\/scope-finder.so\",PLUGIN=\"${ROOT_FOLDER}\/llvm-build\/lib\/scope-finder.so\",g" scopetest.sh

sed -i "s,\$HOME\/tf\/scopetest.sh,${ROOT_FOLDER}\/tf\/scopetest.sh,g" runAnalyzesTest.sh 
sed -i "s,export PRA=\"\$BUILD\/PtrRangeAnalysis\/libLLVMPtrRangeAnalysis.so\",export PRA=\"\$BUILD\/PtrRangeAnalysis\/libLLVMPtrRangeAnalysis.so\"\nexport PAT=\"\$BUILD\/PtrAccessType\/libLLVMPtrAccessTypeAnalysis.so\",g" runAnalyzesTest.sh

sed -i "s,export OMP=\"\-I\/home\/kezia\/2015\/openmp\/runtime\/exports\/common\/include\",#export OMP=\"\-I\/home\/kezia\/2015\/openmp\/runtime\/exports\/common\/include,g" runAnalyzesTest.sh
sed -i "s,export PATH=\$PATH:\/home\/periclesrafael\/openmp4\/llvm\/install\/bin\/,#export PATH=\$PATH:\/home\/periclesrafael\/openmp4\/llvm\/install\/bin\/,g" runAnalyzesTest.sh
sed -i "s,export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\/home\/periclesrafael\/openmp4\/llvm\/install\/lib,#export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\/home\/periclesrafael\/openmp4\/llvm\/install\/lib,g" runAnalyzesTest.sh
sed -i "s,export C_INCLUDE_PATH=\$C_INCLUDE_PATH:\/home\/periclesrafael\/openmp4\/llvm\/install\/include,#export C_INCLUDE_PATH=\$C_INCLUDE_PATH:\/home\/periclesrafael\/openmp4\/llvm\/install\/include,g" runAnalyzesTest.sh
sed -i "s,\$OPT -load \$ST -load \$WTM -load \$WAI,\$OPT -load \$PAT -load \$ST -load \$WTM -load \$WAI,g" runAnalyzesTest.sh 

cd "${ROOT_FOLDER}"
