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
 * Akin to loop_nest_inner_serialized_auto.cpp,
 * but the inner loop has a trivial continuation.
 */

#include <scc/autoparallel.h>
#include "common.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

static std::vector<uint64_t> ones;

__attribute__((noinline, assertswarmified))
static void iota(uint64_t* a, uint64_t rows, uint64_t columns, uint64_t base) {
    assert(columns);
    for (uint64_t i = 0; i < (rows & ~1ul); i += 2) {
        std::iota(&a[i * columns],
                  &a[(i + 1) * columns + 1],
                  base + i * columns);
        for (uint64_t
                *p = &a[(i + 1) * columns + 1],
                *e = &a[(i + 2) * columns];
                p != e; p++) {
            *p = *(p-1) + ones[i];
        }
    }

    if (rows % 2) {
        std::iota(&a[(rows - 1) * columns],
                  &a[rows * columns],
                  base + (rows - 1) * columns);
    }
}


int main(int argc, const char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <rows> <columns>" << std::endl;
        return -1;
    }

    uint64_t rows = atoi(argv[1]);
    uint64_t columns = atoi(argv[2]);
    uint64_t n = rows * columns;
    std::vector<uint64_t> v(n);
    ones.resize(rows, 1ul);

    SCC_PARALLEL( iota(v.data(), rows, columns, 0ul) );

    std::vector<uint64_t> expected(n);
    std::iota(expected.begin(), expected.end(), 0ul);
    tests::assert_eq(expected, v);
    return 0;
}
