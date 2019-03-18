bench_name="sparselu"

source_files=( "bots_common.c" "sparselu.c" "bots_main.c" )
COMPILE_FLAGS=" -lm "
RUN_OPTIONS=" -n 625 -m 25 -v 2 -o 1 -c &> sparselu.output "


