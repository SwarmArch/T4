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
 * A read-write iteration over a linked list
 */

#include "common.h"

#include "swarm/aligned.h"
#include <scc/autoparallel.h>
#include <swarm/swarm.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>

using list_aligned_u64 = std::list<swarm::aligned<uint64_t>>;

__attribute__((noinline))
__attribute__((assertswarmified))
void filler(list_aligned_u64& l, uint64_t value) {
    std::fill(l.begin(), l.end(), value);
}

int main(int argc, const char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <size> <value>" << std::endl;
        return -1;
    }

    list_aligned_u64 l(atoi(argv[1]));
    assert(!l.empty());
    const uint64_t value = atoi(argv[2]);

    SCC_PARALLEL( filler(l, value) );

    list_aligned_u64 expected(l.size());
    std::fill(expected.begin(), expected.end(), value);
    assert(expected.size() == l.size());
    tests::assert_true(std::equal(l.begin(), l.end(), expected.begin()));
    return 0;
}
