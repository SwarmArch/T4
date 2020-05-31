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

#include "call_sret_after_other_calls_in_loop.h"

#include "common.h"

#include <scc/autoparallel.h>

#include <iostream>
#include <cassert>
#include <list>

static std::list<uint64_t> values;

// Force complex bookkeeping and an early loop chain with this function
bool reached_end(std::list<uint64_t>::iterator it, std::list<uint64_t> &l);


__attribute__((noinline))
__attribute__((assertswarmified))
static uint64_t run() {
    std::iota(values.begin(), values.end(), 0ul);

    auto it = values.begin();
    while (!reached_end(it, values)) {
        uint64_t& v = *(it++);
        uint64_t one = single_one();
        SeveralInts_t si = all_ones_sret();
        uint64_t five = si.one + si.two + si.three + si.four + si.five;
        v += five + one;
    }
    return std::accumulate(values.begin(), values.end(), 0ul);
}


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }

    values.resize(atoi(argv[1]));
    uint64_t n = values.size();
    assert(n > 0);

    uint64_t sum = -1;
    SCC_PARALLEL({
        sum = run();
    });

    tests::assert_eq((n * (n - 1) / 2) + 6 * n, sum);
    return 0;
}
