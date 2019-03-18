bench_name="strassen"

source_files=( "bots_common.c" "strassen.c" "bots_main.c" )
COMPILE_FLAGS=" -lm "
RUN_OPTIONS=" -n 4096 -v 2 -o 1 -c &> strassen.output "


