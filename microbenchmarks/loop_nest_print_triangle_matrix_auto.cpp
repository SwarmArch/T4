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
 * Build a triangular matrix, printf its values in a doubly nested loop.
 *
 * This reproduces the segfault in swarm/Swarm-IR#466 given an input of 3 or 4,
 * and using compiler swarm/Swarm-IR@850133fdd974f2e2d52876a or earlier.
 * The code is intended to mimic
 * https://github.mit.edu/swarm/benchmarks/blob/117526cd1fff91f5953d44b801b830c8566cfabd/speccpu2006/482.sphinx3/subvq.c#L261-L266
 *
 * In particular the test consists of
 * 1) an outer loop that is progressively expanded due to unknown bounds
 *    due to the opaque printf() calls.
 * 2) an inner loop that is eagerly chained
 *
 * See the postmortem explaining the issue this microbenchmark surfaced at:
 * https://github.mit.edu/swarm/Swarm-IR/pull/486#discussion_r13526
 */

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>


// Really just a wrapper for the size of one row
struct Row {
    uint32_t dummy;
    uint8_t dummy2;
    uint64_t size;
};

struct TriangleMatrix {
    const uint64_t nrows;
    Row *const rows;
    uint32_t **const data;

    TriangleMatrix(uint64_t n)
        : nrows(n)
        , rows(new Row[n])
        , data(new uint32_t*[n])
    {
        data[0] = new uint32_t[nrows * (nrows + 1) / 2];
        for (uint64_t i = 0; i < nrows; i++) {
            rows[i].size = i + 1;
            data[i] = data[0] + (i * (i + 1)) / 2;
        }
    }

    ~TriangleMatrix() {
        delete [] data[0];
        delete [] data;
        delete [] rows;
    }
};


__attribute__((noinline, assertswarmified))
void flatten(const TriangleMatrix* tm, uint32_t* flat) {
    for (uint64_t i = 0; i < tm->nrows; i++) {
        for (uint64_t j = 0; j < tm->rows[i].size; j++) {
            printf(" %d", tm->data[i][j]);
            flat[j + (i * (i + 1)) / 2] = tm->data[i][j];
        }
        printf("\n");
    }
}


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }
    const uint64_t n = atoi(argv[1]);
    const uint64_t nnp1d2 = n * (n + 1) / 2;
    std::vector<uint32_t> flat(nnp1d2);
    TriangleMatrix tm(n);
    std::iota(tm.data[0], tm.data[0] + nnp1d2, 1ul);

    SCC_PARALLEL( flatten(&tm, flat.data()) );

    std::vector<uint32_t> expected(nnp1d2);
    std::iota(expected.begin(), expected.end(), 1ul);
    tests::assert_eq(expected, flat);

    return 0;
}
