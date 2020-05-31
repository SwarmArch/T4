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
 * This test forces interaction of private heap-allocated variables (formerly
 * stack variables) and continuations following a function spawn.
 *
 * Function foo passes its local stack-allocated array of structs (each of which
 * holds an array of ints) to a function reduceThenIncrement, that reads and
 * updates the data. The stack-allocated data should be automatically
 * heap-allocated by the compiler.
 *
 * To make this more challenging, foo recursively calls itself between two calls
 * to reduceThenIncrement. In a sequential program, each recursive instance of
 * foo has its own private stack variables. In an auto-parallelized program,
 * each task instance of foo has its own private heap-allocated variables. If
 * there are any bugs in when the call to free is made, this test fails (as it
 * does in compiler commit f66745905fe3d9).
 */

#include <scc/autoparallel.h>
#include "common.h"

constexpr uint64_t NS = 2;
constexpr uint64_t NC = 50;
constexpr uint64_t RECURSIVE_CALLS = 24;

struct S {
    int8_t a[NC];
    uint64_t x;
};

static uint64_t r = 0;

__attribute__((noinline))
__attribute__((assertswarmified))
static void reduceThenIncrement(S* s) {
    for (int i = 0; i < NS; i++) {
        r = std::accumulate(s[i].a, s[i].a + NC, r);
        for (int j = 0; j < NC; j++) {
            s[i].a[j]++;
        }
    }
}

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo(uint64_t calls) {
    S s[NS];
    for (int i = 0; i < NS; i++) std::iota(s[i].a, s[i].a + NC, 0);
    reduceThenIncrement(s);
    // Recursive call, to generate several sets of (formerly stack- but now)
    // heap-allocated variables. If this task frees those stack variables when
    // it returns, and not in a continuation as it should, then we'll see
    // overwritten data, and a computation error.
    if (calls) foo(calls - 1);
    reduceThenIncrement(s);
}

int main() {
    SCC_PARALLEL( foo(RECURSIVE_CALLS - 1) );

    // 2 * RECURSIVE_CALLS arithmetic series starting from 0
    // 2 * RECURSIVE_CALLS arithmetic series starting from 1
    tests::assert_eq(RECURSIVE_CALLS * (NC - 1) * NC +
                     RECURSIVE_CALLS * NC * (NC + 1),
                     r);
    return 0;
}
