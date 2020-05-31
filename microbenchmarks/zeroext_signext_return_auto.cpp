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
 * One function's return value is zero-extended, the other's is sign-extended.
 */
#include <algorithm>
#include <cassert>
#include <iostream>
#include <array>

#include "common.h"
#include <scc/autoparallel.h>

constexpr uint64_t NC = 1ul << 16;

volatile bool zero = false;
volatile int8_t negthree = -3;

__attribute__((noinline, assertswarmified))
bool zeroextend(uint64_t* begin, uint64_t size) {
    std::for_each(begin, begin + size, [] (uint64_t& c) { c++; });
    return zero;
}


__attribute__((noinline, assertswarmified))
int8_t signextend(uint64_t* begin, uint64_t size) {
    std::for_each(begin, begin + size, [] (uint64_t& c) { c++; });
    return negthree;
}


__attribute__((noinline, assertswarmified))
void caller(uint64_t* begin, uint64_t size) {
    uint64_t midpoint = size / 2;
    uint64_t shouldBeZero = zeroextend(begin, midpoint);
    if (shouldBeZero) std::abort();
    int64_t shouldBeNegative = signextend(begin + midpoint, size - midpoint);
    if (shouldBeNegative >= 0) std::abort();
}


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }
    std::array<uint64_t, NC> v;
    std::fill(v.begin(), v.end(), 0ul);
    uint64_t size = atoi(argv[1]);
    assert(size < NC);

    SCC_PARALLEL( caller(&v[0], size) );

    std::array<uint64_t, NC> expected;
    std::fill(expected.begin(), std::next(expected.begin(), size), 1ul);
    std::fill(std::next(expected.begin(), size), expected.end(), 0ul);
    tests::assert_eq(expected, v);
    return 0;
}
