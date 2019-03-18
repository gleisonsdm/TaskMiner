bench_name="fft"

source_files=( "bots_common.c" "fft.c" "bots_main.c" )
COMPILE_FLAGS=" -lm "
RUN_OPTIONS=" -n 33554432 -v 2 -o 1 -c &> fft.output "


