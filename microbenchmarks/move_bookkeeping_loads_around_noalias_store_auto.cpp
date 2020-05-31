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

/*
 * A test where it is desirable to exercise progressive enqueue, and
 * to exercise the transformation introduced in swarm/Swarm-IR#512,
 * but enabling the desired parallelization transformation requires
 * alias analysis.
 */

#include "common.h"
#include <scc/autoparallel.h>

#include <cassert>
#include <vector>

__attribute__((noinline, assertswarmified))
void foo(uint64_t *n,
         uint64_t data[],
         uint64_t *__restrict some_noalias_pointer) {
    for (uint64_t i = 0; i < *n; ++i) {
        switch (i % 3) {
        case 0:
            // In this case, alias analysis should trivially prove that
            // *n doesn't change on this branch.
            ++*some_noalias_pointer;
            break;
        case 1:
            // The compiler must assume &data[i] could alias with n.
            data[i] = 42;
            break;
        default:
            break;
        }
    }
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }

    uint64_t n = atoi(argv[1]);
    assert(n > 0);
    uint64_t n_test = n;

    std::vector<uint64_t> test_array(n+2, 0);
    uint64_t test_val = 0;
    SCC_PARALLEL( foo(&n_test, test_array.data(), &test_val) );
    tests::assert_eq(n, n_test);

    uint64_t n_verify = n;
    std::vector<uint64_t> verify_array(n+2,0);
    uint64_t verify_val = 0;
    foo(&n_verify, verify_array.data(), &verify_val);

    tests::assert_eq(n_verify, n_test);
    tests::assert_eq(verify_val, test_val);
    tests::assert_eq(verify_array, test_array);

    return 0;
}
