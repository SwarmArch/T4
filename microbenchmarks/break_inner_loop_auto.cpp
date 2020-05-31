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

#include "common.h"

#include <scc/autoparallel.h>

#include <algorithm>

#define NC (1024UL)

static uint64_t counters[NC];

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
  for (uint64_t c = 0; c < NC / 32; c++) {
    for (uint64_t i = 0; i < 32; i++) {
      if (counters[32*c + i]) {
        // This block is technically not in the loop, so there are two exits.
        counters[32*c + i] = 2;
        break;
      }
      counters[32*c + i] = 3;
    }
  }
}

int main(int argc, const char** argv) {
  SCC_PARALLEL({
    counters[NC/2 + 10] = 1UL;
    foo();
  });

  uint64_t verify[NC];
  std::fill(verify, verify + NC, 3ul);
  verify[NC/2 + 10] = 2;
  std::fill(verify + NC/2 + 10 + 1, verify + NC/2 + 32, 0ul);
  tests::assert_true(std::equal(verify, verify + NC, counters));
  return 0;
}
