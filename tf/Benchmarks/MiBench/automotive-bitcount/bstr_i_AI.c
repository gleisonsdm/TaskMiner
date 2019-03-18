#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif
/* +++Date last modified: 05-Jul-1997 */

/*
**  Make an ascii binary string into an integer.
**
**  Public domain by Bob Stout
*/

#include <string.h>
#include "bitops.h"

unsigned int bstr_i(char *cptr) {
  unsigned int i, j = 0;

  while (cptr && *cptr && strchr("01", *cptr)) {
    i = *cptr++ - '0';
    j <<= 1;
    j |= (i & 0x01);
  }
  return (j);
}

#ifdef TEST

#include <stdlib.h>

int main(int argc, char *argv[]) {
  char *arg;
  unsigned int x;

  while (--argc) {
    x = bstr_i(arg = *++argv);
    printf("Binary %s = %d = %04Xh\n", arg, x, x);
  }
  return EXIT_SUCCESS;
}

#endif /* TEST */

