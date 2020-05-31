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

#include "pass_struct_byval_and_update_it.h"

__attribute__((assertswarmified))
uint32_t scale_and_sum(BunchOfVals bov, uint32_t multiplier) {
    // Mutate the local copy of bov; this test exists to ensure the caller's
    // copy is left untouched.
    for (uint64_t i = 0; i < NC; i++) bov.vals[i] *= multiplier;
    uint32_t sum = 0;
    for (uint64_t i = 0; i < NC; i++) sum += bov.vals[i];
    return sum;
}
