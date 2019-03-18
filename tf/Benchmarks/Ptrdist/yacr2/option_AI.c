#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif

/*
 *
 * option.c
 *
 */

#include <string.h>

#define OPTION_CODE

/*
 *
 * Includes.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "channel.h"

/*
 *
 * Code.
 *
 */

void Option(int argc,
            char *argv[]) {
  /*
     * Check arguments.
     */
  if (argc != 2) {
    printf("\nUsage: yacr2 <filename>\n\n");
    exit(1);
  }

  /*
     * Specified options.
     */
  channelFile = argv[1];
}

