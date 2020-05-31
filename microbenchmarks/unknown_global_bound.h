/** $lic$
 * Copyright (C) 2014-2020 by Massachusetts Institute of Technology
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 * This software was developed and released to accompany the T4 ISCA 2020 paper
 * ("T4: Compiling Sequential Code for Effective Speculative Parallelization in
 * Hardware", Ying et al., ISCA-47, June 2020).  If you use this software in
 * your research, we request that you reference the T4 paper, and we ask that
 * you send us a citation of your work.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <scc/autoparallel.h>
#include "common.h"

constexpr uint64_t NC = 1024UL;

uintT n;
uintT *counters;

__attribute__((noinline))
__attribute__((assertswarmified))
static void bar(uintT c) {
    counters[c]++;
}

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
  // Since the compiler must assume bar() could modify n,
  // it cannot assume that n is loop-invariant, and therefore it has to load
  // n to check the exit condition on every iteration of the loop.
  // Furthermore, this loop has no live-in local values to capture into a closure.
  for (uintT c = 0; c < n; c++) {
    bar(c);
  }
}

static inline void unknown_bound() {
  counters = new uintT[NC]();
  const uintT N = NC/2 + 1;
  n = N;

  SCC_PARALLEL( foo() );

  tests::assert_true(std::all_of(counters, counters + N,
                                 [](uintT x) { return x == 1; }));
  tests::assert_true(std::all_of(counters + N, counters + NC,
                                 [](uintT x) { return x == 0; }));

  delete [] counters;
}
