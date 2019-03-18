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

#include "math.h"

#define DEPTH_CUTOFF 12
int taskminer_depth_cutoff = 0;
char cutoff_test;

static long long res;

long long fib(int n) {
  for (int i = 0; i < 30000; i++);
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

#pragma omp critical
taskminer_depth_cutoff--;
   return x + y; 
}

void fib0(int n) {
  #pragma omp parallel
  #pragma omp single
  #pragma omp task
  res = fib(n);
  printf("Fibonacci result for %d is %lld\n", n, res);
}

int main(int argc, char *argv[]) {
  int n = atoi(argv[1]);
  fib0(n);
  return 0;
}
