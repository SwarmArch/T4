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
 * Deal with short and long memsets.
 */
#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

constexpr uint64_t SMALL = 17;
constexpr uint8_t SMALL_CONTENT = 0xfa;
constexpr uint8_t LARGE_CONTENT = 0xef;


std::array<uintT, SMALL> small;
std::vector<uintT> large;
size_t begin;
size_t end;
void* smallmemsetRet;
void* largememsetRet;


__attribute__((noinline, assertswarmified))
void memsetter(uintT* sbegin, uintT* lbegin, uintT* lend) {
    smallmemsetRet = memset(sbegin, SMALL_CONTENT, SMALL * sizeof(uintT));
    size_t length = (lend - lbegin - 1) * sizeof(uintT);
    largememsetRet = memset(lbegin, LARGE_CONTENT, length);
}


__attribute__((noinline, assertswarmified))
void setup() {
    // These may be converted to memsets too
    std::fill(small.begin(), small.end(), 0ul);
    std::fill(large.begin(), large.end(), 0ul);

    assert(begin < end);
    assert(end <= large.size()); // Exclusive

    memsetter(&small[0],
              large.data() + begin,
              large.data() + end);
}


static inline int memset_main(int argc, const char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <size> <begin> <end>" << std::endl;
        return -1;
    }
    large.resize(atoi(argv[1]));
    begin = atoi(argv[2]);
    end = atoi(argv[3]);

    SCC_PARALLEL( setup() );

    std::array<uintT, SMALL> sverify;
    std::vector<uintT> lverify(large.size(), 0ul);
    memset(&sverify[0], SMALL_CONTENT, sizeof(sverify));
    memset(lverify.data() + begin,
           LARGE_CONTENT,
           (end - begin - 1) * sizeof(uintT));
    tests::assert_eq(sverify, small);
    tests::assert_eq(lverify, large);
    tests::assert_eq((void*)&small[0], smallmemsetRet);
    tests::assert_eq((void*)&large[begin], largememsetRet);
    return 0;
}
