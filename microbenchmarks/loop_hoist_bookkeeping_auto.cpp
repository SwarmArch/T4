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

#include <scc/autoparallel.h>
#include "common.h"

#define NC (101ul)

static uint64_t counters[NC];

__attribute__((noinline))
__attribute__((assertswarmified))
void test() {
  // The computation of (i + 1) is shared between the loop body and
  // the loop latch (which uses it for the exit condition).
  for (int i = 0; i < NC-1; i++)
    counters[i + 1]++;
}

int main(int argc, const char* argv[]) {
  SCC_PARALLEL( test() );

  uint64_t verify[NC];
  verify[0] = 0;
  std::fill(verify + 1, verify + NC, 1ul);
  tests::assert_true(std::equal(verify, verify + NC, counters));
}
