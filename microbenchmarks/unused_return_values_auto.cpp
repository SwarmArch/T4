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

#include "common.h"
#include <scc/autoparallel.h>

#include <algorithm>
#include <array>

#define NC (579ul)

static std::array<uint64_t, NC> counters;

__attribute__((noinline))
__attribute__((assertswarmified))
static int do_nothing() {
  static int ret = 0;
  return ret++;
}

__attribute__((noinline))
__attribute__((assertswarmified))
static uint64_t do_work(uint64_t v) {
    do_nothing();
    for (uint64_t i = 0; i < NC; i++) {
        counters[i] += v;
    }
    do_nothing();
    do_nothing();
    return v - 1;
}

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
    // Don't consume the return value; these two should easily run in parallel
    do_work(1);
    do_work(2);
    // This third call should run in parallel with the first two, but the fourth
    // should run after the third because it uses the return value.
    // TODO(mcj) we should be able to assert those statements with more control
    uint64_t r = do_work(3);
    do_work(r);
}

int main(int argc, const char** argv) {
    SCC_PARALLEL({
        foo();
    });

    static std::array<uint64_t, NC> verify;
    std::fill(verify.begin(), verify.end(), 8ul);
    tests::assert_eq(verify, counters);
    return 0;
}
