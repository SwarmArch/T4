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

#include <array>

constexpr uint64_t NC = 5;

__attribute__((noinline))
__attribute__((assertswarmified))
static void increment(uint64_t *counters, uint64_t i) {
    counters[i]++;
}

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo(uint64_t *counters, uint64_t *queries) {
    for (uint64_t c = 0; c < NC; c++) {
        if (c) increment(counters, 1);
        else increment(counters, 2);
        queries[c] = counters[c];
        increment(counters, c);
    }
}

int main(int argc, const char** argv) {
    std::array<uint64_t, NC> counters, queries;
    counters.fill(0);
    queries.fill(0);

    SCC_PARALLEL( foo(&counters[0], &queries[0]) );

    std::array<uint64_t, NC> vcounters, vqueries;
    vcounters.fill(0);
    vqueries.fill(0);
    foo(&vcounters[0], &vqueries[0]);
    tests::assert_eq(counters, vcounters);
    tests::assert_eq(queries, vqueries);

    return 0;
}
