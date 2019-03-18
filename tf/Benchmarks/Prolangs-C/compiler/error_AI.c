#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif
/****  error.c  **********************************************************/

#include "global.h"
extern void match(int t);

void error(char *m) /*   generates all error messages */
{
  ErrorFlag = 1;
  printf("\nERROR: line %d: %s \n", lineno, m);

  if (lookahead == DONE)
    return;

  printf("Skipping: ");
  while ((lookahead != ';') && (lookahead != EOF)) {
    lookahead = getc(stdin);
    if (lookahead == '\n')
      ++lineno;
    if (lookahead != EOF)
      printf("%c", lookahead);
  }

  if (lookahead == EOF) {
    lookahead = DONE;
    return;
  } else
    match(';');

  printf("\n continuing parsing...\n");
  return;
}

