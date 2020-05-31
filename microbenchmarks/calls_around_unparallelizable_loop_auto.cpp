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
#include <swarm/swarm.h>

#include <algorithm>
#include <array>

#define NC (2018UL)

std::array<uint64_t, NC> counters;

__attribute__((noinline))
__attribute__((assertswarmified))
void do_work(uint64_t v) {
    for (uint64_t i = 0; i < NC; i++) {
        counters[i] += v;
    }
}

__attribute__((noinline))
__attribute__((assertswarmified))
uint64_t square(uint64_t x) {
  counters[42] += 1; // just so this function isn't marked read-only
  return x * x;
}

__attribute__((noinline))
__attribute__((assertswarmified))
void foo() {
    static volatile uint64_t x;
    // Don't consume the return value; these two should easily run in parallel
    do_work(1);
    x = 2;
    while (x < 10) { // This loop is unparallelizable
      // This call to square is a spawnsite that forces fractalization
      x = square(x);
    }
    do_work(x);
}

int main(int argc, const char** argv) {
    SCC_PARALLEL( foo() );

    static std::array<uint64_t, NC> verify;
    std::fill(verify.begin(), verify.end(), 17UL);
    verify[42] += 2;
    tests::assert_eq(verify, counters);
    return 0;
}
