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
 * foo has an (unreachable) infinite loop
 */
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

__attribute__((noinline, assertswarmified))
void foo(uint64_t b, std::vector<uint32_t>& v) {
    if (b) while (true);
    std::for_each(v.begin(), v.end(), [] (uint32_t& c) { c++; });
}


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }
    std::vector<uint32_t> v(atoi(argv[1]), 0);
    assert(!v.empty());
    volatile uint64_t b = false;

    SCC_PARALLEL( foo(b, v) );

    std::vector<uint32_t> expected(v.size());
    std::fill(expected.begin(), expected.end(), 1);
    tests::assert_eq(expected, v);
    return 0;
}
