#!/bin/bash
THIS=$(pwd)
cd "${THIS}/tf"
BENCHSDIR="${THIS}/Benchmarks" TASKMINER=1 ANNOTATE=1 COMPILE=0 EXECUTE=0 ./run.sh 
cd "${THIS}"
