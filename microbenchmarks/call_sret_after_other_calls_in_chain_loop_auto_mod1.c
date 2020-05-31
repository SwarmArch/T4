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

// TODO(mcj) de-duplicate this whole file from
// call_sret_after_other_calls_in_loop_auto_mod1.c

#include "call_sret_after_other_calls_in_loop.h"
#include "swarm/rand.h"

#include <string.h>

#define STACK_ENTRIES (10000)

volatile SeveralInts_t global_si = {1, 1, 1, 1, 1};

// Can't assert swarmified due to the random() call in swarm_rand64()
SeveralInts_t all_ones_sret() {
    if (swarm_rand64() % 2) return global_si;

    char bigstack[STACK_ENTRIES];
    memset(bigstack, 'a', STACK_ENTRIES - 1);
    bigstack[1] = '\0';
    uint8_t len = strlen(bigstack);
    SeveralInts_t si = {len, len, len, len, len};
    return si;
}

__attribute__((assertswarmified))
uint64_t single_one() {
    return 1;
}
