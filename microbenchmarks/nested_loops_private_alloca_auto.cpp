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

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <scc/autoparallel.h>
#include "common.h"

#define NC (42)

struct BunchOfVals {
    uint32_t vals[NC];
};


__attribute__((noinline, assertswarmified))
void copy_and_double(const BunchOfVals* src, BunchOfVals* dst, uint64_t n) {
    for (uint64_t i = 0; i < n; i++) {
        BunchOfVals local = src[i];
        for (uint64_t j = 0; j < NC; j++) local.vals[j] *= 2;
        dst[i] = local;
    }
}


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }

    uint64_t n = atoi(argv[1]);
    BunchOfVals* originals = new BunchOfVals[n];
    BunchOfVals* doubles = new BunchOfVals[n];
    for (uint64_t i = 0; i < n; i++) {
        for (uint64_t j = 0; j < NC; j++) {
            originals[i].vals[j] = std::rand() % UINT16_MAX;
        }
    }

    SCC_PARALLEL( copy_and_double(originals, doubles, n) );

    auto rightIsDouble = [] (const BunchOfVals& l, const BunchOfVals& r) {
        return std::equal(l.vals, l.vals + NC, r.vals,
                          [] (uint32_t l, uint32_t r) { return 2 * l == r; });
    };
    tests::assert_true(std::equal(originals, originals + n,
                                  doubles, rightIsDouble));
    delete [] originals;
    delete [] doubles;

    return 0;
}
