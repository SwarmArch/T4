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

/**
 * The loop increments counters with different stride patterns
 */
#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

constexpr uint64_t NC = 1 << 20;
alignas(SWARM_CACHE_LINE) static uint16_t sixteens[NC];
alignas(SWARM_CACHE_LINE) static uint64_t sixtyfours[NC];
struct FiveBytes {
    uint8_t vals[5];

    bool operator==(const FiveBytes& rhs) const {
        return std::equal(vals, vals + 5, rhs.vals);
    }
};

static std::string to_string(const FiveBytes& fb) {
    return std::accumulate(fb.vals + 1, fb.vals + 5, std::to_string(fb.vals[0]),
                           [](std::string a, uint8_t b) {
                               return std::move(a) + ',' + std::to_string(b);
                           });
}

alignas(SWARM_CACHE_LINE) static FiveBytes forties[NC];

static std::vector<uint8_t> randoms;


void increment(uint64_t size);


static inline int loop_multistride_main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }
    uint64_t size = atoi(argv[1]);
    assert(size && size <= NC);
    std::fill(&sixteens[0], &sixteens[size], 0);
    std::fill(&sixtyfours[0], &sixtyfours[size], 0ul);
    std::fill(&forties[0], &forties[size], FiveBytes({{0,0,0,0,0}}));

    randoms.resize(size);
    std::generate(randoms.begin(), randoms.end(),
                  [] () { return static_cast<uint8_t>(std::rand()); });

    SCC_PARALLEL( increment(size) );

    std::vector<FiveBytes> expectedforties(size, {{0,0,0,0,0}});
    for (uint64_t i = 0; i < expectedforties.size(); i++) {
        expectedforties[i].vals[0] = randoms[i];
    }

    tests::assert_startswith(sixteens, randoms);
    tests::assert_startswith(sixtyfours, randoms);
    tests::assert_startswith(forties, expectedforties);
    return 0;
}
