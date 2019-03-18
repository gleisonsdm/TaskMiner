#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif
/* NIST Secure Hash Algorithm */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sha.h"

int main(int argc, char **argv) {
  FILE *fin;
  SHA_INFO sha_info;

  if (argc < 2) {
    fin = stdin;
    sha_stream(&sha_info, fin);
    sha_print(&sha_info);
  } else {
    while (--argc) {
      fin = fopen(*(++argv), "rb");
      if (fin == NULL) {
        printf("error opening %s for reading\n", *argv);
      } else {
        sha_stream(&sha_info, fin);
        sha_print(&sha_info);
        fclose(fin);
      }
    }
  }
  return (0);
}

