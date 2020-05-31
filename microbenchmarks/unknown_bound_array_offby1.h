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

// A test similar to unknown_bound_array, but the indexing is off by one.

#include <scc/autoparallel.h>
#include "common.h"

#include <cstdint>
#include <vector>


__attribute__((noinline, assertswarmified))
static void increment_actives(const uint_t* active, uint64_t* counter) {
    for (uint64_t i = 1; active[i - 1]; i++) {
        counter[i] += active[i];
    }
}


static inline int unknown_bound_array(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }

    const uint64_t n = atoi(argv[1]);
    std::vector<uint64_t> counter(n, 0);
    std::vector<uint_t> active(n, 1);
    active[n - 1] = 0;

    SCC_PARALLEL( increment_actives(active.data(), counter.data()) );

    std::vector<uint64_t> expected(n, 1);
    expected[0] = 0;
    expected[n - 1] = 0;
    tests::assert_eq(expected, counter);
    return 0;
}
