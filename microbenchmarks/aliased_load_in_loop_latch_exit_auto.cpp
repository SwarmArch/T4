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

// This test loads a value used to determine the exit condition
// at the loop latch.
// Unlike the load_after_* tests, the load is not followed by a store
// to some other in-memory variable, so the load isn't part of
// the body tasks of the loop, but is actually part of the latch bookkeeping.
// Unlike the unknown_bound_* tests, where no pointers actually alias,
// here the pointers do actually alias, so it is important that the load
// remains correctly ordered after the loop body.

#include "common.h"

#include <scc/autoparallel.h>

#include <vector>

// Calls to this noinline function are spawned, thus testing the timestamp
// ordering of these increments relative to the loads in the loop latch.
__attribute__((noinline))
__attribute__((assertswarmified))
void increment(uint64_t *x) { ++*x; }

__attribute__((noinline))
__attribute__((assertswarmified))
void foo(uint64_t a[], uint64_t b[], uint64_t size) {
    uint64_t c = 0;
    do {
        increment(&a[c]);
    // The exit condition aliases with the next loop body iteration.
    } while (b[++c] == 0);
}

__attribute__((noinline))
__attribute__((assertswarmified))
void bar(uint64_t a[], uint64_t b[], uint64_t size) {
    uint64_t c = 0;
    do {
        increment(&a[c]);
    // The exit condition aliases with the previous loop body iteration.
    } while (b[c++] == 1);
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return 1;
    }
    uint64_t size = atoi(argv[1]);
    std::vector<uint64_t> test_foo(size + 1, 0UL);
    test_foo[size] = 2;
    std::vector<uint64_t> test_bar(size + 1, 0UL);
    test_bar[size - 1] = 2;

    SCC_PARALLEL({
        foo(test_foo.data(), test_foo.data(), size);
        bar(test_bar.data(), test_bar.data(), size);
    });

    std::vector<uint64_t> verify(size + 1, 1UL);
    verify[size] = 2;
    tests::assert_eq(verify, test_foo);
    verify[size - 1] = 3;
    verify[size] = 0;
    tests::assert_eq(verify, test_bar);

    return 0;
}
