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

#include <scc/autoparallel.h>
#include "common.h"

#include <array>

#define NC (1013ul)

static std::array<uint64_t, NC+1> counters;

__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
    #pragma clang loop expand(disable)
    for (uint64_t c = 0; c < NC; c++) {
        counters[c]++;
    }
}

int main(int argc, const char** argv) {
    SCC_PARALLEL( foo() );

    std::array<uint64_t, NC+1> verify;
    verify.fill(1ul);
    verify[NC] = 0;
    tests::assert_eq(verify, counters);

    return 0;
}
