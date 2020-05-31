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

// This tests multi-level loop subsumption due to a data dependence between
// a a loop and an exit reached from a nested inner loop, as discussed in
// swarm/Swarm-IR@c48b778b57e776ac61d30f9d82f0e9a250d42fa9

#include "common.h"

#include <scc/autoparallel.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>

#define NC (1024ul)

std::array<uint64_t, NC> counters = {{0}};

__attribute__((noinline))
__attribute__((assertswarmified))
int do_increment(uint64_t c) {
  counters[c]++;
  return counters[0];
}

__attribute__((noinline))
__attribute__((assertswarmified))
unsigned foo(unsigned n) {
  unsigned c = 0;
  while (true) {
    // This if complicates control flow sufficiently to prevent loop rotation
    // from rotating the exit edge to the latch.
    if (counters[c] != 0xCAFED00D) { // This condition is true on each iteration,
                                     // but the compiler doesn't know that.
      for (int i = 0; i < n; ++i) {
        do_increment(c);
        if (counters[c] == 42)
          // This is exiting simultanously from both loops to a continuation
          // that has a data-dependence on the value of 'c'.
          return c;
        do_increment(c);
      }
    }
    c++;
    // If we made it to here at the loop latch,
    // the backedge is unconditionally taken.
  }
}

int main(int argc, const char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
    return 1;
  }
  uint64_t size = atoi(argv[1]);
  assert(size < NC);
  counters[size - 1] = 39;
  uint64_t n = 7;

  SCC_PARALLEL({
    foo(n);
  });

  std::array<uint64_t, NC> verify = {0};
  std::fill(verify.begin(), verify.begin() + size, 2 * n);
  verify[size - 1] = 42;
  tests::assert_eq(verify, counters);

  return 0;
}
