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

struct myStruct {
  uintT n;
  uintT *counters;
};

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo(myStruct &s) {
  // Since the compiler must assume s.counters[c] could alias with s.n,
  // it cannot assume that s.n is loop-invariant, and therefore it has to load
  // s.n to check the exit condition on every iteration of the loop.
  for (uintT c = 0; c < s.n; c++) {
    s.counters[c]++;
  }
}

static inline void unknown_bound() {
  myStruct s;
  s.counters = new uintT[NC]();
  const uintT n = NC/2 + 1;
  s.n = n;

  SCC_PARALLEL( foo(s) );

  tests::assert_true(std::all_of(s.counters, s.counters + n,
                                 [](uintT x) { return x == 1; }));
  tests::assert_true(std::all_of(s.counters + n, s.counters + NC,
                                 [](uintT x) { return x == 0; }));
  delete [] s.counters;
}
