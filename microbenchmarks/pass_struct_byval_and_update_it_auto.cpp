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
 * Pass (by value) a struct that wraps an integer array to a function that
 * self-multiplies the local struct copy, and returns the sum of the newly
 * scaled array.
 *
 * This tests our compiler's handling of the byval attribute, particularly when
 * the local copy of the struct is updated. This should *not* update the
 * original value.
 */

#include <algorithm>
#include <cassert>
#include <iostream>

#include "common.h"
#include <scc/autoparallel.h>

#include "pass_struct_byval_and_update_it.h"

__attribute__((assertswarmified))
__attribute__((noinline))
uint32_t foo() {
    BunchOfVals bov;
    std::iota(bov.vals, bov.vals + sizeof(bov.vals) / sizeof(bov.vals[0]), 0ul);
    return scale_and_sum(bov, 2) + scale_and_sum(bov, 3);
}


int main() {
    uint32_t val;
    SCC_PARALLEL({
        val = foo();
    });

    tests::assert_eq(5u * (NC * (NC - 1)) / 2, val);
    return 0;
}
