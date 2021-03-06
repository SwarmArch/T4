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

#include <algorithm>

#include <scc/autoparallel.h>
#include "common.h"

#define SIZE (1000)

static uint64_t results[SIZE];

__attribute__((noinline))
static void baz(uint64_t* a, uint64_t* b, uint64_t* c, int i) {
  // make sure a, b, and c don't overlap
  *b = i;
  *a = std::numeric_limits<uint64_t>::max();
  *c = std::numeric_limits<uint64_t>::max();
}

__attribute__((noinline))
static uint64_t bar(int i) {
  uint64_t a;
  uint64_t b;
  uint64_t c;
  baz(&a, &b, &c, i);

  return b;
}

__attribute__((noinline))
static void foo() {
    // allocate things to the stack many times
    for (int i = 0; i < SIZE; i++) {
      results[i] = bar(i);
    }
}

int main(int argc, const char** argv) {
    SCC_PARALLEL( foo() );

    tests::assert_true(std::all_of(results, results + SIZE,
                                   [] (uint64_t& r) {return r == (&r - results);}));

    return 0;
}
