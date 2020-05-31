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

static uint64_t output = 0;

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo(uint64_t *a, uint64_t n) {
  uint64_t last;
  for (uint64_t c = 0; c < n; c++) {
    last = a[c];
  }
  output = last;
}

int main(int argc, const char** argv) {
  uint64_t n = atoi(argv[1]);
  uint64_t *array = new uint64_t[n];
  std::iota(array, array + n, 0ul);

  SCC_PARALLEL( foo(array, n) );

  delete[] array;

  tests::assert_eq(n - 1, output);

  return 0;
}
