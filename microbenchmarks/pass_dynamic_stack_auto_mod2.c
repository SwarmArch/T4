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

#include "pass_dynamic_stack.h"

// FIXME(mcj) Not parallelized as of swarm/Swarm-IR@ca8ee97579771e6
// __attribute__((assertswarmified))
uint64_t reduce8(uint8_t* array, uint32_t size) {
    if (size == 0) return 0ul;
    uint64_t copies[size];
    for (uint32_t i = 0; i < size; i++) {
        copies[i] = array[i];
    }
    return reduce64(copies, size);
}
