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
 * A read-write iteration over a vector
 */
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

__attribute__((noinline, assertswarmified))
void filler(std::vector<uint64_t>& v, uint64_t value) {
    std::fill(v.begin(), v.end(), value);
}

int main(int argc, const char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <size> <value>" << std::endl;
        return -1;
    }

    std::vector<uint64_t> v(atoi(argv[1]));
    assert(!v.empty());
    const uint64_t value = atoi(argv[2]);

    SCC_PARALLEL( filler(v, value) );

    std::vector<uint64_t> expected(v.size());
    std::fill(expected.begin(), expected.end(), value);
    assert(expected.size() == v.size());
    tests::assert_true(std::equal(v.begin(), v.end(), expected.begin()));
    return 0;
}
