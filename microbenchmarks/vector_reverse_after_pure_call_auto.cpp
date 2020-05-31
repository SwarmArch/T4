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
 * Reverse part of a vector.
 *
 * Determine the amount up to which to reverse by calling a read-only (pure)
 * function. This tests the interaction of continuation-passing for a function
 * in a different module vs. the read-only attribute.
 */
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

uint64_t upto;

// Defined in a separate module
__attribute__((pure))
uint64_t return_self(uint64_t x);


__attribute__((noinline, assertswarmified))
void reverser(uint8_t* v) {
    uint64_t d = return_self(upto);
    std::reverse(v, v + d);
}


int main(int argc, const char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <size> <upto>" << std::endl;
        return -1;
    }
    std::vector<uint8_t> v(atoi(argv[1]));
    upto = atoi(argv[2]);
    assert(!v.empty());
    assert(upto < v.size());

    for (uint64_t i = 0; i < upto; i++) v[i] = upto - 1 - i;
    std::iota(v.begin() + upto, v.end(), upto);

    SCC_PARALLEL( reverser(v.data()) );

    std::vector<uint8_t> expected(v.size());
    std::iota(expected.begin(), expected.end(), 0ul);
    tests::assert_eq(expected, v);
    return 0;
}
