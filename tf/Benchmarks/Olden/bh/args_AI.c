#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif
/* For copyright information, see olden_v1.0/COPYRIGHT */
#include <stdlib.h>

extern int NumNodes;
extern int nbody;

int dealwithargs(int argc, char *argv[]) {
  int level;

  if (argc > 2)
    NumNodes = atoi(argv[2]);
  else
    NumNodes = 4;

  if (argc > 1)
    nbody = atoi(argv[1]);
  else
    nbody = 32;

  return level;
}

