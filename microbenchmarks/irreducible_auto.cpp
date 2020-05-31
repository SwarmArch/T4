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

// This tests the compiler's handling of an irreducible CFG.

#include "common.h"

#include <scc/autoparallel.h>

#include <algorithm>
#include <array>
#include <cassert>

#define NC (1024ul)

std::array<uint64_t, NC> counters = {{0}};

__attribute__((noinline))
__attribute__((assertswarmified))
void do_increment(uint64_t c) {
  counters[c]++;
}

__attribute__((noinline))
//TODO: Uncomment this if we ever handle irreducible control flow
//__attribute__((assertswarmified))
void foo(unsigned size) {
  if (size == 0) goto exit;

  if (size & 1) goto odd;
  else goto even;

// even and odd form a cycle in the CFG, but not a proper natural loop
// because the cycle has two entry points.
odd:
  --size;
  do_increment(size);
  if (size == 0) goto exit;
  else goto even;

even:
  --size;
  do_increment(size);
  goto odd;

exit:
  return;
}

int main(int argc, const char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
    return 1;
  }
  uint64_t size = atoi(argv[1]);
  assert(size < NC);

  SCC_PARALLEL( foo(size) );

  std::array<uint64_t, NC> verify = {0};
  std::fill(verify.begin(), verify.begin() + size, 1UL);
  tests::assert_eq(verify, counters);

  return 0;
}
