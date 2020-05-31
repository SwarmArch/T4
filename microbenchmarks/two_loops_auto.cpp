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

#define NC (125ul)

static uint64_t counters[NC];

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
  for (uint64_t c = 0; c < NC; c++) {
    counters[c]++;
  }

  for (uint64_t c = 0; c < NC; c++) {
    counters[c]++;
  }
}

int main(int argc, const char** argv) {
  SCC_PARALLEL( foo() );

  tests::assert_true(std::all_of(counters, counters + NC,
                                 [] (uint64_t c) {return c == 2;}));

  return 0;
}
