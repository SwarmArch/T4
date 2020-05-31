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

// Our simplest test of swarm_spawn syntax.
// If this fails, we have problems in very basic clang code gen, loop expansion,
// or lowering, or some other basic handling of Swarm tasks, as this does not
// rely on memtuple runners, closures of any sort, passing lots of arguments,
// domain atomicity, or even the timestamps.

#include <swarm/swarm.h>
#include <array>

#include "common.h"
#include "swarm/api.h"  // For access to swarm::run() and some some types

#define NC (7ul)

static std::array<uint64_t, NC> counters;

int main(int argc, const char** argv) {
  swarm_spawn (0)
      for (uint64_t c = 0; c < NC; c++) {
          swarm_spawn (c) counters[c]++;
      }

  swarm::run();

  std::array<uint64_t, NC> verify;
  std::fill(verify.begin(), verify.end(), 1ul);
  tests::assert_eq(verify, counters);
  return 0;
}
