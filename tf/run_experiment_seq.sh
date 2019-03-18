#!/bin/bash

rm /home/gleison/tf/run.txt
rm /home/gleison/tf/run.log
rm /home/gleison/tf/report.csv
rm /home/gleison/tf/report_outputs.csv
RUNTIME=10m JOBS=1 COMPILE=1 ANNOTATE=0 TASKMINER=0 EXEC=1 DIFF=1 ./run.sh
#RUNTIME=10m JOBS=1 COMPILE=1 ANNOTATE=0 TASKMINER=0 EXEC=1 DIFF=1 ./run.sh
