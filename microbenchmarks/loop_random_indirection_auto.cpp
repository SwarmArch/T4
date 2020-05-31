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

/**
 * Loop lookup indirection and stack-allocated variables, oh my
 */
#include <algorithm>
#include <random>

#include <scc/autoparallel.h>
#include "common.h"

constexpr uint64_t NC = 1999;
static uint64_t counters[NC];

__attribute__((noinline, assertswarmified))
static void increment(uint64_t* indices) {
    for (int i = 0; i < NC; i++) {
        counters[indices[i]]++;
    }
}

__attribute__((noinline))
// FIXME(mcj) Due to exception-handling in these std calls, setup is not
// parallelized.
static void setup(uint64_t* indices) {
    std::iota(indices, indices + NC, 0ul);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices, indices + NC, g);
}

int main() {
    SCC_PARALLEL({
        uint64_t indices[NC];
        setup(indices);
        increment(indices);
    });

    uint64_t verify[NC];
    std::fill(verify, verify + NC, 1ul);
    tests::assert_true(std::equal(verify, verify + NC, counters));
    return 0;
}
