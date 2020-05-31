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

// This test reproduces the issues with handling the control flow of 429.mcf's
// main outermost loop in its global_opt() function.

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
void foo(unsigned size) {
  uint64_t c = 0;
  while (true) {
    do_increment(c);
    if (c == size)
      break;
    do_increment(c++);
    // The loop latch here does not exit, although it does conditionally branch
    // to this terminated-by-unreachable block.
    if (!counters[0]) {
      printf("This should not happen.\n");
      std::abort();
    }
  }
}

int main(int argc, const char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
    return 1;
  }
  uint64_t size = atoi(argv[1]);
  assert(size < NC);

  SCC_PARALLEL({
    foo(size);
  });

  std::array<uint64_t, NC> verify = {0};
  std::fill(verify.begin(), verify.begin() + size, 2UL);
  verify[size] = 1UL;
  tests::assert_eq(verify, counters);

  return 0;
}
