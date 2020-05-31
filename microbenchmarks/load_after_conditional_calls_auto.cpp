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
 * What happens when a load appears after a call protected by a condition?
 */
#include "common.h"

#include <scc/autoparallel.h>

#include <algorithm>

static uint64_t counter;
static uint64_t queries[3] = {0, 0, 0};

__attribute__((noinline, assertswarmified))
static void increment() {
    counter++;
}

__attribute__((noinline, assertswarmified))
static void foo(bool p, bool q) {
    if (!p && !q) increment();
    if (!p && q) increment();
    queries[2*p + q] = counter;
}

int main() {
    counter = 0;

    SCC_PARALLEL({
        foo(false, false);
        foo(false, true);
        foo(true, false);
    });

    //for (int q = 0; q < 3; ++q)
    //  printf("queries[%d] == %ld\n", q, queries[q]);
    tests::assert_true(
            queries[0] == 1ul &&
            queries[1] == 2ul &&
            queries[2] == 2ul &&
            counter == 2
            );
    return 0;
}
