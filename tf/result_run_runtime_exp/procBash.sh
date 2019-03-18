#!/bin/bash

sed '2,${/Starttime/d}' runP_1.log &> runP_1_proc.log
sed '2,${/Starttime/d}' runP_2.log &> runP_2_proc.log
sed '2,${/Starttime/d}' runP_3.log &> runP_3_proc.log
sed '2,${/Starttime/d}' runP_4.log &> runP_4_proc.log
sed '2,${/Starttime/d}' runP_5.log &> runP_5_proc.log

sed '2,${/Starttime/d}' runS_1.log &> runS_1_proc.log
sed '2,${/Starttime/d}' runS_2.log &> runS_2_proc.log
sed '2,${/Starttime/d}' runS_3.log &> runS_3_proc.log
sed '2,${/Starttime/d}' runS_4.log &> runS_4_proc.log
sed '2,${/Starttime/d}' runS_5.log &> runS_5_proc.log

python format_rtm_csv.py runP_1_proc.log &> runP_1.csv
python format_rtm_csv.py runP_2_proc.log &> runP_2.csv
python format_rtm_csv.py runP_3_proc.log &> runP_3.csv
python format_rtm_csv.py runP_4_proc.log &> runP_4.csv
python format_rtm_csv.py runP_5_proc.log &> runP_5.csv

python format_rtm_csv.py runS_1_proc.log &> runS_1.csv
python format_rtm_csv.py runS_2_proc.log &> runS_2.csv
python format_rtm_csv.py runS_3_proc.log &> runS_3.csv
python format_rtm_csv.py runS_4_proc.log &> runS_4.csv
python format_rtm_csv.py runS_5_proc.log &> runS_5.csv
