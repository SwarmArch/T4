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
 * readnoner's end parameter is given the attribute readnone
 */
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

__attribute__((noinline, assertswarmified))
void readnoner(uint16_t* begin, uint16_t* end) {
    std::for_each(begin, end, [] (uint16_t& c) { c++; });
}


__attribute__((noinline, assertswarmified))
void caller(std::vector<uint16_t>& v) {
    uint16_t* begin = v.data();
    uint16_t* endm1 = v.data() + v.size() - 1;
    readnoner(begin, endm1);
    // endm1 should be deemed readnone in readnoner,
    // but ensure that its update here is spawned successfully.
    (*endm1)++;
}


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }
    std::vector<uint16_t> v(atoi(argv[1]), 0);
    assert(!v.empty());

    SCC_PARALLEL( caller(v) );

    std::vector<uint16_t> ones(v.size(), 1);
    tests::assert_eq(ones, v);
    return 0;
}
