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
 * This test sets up dynamically sized stack-allocated variables that are passed
 * to function calls. For extra fun, it uses recursion. This set up is simplest
 * using pure C, so see the C modules for implementations.
 *
 * As of swarm/Swarm-IR@ca8ee97579771e6, the compiler does not crash,
 * but none of the functions are parallelized.
 */

#include <scc/autoparallel.h>
#include "common.h"
#include "pass_dynamic_stack.h"

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }

    uint64_t size = atoi(argv[1]);
    uint64_t x;

    SCC_PARALLEL({
        x = foo(size);
    });

    uint64_t expected = (size % 2) ? (size + 1) : (2 * size);
    tests::assert_eq(expected, x);
    return 0;
}
