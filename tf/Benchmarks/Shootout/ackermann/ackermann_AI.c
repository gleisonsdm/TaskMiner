#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif
/* -*- mode: c -*-
 * $Id: ackermann.c 35417 2007-03-28 04:46:30Z jeffc $
 * http://www.bagley.org/~doug/shootout/
 */

#include <stdio.h>
#include <stdlib.h>

int Ack(int M, int N) {
  #pragma omp critical
  taskminer_depth_cutoff++;
  if (M == 0)
    return (N + 1);
  if (N == 0)
    {
    cutoff_test = (taskminer_depth_cutoff < DEPTH_CUTOFF);
    #pragma omp task untied default(shared) final(cutoff_test)
    return (Ack(M - 1, 1));
  }
  return (Ack(M - 1, Ack(M, (N - 1))));
#pragma omp critical
taskminer_depth_cutoff--;
}

int main(int argc, char *argv[]) {
  int n = ((argc == 2) ? atoi(argv[1]) : 8);

  printf("Ack(3,%d): %d\n", n, Ack(3, n));
  return (0);
}

