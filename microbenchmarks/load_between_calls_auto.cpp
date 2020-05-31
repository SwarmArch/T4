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
 * What happens when a load appears between two function calls?
 */
#include <algorithm>

#include <scc/autoparallel.h>
#include "common.h"

constexpr uint64_t CHUNK = 64ul;
constexpr uint64_t NC = 1024ul;
static uint64_t counters[NC];

__attribute__((noinline))
__attribute__((assertswarmified))
static void increment_range(uint64_t* begin, uint64_t* end, uint64_t v) {
    for (uint64_t* p = begin; p != end; p++) {
        *p += v;
    }
}

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
    // The second range of increments is dependent on the first. This will only
    // be captured if the continuation after the first call is spawned as a
    // task.
    uint64_t* begin = counters;
    increment_range(begin, begin + CHUNK, CHUNK);
    begin = counters + counters[0];
    increment_range(begin, begin + CHUNK, CHUNK);
}

int main(int argc, const char** argv) {
    std::fill(counters, counters + NC, 0ul);

    SCC_PARALLEL( foo() );

    uint64_t verify[NC];
    std::fill(verify, verify + 2 * CHUNK, CHUNK);
    std::fill(verify + 2 * CHUNK, verify + NC, 0ul);
    tests::assert_true(std::equal(verify, verify + NC, counters));
    return 0;
}
