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

#define SIZE (1000ul)

static uint64_t results[SIZE];

__attribute__((noinline))
static void bar(uint64_t* a, uint64_t i) {
  *a = i;
}

__attribute__((noinline))
static void foo() {
    for (int i = 0; i < SIZE; i++) {
      uint64_t a = 0;
      bar(&a, i);

      if (i % 2 == 0) {
        uint64_t b = 0;
        bar(&b, i);
        results[i] = a + b;
      } else {
        uint64_t c = 0;
        bar(&c, -i);
        results[i] = a + c;
      }
    }
}

int main(int argc, const char** argv) {
    SCC_PARALLEL( foo() );

    tests::assert_true(std::all_of(results, results + SIZE,
                                   [] (uint64_t& r) {
                                              return (&r - results) % 2 == 0 ?
                                                (r == 2*(&r - results)) :
                                                (r == 0);}));
    return 0;
}
