bench_name="uts"

source_files=( "bots_common.c" "brg_sha1.c" "uts.c" "bots_main.c" )
COMPILE_FLAGS=" -lm "
RUN_OPTIONS=" -f medium.input -v 1 -o 1 -c &> uts.output "


