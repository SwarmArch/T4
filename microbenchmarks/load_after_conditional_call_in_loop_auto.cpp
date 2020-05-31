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

constexpr uint64_t NC = 1024ul;
static uint64_t counters[NC], queries[NC];

__attribute__((noinline))
__attribute__((assertswarmified))
static void increment(uint64_t c) {
    counters[c]++;
}

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
    for (uint64_t c = 0; c < NC; c++) {
        if (c) increment(c);
        queries[c] = counters[c];
        increment(c);
    }
}

int main(int argc, const char** argv) {
    std::fill(counters, counters + NC, 0ul);
    std::fill(queries, queries + NC, 0ul);

    SCC_PARALLEL( foo() );

    uint64_t vcounters[NC], vqueries[NC];
    // The first counter was incremented only once
    vcounters[0] = 1ul;
    vqueries[0] = 0ul;
    std::fill(vcounters + 1, vcounters + NC, 2ul);
    std::fill(vqueries + 1, vqueries + NC, 1ul);
    tests::assert_true(
            std::equal(vcounters, vcounters + NC, counters) &&
            std::equal(vqueries, vqueries + NC, queries)
            );
    return 0;
}
