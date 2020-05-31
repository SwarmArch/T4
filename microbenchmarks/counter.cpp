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

#include <stdlib.h>
#include <array>
#include <iostream>
#include <algorithm>
#include <swarm/swarm.h>

#include "swarm/api.h"  // For access to swarm::run() and some some types
#include "common.h"


#define NC (32768ul)
static std::array<uint64_t, NC> counters alignas(SWARM_CACHE_LINE);


static void task(uint64_t c, uint64_t upsLeft) {
    uint64_t ts = swarm::timestamp();
    counters[c]++;
    if (pls_likely(--upsLeft)) {
        swarm_spawn (ts+c+1) task(c, upsLeft);
    }
}


static int counter(int argc, const char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <nupdates> [<ncounters>]" << std::endl;
        std::abort();
    }

    uint64_t nupdates = atoi(argv[1]);
    uint64_t ncounters = (argc == 3)? atoi(argv[2]) : 128;

    assert(ncounters > 0);
    assert(ncounters <= NC);

    std::fill(counters.begin(), counters.end(), 0ul);

    uint64_t upc = nupdates / ncounters;
    uint64_t rem = nupdates % ncounters;
    swarm_spawn (0) {
        for (uint64_t c = 0; c < ncounters; c++) {
            uint64_t updates = upc + (c < rem);
            if (updates)
                swarm_spawn (0) task(c, updates);
        }
    }

    swarm::run();

    printf("Counters:\n");
    uint64_t last = counters[0];
    uint64_t lastStart = 0;
    for (uint64_t c = 0; c < ncounters; c++) {
        uint64_t v = counters[c];
        if (v != last) {
            printf(" %3ld - %3ld:  %ld\n", lastStart, c-1, last);
            last = v;
            lastStart = c;
        }
    }
    if (lastStart < ncounters-1) {
        printf(" %3ld - %3ld:  %ld\n", lastStart, ncounters-1, last);
    }

    tests::assert_eq(nupdates,
                     std::accumulate(counters.begin(), counters.end(), 0ul));
    return 0;
}

int main(int argc, const char** argv) {
    return counter(argc, argv);
}
