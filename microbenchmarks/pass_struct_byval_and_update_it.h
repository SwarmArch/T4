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

#ifndef __MICROBENCHMARKS_SCC_PASS_STRUCT_BYVAL_AND_UPDATE_IT_H__
#define __MICROBENCHMARKS_SCC_PASS_STRUCT_BYVAL_AND_UPDATE_IT_H__

#include <stdint.h>

#define NC (416u)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t vals[NC];
} BunchOfVals;

// Pass bov by value (i.e. trigger the byval attribute)
uint32_t scale_and_sum(BunchOfVals bov, uint32_t multiplier);

#ifdef __cplusplus
}
#endif

#endif // __MICROBENCHMARKS_SCC_PASS_STRUCT_BYVAL_AND_UPDATE_IT_H__
