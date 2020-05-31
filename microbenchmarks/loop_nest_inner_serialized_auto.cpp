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
 * The first inner loop of iota represents a serial chain of loop-carried
 * dependences on memory. The inner loop should be serialized by the compiler,
 * and this test is exploring the parallelization of the outer loop iteration.
 * The test is inspired by the dc[k] loop in 456.hmmer/fast_algorithms.c.
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
    // This function is a dumb way to implement std::iota where the first inner
    // loop has a memory-based loop-carried dependence which should cause the
    // compiler to serialize it, and the second inner loop uses std::iota which
    // SCC should parallelize.
    assert(columns);
    a[0] = base;

    // The outer loop should be easily parallelized
    for (uint64_t i = 0; i < (rows & ~1ul); i += 2) {
        // This inner loop is serial
        for (uint64_t
                *p = &a[i * columns + 1],
                *e = &a[(i + 1) * columns];
                p != e; p++) {
            // Fill the row using a serial chain of dependences.
            // Load from ones otherwise the compiler recognizes this is simply a
            // striding store of an affine function.
            *p = *(p-1) + ones[i];
            // TODO(mcj) Ideally this would also test having an unreachable exit
            // within the serialized inner loop, but the compiler loses it over
            // multiple exit paths, so this is commented out for now.
            // if (a[i * columns] != (base + i * columns)) std::abort();
        }

        // Add a non-trivial continuation so that the loop above is spawned
        // This inner std::iota should be highly parallel
        std::iota(&a[(i + 1) * columns],
                  &a[(i + 2) * columns + 1],
                  base + (i + 1) * columns);
    }

    if (rows % 2) {
        std::iota(&a[(rows - 1) * columns + 1],
                  &a[rows * columns],
                  base + (rows - 1) * columns + 1);
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
