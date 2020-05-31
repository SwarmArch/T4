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
 * This tests how values are captured and passed from before to after a
 * progressively expanded loop (i.e. to its continuation).
 * It successfully reveals a use-after-free bug in swarm/Swarm#618
 */

#include <scc/autoparallel.h>
#include "common.h"

#include <cassert>
#include <cstdint>
#include <vector>

constexpr uint64_t REPEATS = 49;

extern "C" {
// Implemented in the C module
unsigned two();
}


__attribute__((noinline, assertswarmified))
static uint64_t count_actives_times_two(const uint8_t* active) {
    uint64_t sum = 0;
    for (uint64_t r = 0; r < REPEATS; r++) {
        // The inner while loop is progressive enqueued as the bound is unknown.
        // Variables two_ and active are used both before and after the loop.
        const uint64_t two_ = two();
        uint64_t local = two_ - 2ul;
        for (uint64_t i = 0; active[i]; i++) {
            local += active[i];
        }
        // 2 * local / 1
        sum += (two_ * local / active[0]);
    }
    return sum;
}


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }

    const uint64_t n = atoi(argv[1]);
    assert(n);
    std::vector<uint8_t> active(n, 1);

    uint64_t count;
    SCC_PARALLEL({
        count = count_actives_times_two(active.data());
    });

    uint64_t expected = REPEATS * 2 * n;
    tests::assert_eq(expected, count);
    return 0;
}
