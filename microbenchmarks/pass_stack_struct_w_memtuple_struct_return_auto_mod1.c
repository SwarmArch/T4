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

#include "pass_stack_struct_w_memtuple_struct_return_auto.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

__attribute__((noinline))
__attribute__((assertswarmified))
S getPartialSum(uint32_t dummy1, double dummy2, S in) {
    // We expect powers of two
    assert(in.size && !(in.size & (in.size - 1)));

    S out;
    out.size = in.size / 2;
    out.components = (Component*)calloc(out.size, sizeof(Component));

    for (uint64_t i = 0; i < in.size; i += 2) {
        out.components[i / 2].v =
                (in.components[i].valid * in.components[i].v) +
                (in.components[i + 1].valid * in.components[i + 1].v);
        out.components[i / 2].valid =
                in.components[i].valid ||
                in.components[i + 1].valid;
    }
    return out;
}
