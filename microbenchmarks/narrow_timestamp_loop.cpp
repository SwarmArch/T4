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
#include <swarm/swarm.h>

#include "swarm/api.h"  // For access to swarm::run() and some some types
#include "swarm/algorithm.h"
#include "common.h"

#define NC (240)

static uint64_t counters[NC];

static void foo() {
  // This loop is written with a 64-bit induction variable,
  // although it would be sufficient to use a 32-bit one.
  for (uint64_t c = 0; c < NC; c++) swarm_spawn((uint32_t)c) {
    counters[c]++;
  }
}

int main(int argc, const char** argv) {
  swarm_spawn (0) foo();

  swarm::run();

  tests::assert_true(std::all_of(swarm::u64it(0), swarm::u64it(NC),
                                 [] (uint64_t i) {return counters[i] == 1;}));

  return 0;
}
