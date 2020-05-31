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

// A simple test of loop continuations with progressive expansion

#include <scc/autoparallel.h>
#include "common.h"

#include <algorithm>
#include <array>

#define NC (355UL)

uint64_t accum = 0UL;

struct myStruct {
 uint64_t n;
 uint64_t *counters;
};

__attribute__((noinline))
__attribute__((assertswarmified))
void foo(myStruct &s) {
    for (int i = 0; i < 42; i++) {
        // Since the compiler must assume s.counters[c] could alias with s.n,
        // it cannot assume that s.n is loop-invariant, and therefore it has to load
        // s.n to check the exit condition on every iteration of the loop.
        for (uint64_t c = 0; c < s.n; c++) {
            s.counters[c]++;
        }
        accum += s.counters[7];
    }
}

int main(int argc, const char** argv) {
    myStruct s;
    std::array<uint64_t, NC> counters = {{0}};
    s.counters = &counters[0];
    s.n = NC/2+1;

    SCC_PARALLEL({
        foo(s);
    });

    std::array<uint64_t, NC> verify = {{0}};
    std::fill(verify.begin(), verify.begin() + s.n, 42UL);
    tests::assert_eq(verify, counters);
    tests::assert_eq(42UL * (42UL + 1) / 2, accum);

    return 0;
}
