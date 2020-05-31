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

// This test demonstrates
// https://github.mit.edu/swarm/Swarm-IR/pull/583

#include <algorithm>

#include <scc/autoparallel.h>
#include "common.h"

#define NC (733UL)

uint64_t counters[NC];

__attribute__((noinline))
__attribute__((assertswarmified))
void bar() {
  counters[NC-1] = 42;
}

__attribute__((noinline))
__attribute__((assertswarmified))
void foo() {
  uint64_t c = 0;
  // Because c is an output of this loop, the continuation of this loop, which
  // will be the two canonicalized loops below, will be subsumed into this loop,
  // which will then be early chained because the exit condition load is unsafe
  // to move without proper understanding of control flow.
  while (!counters[c]) {
    counters[c++]++;
    bar();
  }

  for (; c < NC; c++) {
    counters[c] += 2;
    bar();
  }

  for (uint64_t i = 0; i < NC; i++) {
    bar();
  }
}

int main(int argc, const char** argv) {
  counters[NC/2] = 1UL;

  SCC_PARALLEL({
    foo();
  });

  uint64_t verify[NC];
  std::fill(verify, verify + NC/2, 1ul);
  verify[NC/2] = 3ul;
  std::fill(verify + NC/2 + 1, verify + NC - 1, 2ul);
  verify[NC-1] = 42;
  tests::assert_true(std::equal(verify, verify + NC, counters));

  return 0;
}
