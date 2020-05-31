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

#include "common.h"

#include <scc/autoparallel.h>
#include <swarm/swarm.h>

#include <array>
#include <complex>

#define NC (99ul)

std::array<uint64_t, NC> counters;

__attribute__((noinline))
__attribute__((assertswarmified))
int do_nothing() {
  static int x = 0;
  return x++;
}

__attribute__((noinline))
__attribute__((assertswarmified))
std::complex<double> cmplx( double x, double y )  {
    do_nothing(); // This call "accesses memory" and prevents optimization
    std::complex<double> c;
    c.real(x); c.imag(y);
    return c;
}

__attribute__((noinline))
__attribute__((assertswarmified))
void foo(uint64_t c) {
    counters[c] += std::abs(cmplx(1.0, 0.0));
}

int main(int argc, const char** argv) {
    SCC_PARALLEL({
        for (uint64_t c = 0; c < NC; c++) {
            foo(c);
        }
    });

    std::array<uint64_t, NC> verify;
    verify.fill(1ul);
    tests::assert_eq(verify, counters);
    return 0;
}
