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
 * Deal with short and long memcpys.
 */
#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

constexpr uint64_t SMALL = 17;

std::array<uintT, SMALL> small;
std::vector<uintT> large;
void* smallmemcpyRet;
void* largememcpyRet;


__attribute__((noinline, assertswarmified))
void memcopier(const uintT* sbegin, const uintT* lbegin, const uintT* lend) {
    smallmemcpyRet = memcpy(&small[0], sbegin, SMALL * sizeof(uintT));
    size_t length = (lend - lbegin) * sizeof(uintT);
    largememcpyRet = memcpy(&large[0], lbegin, length);
}


static inline int memcpy_main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <size>" << std::endl;
        return -1;
    }
    large.resize(atoi(argv[1]));

    std::array<uintT, SMALL> ssource;
    std::vector<uintT> lsource(large.size(), 0ul);
    for (uint64_t i = 0; i < SMALL; i++) ssource[i] = rand();
    for (uint64_t i = 0; i < large.size(); i++) lsource[i] = rand();

    SCC_PARALLEL({
        memcopier(&ssource[0], &lsource[0], &lsource[lsource.size()]);
    });

    tests::assert_eq(ssource, small);
    tests::assert_eq(lsource, large);
    tests::assert_eq((void*)&small[0], smallmemcpyRet);
    tests::assert_eq((void*)&large[0], largememcpyRet);
    return 0;
}
