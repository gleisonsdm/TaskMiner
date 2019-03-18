#!/bin/bash
sed -n '/\/home\/gleison\/tf\/Benchmarks/p' /tmp/tempao.txt | head -1 &> tmpout.txt
sed -i 's/tmp.c*//' tmpout.txt
sed -i 's/^.*Benchmarks*//' tmpout.txt
v0="$(cat tmpout.txt)"
echo -n ${v0} >> /home/gleison/tf/stats.csv 
echo -n ',' >> /home/gleison/tf/stats.csv

sed -n '/Number of function calls/p' /tmp/tempao.txt | head -1 &> tmpout.txt
sed -i 's/writeInFile - Number of function calls//g' tmpout.txt
sed -i 's/[ \t]*$//' tmpout.txt 
v1="$(cat tmpout.txt)"
echo -n ${v1} >> /home/gleison/tf/stats.csv 
echo -n ',' >> /home/gleison/tf/stats.csv

sed -n '/Number of pragmas inserted/p' /tmp/tempao.txt | head -1 &> tmpout.txt
sed -i 's/recoverExpressions - Number of pragmas inserted*//' tmpout.txt
sed -i 's/[ \t]*$//' tmpout.txt 
v2="$(cat tmpout.txt)"
echo -n ${v2} >> /home/gleison/tf/stats.csv 
echo -n ',' >> /home/gleison/tf/stats.csv

sed -n '/Number of functions annotated/p' /tmp/tempao.txt | head -1 &> tmpout.txt
sed -i 's/recoverExpressions - Number of functions annotated*//' tmpout.txt
sed -i 's/[ \t]*$//' tmpout.txt 
v3="$(cat tmpout.txt)"
echo -n ${v3} >> /home/gleison/tf/stats.csv 
echo -n ',' >> /home/gleison/tf/stats.csv
echo '' >> /home/gleison/tf/stats.csv
