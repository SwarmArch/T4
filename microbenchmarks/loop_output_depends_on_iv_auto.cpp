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
 * The outer loop of foo has an output phi which complicates its
 * parallelization. This test was intended to reproduce swarm/Swarm-IR#441
 * (the first outer loop of P7Viterbi in 456.hmmer/fast_algorithms.c). In that
 * loop the loop output phi's incoming instruction has operands that are all
 * loop invariant (likely created as a serial post-loop optimization).
 * Unfortunately that case is hard to reproduce. In this resulting test, the
 * output phi depends on an induction phi in foo's outer loop. As of
 * swarm/Swarm-IR@19edf838ad5720c0b09a7db031b46769f9cdcacb, the outer loop is
 * eagerly chained.
 */
#include <numeric>

#include <scc/autoparallel.h>
#include "common.h"

volatile uint64_t rows = 9;

__attribute__((noinline, assertswarmified))
static void setup(uint64_t*** pa, uint64_t rows, uint64_t cols) {
    uint64_t* buffer = new uint64_t[rows * cols];
    std::iota(buffer, buffer + rows * cols, 0ul);
    uint64_t** ptrs = new uint64_t*[rows];
    for (uint64_t i = 0; i < rows; i++) ptrs[i] = &buffer[i * cols];
    *pa = ptrs;
}


__attribute__((noinline, assertswarmified))
static void destroy(uint64_t*** pa) {
    uint64_t** ptrs = (*pa);
    uint64_t* buffer = ptrs[0];
    delete [] buffer;
    delete [] ptrs;
}


__attribute__((noinline, assertswarmified))
static uint64_t foo(uint64_t rows, uint64_t cols) {
    uint64_t** a;
    setup(&a, rows, cols);
    uint64_t r;
    // Increment all entries except those of the last row
    for (r = 0; r < rows - 1; r++) {
        uint64_t* row = a[r];
        for (uint64_t c = 0; c < cols; c++) {
            row[c]++;
        }
    }
    // Return the sum of the first and last entries of the table
    // (the first entry was incremented, the last was not)
    uint64_t ret = a[r][cols - 1] + a[0][0];
    destroy(&a);
    return ret;
}


int main(int argc, const char** argv) {
    uint64_t cols = atoi(argv[1]);

    uint64_t output;
    SCC_PARALLEL({
        output = foo(rows, cols);
    });

    tests::assert_eq(rows * cols, output);

    return 0;
}
