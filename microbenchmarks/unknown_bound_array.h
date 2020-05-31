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

// This test mimics an important loop in 482.sphinx3, where the loop bound is
// unknown and determined by an array of values:
// https://github.mit.edu/swarm/benchmarks/blob/592f51372b2c897a64d8ad196fde42a00ae78401/speccpu2006/482.sphinx3/cont_mgau.c#L642-L668
//
// As of compiler swarm/Swarm-IR@c2c72dba25a040862b3c206ab the loop of interest
// is chained by LoopExpansion because:
// microbenchmarks/scc/unknown_bound_array.h:19:5: remark: failed to strengthen all IVs other than the canonical one. [-Rpass-analysis=swarm-loop-expansion]
// microbenchmarks/scc/unknown_bound_array.h:19:5: remark: pipelining chain of iterations [-Rpass=swarm-loop-expansion]

#include <scc/autoparallel.h>
#include "common.h"

#include <cstdint>
#include <vector>


__attribute__((noinline, assertswarmified))
static void increment_actives(const uint_t* active, uint64_t* counter) {
    for (uint64_t i = 0; active[i]; i++) {
        counter[i] += active[i];
    }
}


static inline int unknown_bound_array(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }

    const uint64_t n = atoi(argv[1]);
    std::vector<uint64_t> counter(n, 0);
    std::vector<uint_t> active(n, 1);
    active[n-1] = 0;

    SCC_PARALLEL( increment_actives(active.data(), counter.data()) );

    tests::assert_eq(active, counter);
    return 0;
}
