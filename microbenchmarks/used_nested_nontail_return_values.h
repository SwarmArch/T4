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
 * Nested, non-tail-recursive, sometimes-used, return values.
 * The #includer should define uintT. As of compiler commit 1ee10e59d3,
 * using uintT = uint16_t reveals we don't parallelize 16-bit return values
 */

#include <algorithm>

#include "common.h"
#include <scc/autoparallel.h>

constexpr uint64_t NC = 1025ul;

__attribute__((noinline))
__attribute__((assertswarmified))
static uintT one() { return 1; }

__attribute__((noinline))
__attribute__((assertswarmified))
static uintT mean(uintT* l, uintT* r) {
    return  (*l + *r) / 2;
}

__attribute__((noinline))
__attribute__((assertswarmified))
static uintT work(uintT v, uintT* begin, uintT* end) {
    //swarm::info("work v %d");
    if (!v) return one();

    uintT* mid = begin + (end - begin) / 2;
    for (uintT* p = begin; p < mid; p++) *p += v;

    v = work(v >> 1, begin, end);

    for (uintT* p = mid; p < end; p++) *p += v;
    return mean(begin, end - 1);
}

__attribute__((noinline))
__attribute__((assertswarmified))
static uintT foo() {
    uintT counters[NC];
    std::fill(counters, counters + NC, 0);

    /*unused = */ work(16, counters, counters + (NC / 2));
    uintT v  = work(25, counters + (NC / 2), counters + NC);

    uintT ret = mean(counters, counters + (NC / 2) - 1) + v;
    return ret;
}

static inline int run() {
    uintT x;
    SCC_PARALLEL({
        x = foo();
    });

    tests::assert_eq(foo(), x);
    return 0;
}
