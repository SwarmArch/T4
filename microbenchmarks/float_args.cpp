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

// A simple test of passing floating point arguments to tasks.

#include <swarm/swarm.h>

#include "swarm/api.h"  // For access to swarm::run() and some some types
#include "common.h"

double r;

__attribute__((noinline))
void bar(double x, double y, double z) {
  swarm_spawn(swarm::timestamp()) {
    r += x + z;
  }
}

__attribute__((noinline))
void foo(double d) {
  swarm_spawn (1) bar(d, d + d, d * 8.0);
  swarm_spawn (2) bar(d * d, d + 2.0, d * 4.0);
}

int main(int argc, const char** argv) {

  swarm_spawn (0) foo(1.0);

  swarm::run();

  tests::assert_eq(14.0, r);

  return 0;
}
