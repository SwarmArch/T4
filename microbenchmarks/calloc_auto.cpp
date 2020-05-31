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

#define NC (10013ul)

static uint64_t* counters;

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
    counters = (uint64_t*)std::calloc(NC + 1, sizeof(*counters));
    for (uint64_t c = 0; c < NC; c++) {
        counters[c]++;
  }
}

int main(int argc, const char** argv) {
    SCC_PARALLEL({
        foo();
    });

    tests::assert_true(std::find_if(counters, counters + NC + 2,
                                    [](uint64_t x) { return x != 1; }) ==
                       &counters[NC]);
    tests::assert_true(counters[NC] == 0);

    return 0;
}
