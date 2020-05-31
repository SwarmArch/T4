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

#include <string.h>

// FIXME(mcj) Not parallelized as of swarm/Swarm-IR@ca8ee97579771e6
// __attribute__((assertswarmified))
uint64_t reduce64(uint64_t* array, uint32_t size) {
    if (size == 0) return 0ul;
    if (size == 1) return array[0];

    uint32_t halfsize = (size + 1) / 2;
    uint64_t intermediates[halfsize];
    for (uint32_t i = 0; i < size - 1; i += 2) {
        intermediates[i / 2] = array[i] + array[i + 1];
    }
    if (size % 2) intermediates[halfsize - 1] = array[size - 1];
    return reduce64(intermediates, halfsize);
}


// FIXME(mcj) Not parallelized as of swarm/Swarm-IR@ca8ee97579771e6
// __attribute__((assertswarmified))
uint64_t foo(uint32_t size) {
    if (size % 2) {
        uint8_t odd[size + 1];
        memset(odd, 1, (size + 1) * sizeof(uint8_t));
        return reduce8(odd, size + 1);
    } else {
        uint8_t even[size];
        memset(even, 2, size * sizeof(uint8_t));
        return reduce8(even, size);
    }
}
