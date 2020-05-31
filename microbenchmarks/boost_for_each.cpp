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
#include <boost/iterator/counting_iterator.hpp>
#include <cstdint>
#include <swarm/swarm.h>

#include "swarm/api.h"
#include "common.h"

constexpr uint64_t N = 1024;
static uint64_t counters[N];

__attribute__((noinline))
static void foo() {
  std::for_each(
      boost::counting_iterator<uint64_t>(0ul),
      boost::counting_iterator<uint64_t>(N),
      [] (uint64_t i) {
        swarm_spawn (0) { counters[i]++; }
      });
}

int main() {
  swarm_spawn (0) foo();
  swarm::run();

  uint64_t verify[N];
  std::fill(verify, verify + N, 1ul);
  tests::assert_true(std::equal(verify, verify + N, counters));
  return 0;
}
