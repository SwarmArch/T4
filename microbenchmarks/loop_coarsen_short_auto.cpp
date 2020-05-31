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

// A test of whether coarsened loops can correctly execute small numbers of iterations.

#include <scc/autoparallel.h>
#include "common.h"

#include <array>

#define NC (25ul)

static std::array<uint64_t, NC + 1> counters;

__attribute__((noinline))
__attribute__((assertswarmified))
void loop(size_t elems) {
  for (int i = 0; i < elems; i++)
    counters[i]++;
}

__attribute__((noinline))
__attribute__((assertswarmified))
void test(size_t elems) {
  for (int i = 0; i < elems; i++)
    loop(i);
}

int main(int argc, const char* argv[]) {
  size_t elems = atoi(argv[1]);
  elems = std::min(NC, elems);
  SCC_PARALLEL( test(elems) );

  //std::cout << "initial loop address: " << &counters[1] << " offset: " << ((((uintptr_t)&counters[1]) & 63) / 8) << std::endl;
  std::array<uint64_t, NC + 1> verify;
  verify.fill(0);
  for (int i = elems-1; i > 0; i--)
    verify[i - 1] = verify[i] + 1;
  tests::assert_eq(verify, counters);
}
