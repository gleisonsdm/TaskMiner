bench_name="sort"

source_files=( "bots_common.c" "sort.c" "bots_main.c" )
COMPILE_FLAGS=" -lm "
RUN_OPTIONS=" -n 33554432 -v 2 -o 1 -c &> sort.output "


