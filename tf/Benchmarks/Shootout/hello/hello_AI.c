#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif
/* -*- mode: c -*-
 * $Id: hello.c 1756 2002-02-14 05:49:23Z lattner $
 * http://www.bagley.org/~doug/shootout/
 */

#include <stdio.h>

int main() {
  puts("hello world\n");
  return (0);
}

