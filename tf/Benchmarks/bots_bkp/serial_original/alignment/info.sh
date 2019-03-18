bench_name="alignment"

source_files=( "alignment.c" "alignment.h" "app-desc.h" "param.h" "sequence.c" "sequence_extern.h"  "sequence.h" )
COMPILE_FLAGS=" -lm "
RUN_OPTIONS="-I/home/gleison/tf/Benchmarks/bots/common/ -I/usr/include/linux/ "
