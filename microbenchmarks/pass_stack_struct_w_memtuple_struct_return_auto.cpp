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
 * Sum the entries a vector of N integers, via log(N) partial sums.
 *
 * This test was originally intended to mimic the libquantum
 * quantum_state_collapse bug, revealed by compiler commit 2317affc0724.
 * The test's key features:
 * 1a) pass a struct by value to a function (getPartialSums)
 * 1b) return the same struct type from the same function
 * 2) pass a pointer to stack-allocated data to a spawned function
 * 3) memtuple runners
 * 4) spanning multiple modules, some of which are C files
 *
 * Following compiler commit 6513064a83b2a6f8, which fixed the 462.libquantum
 * bug, this test still revealed a segfault with an input of 13 (2^13 vector
 * entries). That bug no longer manifests.
 */

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

#include "pass_stack_struct_w_memtuple_struct_return_auto.h"

static std::vector<uint64_t> values;
static uint64_t sum;

// Shamefully copied from swarm/algorithm.h, because
// that file has too many dependences on Swarmish variables
constexpr inline uint32_t ilog2(uint64_t i) { return 63 - __builtin_clzl(i); }


__attribute__((noinline, assertswarmified))
void initialize(S* s) {
    for (uint64_t i = 0; i < s->size; i++) {
        s->components[i].valid = true;
        s->components[i].v = values[i];
    }
}


__attribute__((noinline, assertswarmified))
uint64_t loop() {
    S s;
    s.size = values.size();
    s.components = (Component*)calloc(s.size, sizeof(Component));
    // We expect powers of two
    assert(s.size && !(s.size & (s.size - 1)));

    initialize(&s);

    uint64_t iterations = ilog2(s.size);
    for (uint64_t i = 0; i < iterations; i++) {
        // ignore the return value
        iteration(i, &s);
    }
    assert(s.size == 1);
    sum = s.components[0].v;
    free(s.components);
    return sum;
}


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <log nvalues>" << std::endl;
        return -1;
    }

    values.resize(1ul << atoi(argv[1]), 0ul);
    assert(values.size() > 0);
    std::iota(values.begin(), values.end(), 0ul);
    std::random_shuffle(values.begin(), values.end());

    SCC_PARALLEL( loop() );

    tests::assert_eq(std::accumulate(values.begin(), values.end(), 0ul), sum);
    return 0;
}
