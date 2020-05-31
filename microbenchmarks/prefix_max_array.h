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

#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <vector>
#include <scc/autoparallel.h>
#include "common.h"


__attribute__((noinline, assertswarmified))
static int_t foo(int_t* a, index_t n) {
    int_t max = std::numeric_limits<int_t>::min();
    for (index_t c = 0; c < n; c++) {
        max = std::max(max, a[c]);
        a[c] = max;
    }
    return max;
}

static inline int prefix_max_array_main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }

    const index_t n = atoi(argv[1]);
    std::vector<int_t> orig(n);
    std::vector<int_t> prefix_max(n);
    std::mt19937 gen;
    std::uniform_int_distribution<> dis(0, 42);
    for (int_t& i : orig) i = dis(gen);
    std::copy(orig.begin(), orig.end(), prefix_max.begin());
    int_t max;

    SCC_PARALLEL({
        max = foo(prefix_max.data(), n);
    });

    auto max_lambda = [] (int_t l, int_t r) { return std::max(l, r); };
    int_t expected_max = std::accumulate(orig.begin(), orig.end(),
                                         std::numeric_limits<int_t>::min(),
                                         max_lambda);
    std::vector<int_t> expected_prefix_max(n);
    std::partial_sum(orig.begin(), orig.end(), expected_prefix_max.begin(),
                     max_lambda);
    tests::assert_eq(expected_max, max);
    tests::assert_eq(expected_prefix_max, prefix_max);
    return 0;
}
