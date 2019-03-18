#!/bin/bash

sed -i "s/ writeInFile - Number of function calls//g" stats.csv 
sed -i "s/,,/,0,/g" stats.csv 
sed -i "s/,,/,0,/g" stats.csv 
sed -i "s/,,/,0,/g" stats.csv 
sed -i "/,0,0,0,/d" stats.csv
sed -i "s/Game\//\/BenchmarkGame\//g" stats.csv


