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

/*
 * This test replicates a loop in 473.astar's Way_.cpp, with a sequence of
 * adjacent manual tasks that spawn nested tasks.
 */

#include <algorithm>
#include <swarm/swarm.h>
#include <vector>

#include <scc/autoparallel.h>
#include "common.h"

constexpr uint64_t NC = 128;
constexpr uint64_t UNROLL = 8;
constexpr uint64_t MOD = 4;

__attribute__((always_inline))
static void maybeAppendToOut(uint64_t* in, uint64_t** out) {
  // Load *in covered by a hint.
  swarm_spawn {
    uint64_t v = *in;
    if (!(v % MOD)) {
      // Serialize updates to *out via a hint.
      swarm_spawn {
        **out = v;
        (*out)++;
      }
    }
  }
}


__attribute__((noinline, assertswarmified))
static void collect_one_in_MOD(uint64_t* in, uint64_t* out) {
    for (uint64_t i = 0; i < NC; i += UNROLL) {
        maybeAppendToOut(&in[i], &out);
        maybeAppendToOut(&in[i + 1], &out);
        maybeAppendToOut(&in[i + 2], &out);
        maybeAppendToOut(&in[i + 3], &out);
        maybeAppendToOut(&in[i + 4], &out);
        maybeAppendToOut(&in[i + 5], &out);
        maybeAppendToOut(&in[i + 6], &out);
        maybeAppendToOut(&in[i + 7], &out);
    }
}


int main() {
    std::vector<uint64_t> in(NC);
    std::vector<uint64_t> out(NC / MOD);
    std::iota(in.begin(), in.end(), 1);

    SCC_PARALLEL( collect_one_in_MOD(in.data(), out.data()) );

    std::vector<uint64_t> expected(NC / MOD);
    for (uint64_t i = 0; i < NC / MOD; i++) expected[i] = (i + 1) * MOD;
    tests::assert_eq(expected, out);
    return 0;
}
