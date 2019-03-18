#include <omp.h>
#ifndef taskminerutils
#define taskminerutils
static int taskminer_depth_cutoff = 0;
#define DEPTH_CUTOFF omp_get_num_threads()
extern char cutoff_test;
#endif
/* For copyright information, see olden_v1.0/COPYRIGHT */

/* build.c
 *
 * By:  Martin C. Carlisle
 * 6/15/94
 * builds the tree for the Power Pricing problem
 *
 * based on code by:  Steve Lumetta, Sherry Li, and Ismail Khalil
 * University of California at Berkeley
 */

#include "power.h"

Root build_tree() {
  int i;
  Root t;
  Lateral l;

  t = (Root)malloc(sizeof(*t));

  for (i = 0; i < NUM_FEEDERS; i++) {
    /* Insert future here, split into two loops */
    l = build_lateral(i * LATERALS_PER_FEEDER, LATERALS_PER_FEEDER);
    t->feeders[i] = l;
  }
  t->theta_R = 0.8;
  t->theta_I = 0.16;
  return t;
}

Lateral build_lateral(int i, int num) {
  #pragma omp critical
  taskminer_depth_cutoff++;
  Lateral l;
  Branch b;
  Lateral next;

  if (num == 0)
    return NULL;
  l = (Lateral)malloc(sizeof(*l));

  {
  cutoff_test = (taskminer_depth_cutoff < DEPTH_CUTOFF);
  #pragma omp task untied default(shared) final(cutoff_test)
  next = build_lateral(i, num - 1);
  #pragma omp taskwait
  }
  b = build_branch(i * BRANCHES_PER_LATERAL, (num - 1) * BRANCHES_PER_LATERAL,
                   BRANCHES_PER_LATERAL);

  l->next_lateral = next;
  l->branch = b;
  l->R = 1 / 300000.0;
  l->X = 0.000001;
  l->alpha = 0.0;
  l->beta = 0.0;
  return l;
#pragma omp critical
taskminer_depth_cutoff--;
}

Branch build_branch(int i, int j, int num) {
  #pragma omp critical
  taskminer_depth_cutoff++;
  Leaf l;
  Branch b;

  if (num == 0)
    return NULL;
  /* allocate branch */
  b = (Branch)malloc(sizeof(*b));

  /* fill in children */
  {
  cutoff_test = (taskminer_depth_cutoff < DEPTH_CUTOFF);
  #pragma omp task untied default(shared) final(cutoff_test)
  b->next_branch = build_branch(i, j, num - 1);
#pragma omp taskwait
}

  for (i = 0; i < LEAVES_PER_BRANCH; i++) {
    l = build_leaf();
    b->leaves[i] = l;
  }

  /* fill in values */
  b->R = 0.0001;
  b->X = 0.00002;
  b->alpha = 0.0;
  b->beta = 0.0;
  return b;
#pragma omp critical
taskminer_depth_cutoff--;
}

Leaf build_leaf() {
  Leaf l;

  l = (Leaf)malloc(sizeof(*l));
  l->D.P = 1.0;
  l->D.Q = 1.0;
  return l;
}

