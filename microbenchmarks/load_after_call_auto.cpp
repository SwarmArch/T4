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
 * What happens when a load appears after a single call?
 */
#include "common.h"

#include <scc/autoparallel.h>

#include <algorithm>

constexpr uint64_t NC = 1024ul;
static uint64_t counters[NC];
static uint64_t firstcounter;

__attribute__((noinline, assertswarmified))
static void increment() {
    std::for_each(counters, counters + NC, [] (uint64_t& c) { c++; });
}

__attribute__((noinline, assertswarmified))
static void foo() {
    firstcounter = 0ul;
    increment();
    firstcounter = counters[0];
}

int main(int argc, const char** argv) {
    SCC_PARALLEL( foo() );
    tests::assert_eq(1ul, firstcounter);
    return 0;
}
