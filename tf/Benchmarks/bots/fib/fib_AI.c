#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif
/**********************************************************************************************/
/*  This program is part of the Barcelona OpenMP Tasks Suite                                  */
/*  Copyright (C) 2009 Barcelona Supercomputing Center - Centro Nacional de Supercomputacion  */
/*  Copyright (C) 2009 Universitat Politecnica de Catalunya                                   */
/*                                                                                            */
/*  This program is free software; you can redistribute it and/or modify                      */
/*  it under the terms of the GNU General Public License as published by                      */
/*  the Free Software Foundation; either version 2 of the License, or                         */
/*  (at your option) any later version.                                                       */
/*                                                                                            */
/*  This program is distributed in the hope that it will be useful,                           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of                            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                             */
/*  GNU General Public License for more details.                                              */
/*                                                                                            */
/*  You should have received a copy of the GNU General Public License                         */
/*  along with this program; if not, write to the Free Software                               */
/*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA            */
/**********************************************************************************************/

#include "bots.h"
#include "fib.h"

static long long res;

long long fib(int n) {
  #pragma omp critical
  taskminer_depth_cutoff++;
  long long x, y;
  if (n < 2)
    return n;

  {
  cutoff_test = (taskminer_depth_cutoff < DEPTH_CUTOFF);
  #pragma omp task untied default(shared) final(cutoff_test)
  x = fib(n - 1);
  }
  {
  cutoff_test = (taskminer_depth_cutoff < DEPTH_CUTOFF);
  #pragma omp task untied default(shared) final(cutoff_test)
  y = fib(n - 2);
#pragma omp taskwait
}

  return x + y;
#pragma omp critical
taskminer_depth_cutoff--;
}

void fib0(int n) {
  res = fib(n);
  printf("Fibonacci result for %d is %lld\n", n, res);
}

