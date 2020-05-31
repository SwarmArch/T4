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
 * This test mixes prolog-requiring loop coarsening due to a store
 * with parallel reductions.
 * It reproduces the crash observed in
 * https://github.mit.edu/swarm/Swarm-IR/pull/678#issuecomment-38255
 */

#include <scc/autoparallel.h>
#include "common.h"

#include <algorithm>
#include <array>

// not a multiple of 8, to test that coarsening gets the termination condition right
#define NC (1011ul)
#define MAGIC (0xf0ul)

static uint64_t sum = 0;
static std::array<uint64_t, NC + 1> ones;
static std::array<uint64_t, NC + 1> counters;

__attribute__((noinline, assertswarmified))
void test(uint64_t start, uint64_t end) {
    counters[0] = MAGIC * 2;
    for (uint64_t i = start; i < end; i++) {
        // As of swarm/Swarm-IR@40ccb887e580964a2d49a6, we must perform a simple
        // store here. An increment of ones[i], as in
        // loop_coarsen_increment_and_reduce_auto,
        // fails to be parallelized because the compiler is fragile AF.
        counters[i] = MAGIC;
        sum += ones[i];
    }
}

int main(int argc, const char* argv[]) {
    uint64_t elems = atoi(argv[1]);
    elems = std::min(NC, elems);
    counters.fill(0);
    ones.fill(1);
    const uint64_t start = elems / 8;

    SCC_PARALLEL( test(start, elems) );

    std::array<uint64_t, NC + 1> verify;
    verify.fill(0);
    verify[0] = MAGIC * 2;
    for (uint64_t i = start; i < elems; i++) verify[i] = MAGIC;
    tests::assert_eq(verify, counters);
    tests::assert_eq(elems - start, sum);
}
