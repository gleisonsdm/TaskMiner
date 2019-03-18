#!/bin/bash

# Command line parameters:
DIFF="diff"
PROG=$1
GOODOUTPUT=$2
TESTOUTPUT=$3
# Output filename:
DIFFOUTPUT=/tmp/${PROG}.diff

# Diff the two files.
$DIFF $GOODOUTPUT $TESTOUTPUT > $DIFFOUTPUT 2>&1

if [[ $? -eq 0 ]]; then
  echo ""
  echo "******************** TEST '$PROG' SUCCEED! ********************"
  echo "$PROG,succeed" >> /home/gleison/tf/report_outputs.csv
else
  # They are different!
  echo ""
  echo "******************** TEST '$PROG' FAILED! ********************"
  echo "Execution Context Diff:"
  echo "$PROG,failed" >> /home/gleison/tf/report_outputs.csv
  head -n 10 $DIFFOUTPUT | cat -v
  # rm $DIFFOUTPUT
  exit 1
fi
