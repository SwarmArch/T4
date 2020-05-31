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
 * Pass (by value) a struct that wraps some integers to a function that
 * self-multiplies the local struct copies, and returns the sum of the newly
 * scaled values. This test is similar to pass_struct_byval_and_update_it_auto,
 * except it uses *local* linkage for the function in question, and uses simple
 * integers. This tests the compiler's handling of the byval attribute,
 * particularly when a byval argument can be promoted.
 */

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>

#include "common.h"
#include <scc/autoparallel.h>

struct BunchOfVals {
    uint32_t v1;
    uint32_t v2;
    uint32_t v3;
};


constexpr uint32_t NC = sizeof(BunchOfVals) / sizeof(uint32_t);


// bov's byval attribute is removed (probably promoted)
__attribute__((noinline, assertswarmified))
static uint32_t scale_and_sum(BunchOfVals bov, uint32_t multiplier) {
    bov.v1 *= multiplier;
    bov.v2 *= multiplier;
    bov.v3 *= multiplier;
    return bov.v1 + bov.v2 + bov.v3;
}


__attribute__((noinline, assertswarmified))
static uint32_t foo() {
    BunchOfVals bov({1, 2, 3});
    return scale_and_sum(bov, 2)
           + scale_and_sum(bov, 3)
           + scale_and_sum(bov, 4);
}


int main() {
    uint32_t val;
    SCC_PARALLEL({
        val = foo();
    });

    tests::assert_eq(9u * (NC * (NC + 1)) / 2, val);
    return 0;
}
