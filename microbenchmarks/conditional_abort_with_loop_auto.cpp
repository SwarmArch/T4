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

#include <cstdio>


static uint64_t c;

__attribute__((noinline))
__attribute__((assertswarmified))
static void do_work() {
  c++;
}

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
  if (c) {
    // This loop should not be parallelized, since this path inevitably aborts.
    for (int i = 0; i < 42; i++) {
      printf("This should not happen.\n");
      std::fflush(nullptr);
    }
    std::abort();
  }
  do_work();
}

int main(int argc, const char** argv) {
  c = 0;
  SCC_PARALLEL( foo() );

  tests::assert_true(c == 1);

  return 0;
}
