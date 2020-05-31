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

// Like the unknown_bound* tests, in which each iteration of the loop has to
// repeatedly load an actually-loop-invariant loop bound variable from memory,
// except there are two loops in a row, and the latter loop might try to re-use
// the value of the loop bound loaded in the last iteration of the first loop.
// The first loop was early-chained until the compiler learned to break the
// dependency between the loops in Swarm-IR@ca54be452c6d1ec646aca329f9f4589cb098dda3

#include <scc/autoparallel.h>
#include "common.h"

constexpr uint64_t NC = 1024UL;

struct myStruct {
  uint64_t n;
  uint64_t *counters;
};

__attribute__((noinline))
__attribute__((assertswarmified))
void foo(myStruct &s) {
  // Since the compiler must assume s.counters[c] could alias with s.n,
  // it cannot assume that s.n is loop-invariant, and therefore it has to load
  // the value s.n from memory after each store to s.counters[c].
  for (uint64_t c = 0; c < s.n; c++) {
    s.counters[c] += 1;
  }
  for (uint64_t c = 0; c < s.n; c++) {
    s.counters[c] += 1;
  }
}

int main() {
  uint64_t counters[NC];
  std::fill(counters, counters+NC, 0UL);
  const uint64_t n = NC/2 + 1;
  myStruct s = {n, counters};

  SCC_PARALLEL({
    foo(s);
  });

  tests::assert_true(std::all_of(s.counters, s.counters + n,
                                 [](uint64_t x) { return x == 2; }));
  tests::assert_true(std::all_of(s.counters + n, s.counters + NC,
                                 [](uint64_t x) { return x == 0; }));
  return 0;
}
