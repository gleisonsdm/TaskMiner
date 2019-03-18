#!/bin/bash

ulimit -s unlimited
rm /home/gleison/tf/stats.csv
rm /home/gleison/tf/run.txt
rm /home/gleison/tf/run.log
rm /home/gleison/tf/report.csv
rm /home/gleison/tf/report_outputs.csv
RUNTIME=30m JOBS=1 COMPILE=1 ANNOTATE=1 TASKMINER=1 EXEC=0 DIFF=0 ./run.sh 
#RUNTIME=30m JOBS=1 COMPILE=1 ANNOTATE=1 TASKMINER=1 EXEC=1 DIFF=1 ./run.sh
#RUNTIME=10m JOBS=1 COMPILE=1 ANNOTATE=0 TASKMINER=0 EXEC=1 DIFF=1 ./run.sh
