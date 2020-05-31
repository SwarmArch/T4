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
 * foo's loop induction variable is a pointer
 */
#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

__attribute__((noinline))
__attribute__((assertswarmified))
void foo(intT* begin, intT* end) {
    for (intT* c = begin; c < end; c++) (*c)++;
    // Avoid the readnone attribute:
    (*(end - 1))++;
}


static inline int loop_pointer_iv(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }
    std::vector<intT> v(atoi(argv[1]), 0);
    assert(!v.empty());

    SCC_PARALLEL( foo(v.data(), v.data() + v.size()) );

    std::vector<intT> expected(v.size(), 1);
    expected[expected.size() - 1] = 2;
    tests::assert_eq(expected, v);
    return 0;
}
