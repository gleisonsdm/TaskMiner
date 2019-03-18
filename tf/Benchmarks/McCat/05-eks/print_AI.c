#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif

/****
    Copyright (C) 1996 McGill University.
    Copyright (C) 1996 McCAT System Group.
    Copyright (C) 1996 ACAPS Benchmark Administrator
                       benadmin@acaps.cs.mcgill.ca

    This program is free software; you can redistribute it and/or modify
    it provided this copyright notice is maintained.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
****/

/************************************************************************/
/*  author :   Mikkel Damsgaard                                         */
/*             Kirsebaerhaven 85b                                        */
/*                                                                      */
/*             DK-8520 Lystrup                                          */
/*             email mikdam@daimi.aau.dk                                */
/*                                                                      */
/*  files :                                                             */
/*  Divsol.c           QRfact.h           Divsol.h           Jacobi.c   */
/*  Jacobi.h           Triang.c           print.c            MM.c       */
/*  Triang.h           print.h            MM.h               QRfact.c   */
/*  main.c             main.h                                           */
/*                                                                      */
/*  It calculates the eigenvalues for 4 different matrixes. It does not */
/*  take any input; those 4 matrixes are calculated by MakeMatrix       */
/*  function. Output is given as 4 files: val2, val3, val4, val5, that  */
/*  contains the eigenvalues for each of the matrixes.                  */
/*                                                                      */
/************************************************************************/
#include "print.h"

void printMatrix(Matrix A) {
  int i;

  #pragma omp parallel
  #pragma omp single
  for (i = 0; i < n; i++)
    {
    cutoff_test = (taskminer_depth_cutoff < DEPTH_CUTOFF);
    #pragma omp task untied default(shared) depend(in:A[i])
    {
    printVector(A[i]);
  #pragma omp taskwait
  }
   }
  printf("\n");
}

void printVector(Vector v) {
  int i;

  for (i = 0; i < n; i++)
    printf("%f ", v[i]);
  printf("\n");
}

