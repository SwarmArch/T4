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
// except in addition to using that loaded value to check the loop exit
// condition, also use it in the loop body across the previous and next
// iteration of the loop.

#include <scc/autoparallel.h>
#include "common.h"

constexpr uint64_t NC = 1024UL;

struct myStruct {
  uint64_t n;
  uint64_t *counters;
};

__attribute__((noinline))
__attribute__((assertswarmified))
uint64_t foo(myStruct &s) {
  uint64_t sum = 0;
  // Since the compiler must assume s.counters[c] could alias with s.n,
  // it cannot assume that s.n is loop-invariant, and therefore it has to load
  // the value s.n from memory after each store to s.counters[c].
  for (uint64_t c = 0; c < s.n; c++) {
    s.counters[c] += s.n; // Uses the value of s.n from the previous iteration
    sum += s.n; // Uses the same fresh value of s.n as the loop exit condition check.
  }
  return sum;
}

int main() {
  uint64_t counters[NC];
  std::fill(counters, counters+NC, 0UL);
  const uint64_t n = NC/2 + 1;
  myStruct s = {n, counters};

  uint64_t sum;
  SCC_PARALLEL({
    sum = foo(s);
  });

  tests::assert_eq(n*n, sum);
  tests::assert_true(std::all_of(s.counters, s.counters + n,
                                 [n](uint64_t x) { return x == n; }));
  tests::assert_true(std::all_of(s.counters + n, s.counters + NC,
                                 [](uint64_t x) { return x == 0; }));
  return 0;
}
