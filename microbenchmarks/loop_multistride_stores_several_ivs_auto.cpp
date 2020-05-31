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
    const uint8_t* const begin = &randoms[0];
    const uint8_t* const end = begin + size;
    uint16_t* p16 = &sixteens[0];
    uint64_t* p64 = &sixtyfours[0];
    FiveBytes* p40 = &forties[0];
    for (const uint8_t* v = begin; v != end; v++) {
        *p16 += *v;
        *p64 += *v;
        p40->vals[0] += *v;
        p16++;
        p64++;
        p40++;
    }
}


int main(int argc, const char** argv) {
    return loop_multistride_main(argc, argv);
}
