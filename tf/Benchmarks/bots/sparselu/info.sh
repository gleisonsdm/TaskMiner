bench_name="sparselu"

source_files=( "bots_common.c" "sparselu.c" "bots_main.c" )
COMPILE_FLAGS=" -lm "
RUN_OPTIONS=" -n 225 -m 15 -v 2 -o 1 -c "


