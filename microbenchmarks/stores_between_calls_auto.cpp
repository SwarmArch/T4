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
 * What happens when function calls are interspersed with stores?
 */
#include <scc/autoparallel.h>
#include "common.h"

#include <algorithm>
#include <array>

constexpr uint64_t CHUNK = 64ul;
constexpr uint64_t NC = 1024ul;
static std::array<uint64_t, NC> counters;

__attribute__((noinline))
__attribute__((assertswarmified))
static void increment_range(uint64_t* begin, uint64_t* end, uint64_t v) {
    for (uint64_t* p = begin; p != end; p++) *p += v;
}

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
    // Can spawn all three calls in parallel from one parent task?
    increment_range(&counters[0], &counters[CHUNK], CHUNK);
    counters[0] = 1;
    increment_range(&counters[CHUNK], &counters[2 * CHUNK], CHUNK);
    counters[0] = 2;
    increment_range(&counters[2 * CHUNK], &counters[3 * CHUNK], CHUNK);
    counters[0] = 3;
}

int main() {
    std::fill(counters.begin(), counters.end(), 0ul);

    SCC_PARALLEL( foo() );

    std::array<uint64_t, NC> verify;
    std::fill(&verify[1], &verify[3 * CHUNK], CHUNK);
    std::fill(&verify[3 * CHUNK], &verify[NC], 0ul);
    verify[0] = 3;
    tests::assert_eq(verify, counters);
    return 0;
}
