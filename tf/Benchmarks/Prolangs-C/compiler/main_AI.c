#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif
/****  main.c  *********************************************************/

#include "global.h"

extern void init(void);
extern void parse(void);

int main(void) {
  init();
  parse();
  return 0; /*  successful termination  */
}

