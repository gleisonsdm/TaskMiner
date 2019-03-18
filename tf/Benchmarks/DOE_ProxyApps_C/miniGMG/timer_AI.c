#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif
//------------------------------------------------------------------------------------------------------------------------------
// Samuel Williams
// SWWilliams@lbl.gov
// Lawrence Berkeley National Lab
//------------------------------------------------------------------------------------------------------------------------------
#include <stdint.h>
uint64_t CycleTime() {
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if !defined(USE_CYCLE_COUNTER)
  return 0;
#elif __has_builtin(__builtin_readcyclecounter)
  return __builtin_readcyclecounter();
#else
  uint64_t lo, hi;
  __asm__ __volatile__("rdtsc"
                       : "=a"(lo), "=d"(hi));
  return ((((uint64_t)hi) << 32) | ((uint64_t)lo));
#endif
}

