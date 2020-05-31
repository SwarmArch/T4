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

// A test where the compiler has no choice but to use early chain form.

#include <scc/autoparallel.h>
#include "common.h"

#include <array>

#define NC (1013ul)

static std::array<uint64_t, NC+1> counters;

__attribute__((noinline))
__attribute__((assertswarmified))
uint64_t increment(uint64_t x) {
  // Perform a volatile store just so the compiler can't conclude anything
  // about this function's memory access properties.
  static volatile uint64_t increment_size;
  increment_size = 1;
  return x + increment_size;
}

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
    // There's no getting around the call to increment(), which is part of
    // the bookkeeping and can't be detached in canonical form. However,
    // as far as the compiler knows when it's parallelizing foo(), increment()
    // could be a big function that needs to be spawned.
    for (uint64_t c = 0; c < NC; c = increment(c)) {
        counters[c]++;
    }
}

int main() {
    SCC_PARALLEL( foo() );

    std::array<uint64_t, NC+1> verify;
    verify.fill(1ul);
    verify[NC] = 0;
    tests::assert_eq(verify, counters);

    return 0;
}
