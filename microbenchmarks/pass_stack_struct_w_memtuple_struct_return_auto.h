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

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CStruct {
    bool valid;
    uint64_t v;
};

typedef struct CStruct Component;

struct SStruct {
    uint32_t pad1;
    uint64_t size;
    Component* components;
    double* pad2;
    int* pad3;
};

typedef struct SStruct S;

S getPartialSum(uint32_t dummy1, double dummy2, S in);
uint64_t iteration(uint64_t i, S* s);


#ifdef __cplusplus
}
#endif
