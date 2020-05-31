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

#ifndef __MICROBENCH_SCC_CALL_SRET_AFTER_OTHER_CALLS_IN_LOOP_H__
#define __MICROBENCH_SCC_CALL_SRET_AFTER_OTHER_CALLS_IN_LOOP_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SeveralInts {
    uint64_t one;
    uint64_t two;
    uint32_t three;
    uint16_t four;
    uint64_t five;
};

typedef struct SeveralInts SeveralInts_t;

SeveralInts_t all_ones_sret();
uint64_t single_one();


#ifdef __cplusplus
}
#endif


#endif // __MICROBENCH_SCC_CALL_SRET_AFTER_OTHER_CALLS_IN_LOOP_H__
