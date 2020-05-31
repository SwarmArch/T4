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

/**
 * At swarm/Swarm-IR@0ef2b22c297d43b15 and earlier, this version caused a
 * compiler crash, reproducing swarm/Swarm-IR#223.
 * As of swarm/Swarm-IR@758f196477a8a5f4, it compiles, but the loop is not
 * parallelized
 * As of swarm/Swarm-IR#295, this is split into a serial chain of tasks.
 * It is not properly parallel-expanded or coarsened.
 */
#include <cstdint>

using intT = int8_t;
// FIXME(mcj) we should find a way to make the int8_t* version parallelizable
#include "loop_pointer_iv.h"

int main(int argc, const char** argv) {
    return loop_pointer_iv(argc, argv);
}
