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

#include <algorithm>
#include <cstdint>
#include <vector>

#include <scc/autoparallel.h>
#include "common.h"

struct myStruct {
 uint64_t n;
 uint64_t *counters;
};

__attribute__((noinline))
//TODO(victory): As of swarm/Swarm-IR#296, this is broken up into a serial
// chain of tasks. However, the right thing to do is to speculate on the load
// of s.n for the latch exit condition check (c < s.n) to do parallel expansion.
__attribute__((assertswarmified))
void foo(myStruct &s) {
  // Since the compiler must assume s.counters[c] could alias with s.n,
  // it cannot assume that s.n is loop-invariant, and therefore it has to load
  // s.n to check each exit condition on every iteration of the loop.
  for (uint64_t c = 0; c < s.n;) {
    s.counters[c++]++;
    if (c >= s.n) break;
    s.counters[c++]++;
    if (c >= s.n) break;
    s.counters[c++]++;
    if (c >= s.n) break;
    s.counters[c++]++;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
    return 1;
  }
  uint64_t size = atoi(argv[1]);

  std::vector<uint64_t> counters(size + 1, 0UL);
  myStruct s;
  s.counters = counters.data();
  s.n = size/2+1;

  SCC_PARALLEL({
    foo(s);
  });

  std::vector<uint64_t> verify(size + 1, 0UL);
  std::fill(verify.begin(), verify.begin() + s.n, 1UL);
  tests::assert_eq(verify, counters);
}
