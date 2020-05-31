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

// A test of compiler-generated memtuple runners.

#include <swarm/swarm.h>

#include "swarm/api.h"  // For access to swarm::run() and some some types
#include "common.h"

static int r;

volatile int args[10];

__attribute__((noinline)) void init_args() {
  int i = 0;
  for (auto& arg : args) arg = ++i;
}

__attribute__((noinline)) int sum(int aa, int bb, int cc, int dd, int ee,
                                  int ff, int gg, int hh, int ii, int jj) {
    int sum = aa + bb + cc + dd + ee + ff + gg + hh + ii + jj;
    args[9] = sum;  // force a volatile mem op
    return sum;
}

int main(int argc, const char** argv) {
  volatile int a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, j=10;
  init_args();
  int aa = args[0], bb = args[1], cc = args[2], dd = args[3], ee = args[4],
      ff = args[5], gg = args[6], hh = args[7], ii = args[8], jj = args[9];

  swarm_spawn (0) r = a+b+c+d+e+f+g+h+i+j;  // mem_runner
  swarm_spawn (1) r += sum(aa, bb, cc, dd, ee, ff, gg, hh, ii, jj);  // reg_runner

  swarm::run();

  tests::assert_eq((10 * 11), r);
  return 0;
}
