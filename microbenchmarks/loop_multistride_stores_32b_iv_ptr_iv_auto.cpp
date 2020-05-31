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

#include "loop_multistride_stores.h"

__attribute__((noinline, assertswarmified))
void increment(uint64_t size) {
    uint8_t* pr = &randoms[0];
    uint16_t* p16 = &sixteens[0];
    uint64_t* p64 = &sixtyfours[0];
    auto* p40 = &forties[0];
    // When i is a decreasing counter, this loop produces a 32-bit phi node.
    // But the other phis are pointers; when strengthening the pointer phis,
    // the compiler requires a 64-bit induction variable, and this causes
    // strife if not handled correctly (i.e. the middle commits of
    // swarm/Swarm-IR#336).
    for (uint32_t i = static_cast<uint32_t>(size); i > 0; i--) {
        uint8_t v = *(pr++);
        *(p16++) += v;
        *(p64++) += v;
        (p40++)->vals[0] += v;
    }
}


int main(int argc, const char** argv) {
    return loop_multistride_main(argc, argv);
}
