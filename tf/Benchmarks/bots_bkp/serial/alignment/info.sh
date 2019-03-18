bench_name="alignment"

#source_files=( "app-desc.h" "alignment.h" "bots_common.h" "bots.h" "bots_main.h" "ompss-app.h" "omp-tasks-app.h" "param.h" "sequence_extern.h" "sequence.h" "serial-app.h" "alignment.c" "bots_common.c" "bots_main.c" "sequence.c" )
source_files=( "alignment.c" "bots_common.c" "bots_main.c" "sequence.c" )
COMPILE_FLAGS=" -lm "
RUN_OPTIONS=" -o 0 -v 2 -f prot.100.aa "


