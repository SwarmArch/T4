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
    for (uint64_t i = 0; i < size; i++) {
        const uint8_t v = randoms[i];
        sixteens[i] += v;
        sixtyfours[i] += v;
        forties[i].vals[0] += v;
    }
}


int main(int argc, const char** argv) {
    return loop_multistride_main(argc, argv);
}
