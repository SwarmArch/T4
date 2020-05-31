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
 * This test mixes prolog-requiring loop coarsening with parallel reductions.
 * Unfortunately as of 2019/03/26, the loop is not parallelized due to
 * bookkeeping problems. Why?
 */

#include <scc/autoparallel.h>
#include "common.h"

#include <algorithm>
#include <array>

#define NC (37ul) // not a multiple of 8, to test that coarsening gets the termination condition right

static uint64_t sum = 0;
static std::array<uint64_t, NC + 1> ones;
static std::array<uint64_t, NC + 1> counters;

__attribute__((noinline, assertswarmified))
void test(uint64_t start, uint64_t end) {
    uint64_t local = 0;
    counters[0] = 2;
    for (uint64_t i = start; i < end; i++) {
        counters[i] += ones[i];
        local += ones[i];
    }
    sum = local;
}

int main(int argc, const char* argv[]) {
    uint64_t elems = atoi(argv[1]);
    elems = std::min(NC, elems);
    counters.fill(0);
    ones.fill(1);

    SCC_PARALLEL( test(elems / 4, elems) );

    std::array<uint64_t, NC + 1> verify;
    verify.fill(0);
    verify[0] = 2;
    for (uint64_t i = elems / 4; i < elems; i++) verify[i] = 1;
    tests::assert_eq(verify, counters);
    tests::assert_eq(elems - elems / 4, sum);
}
