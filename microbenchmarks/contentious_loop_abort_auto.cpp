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

#include <algorithm>

static uint16_t counter[200];

static uint64_t table[160000];

__attribute__((noinline))
__attribute__((assertswarmified))
static void ack(uint64_t idx) {
  uint64_t val = 0;
  for (uint64_t j = 0; j < 200; j++) {
    // Do some other work here that is fully parallelizable
    // but forced to be serial thanks to below

    // Some complicated hash of idx for no reason
    uint64_t x = 123456789123ULL, y = 987654321987ULL;
    unsigned int z1 = 43219876, c1 = 6543217, z2 = 21987643, c2 = 1732654;  // Seed variables
    uint64_t t;

    int i = idx;
    x = x ^ (idx & 0x00000000FFFFFFFF) << j;
    y = y ^ (idx >> 32) << j;

    x = 1490024343005336237ULL * x + 123456789;

    y ^= y << 21;
    y ^= y >> 17;
    y ^= y << 30;  // Do not set y=0!

    t = 4294584393ULL * z1 + c1;
    c1 = t >> 32;
    z1 = t + j;

    t = 4246477509ULL * z2 + c2;
    c2 = t >> 32;
    z2 = t + j;

    val = x + y + z1 + ((uint64_t)z2 << 32);
    table[((idx*200)+j)*8] = val | 1;

    // Part that forces serialization
    counter[j]++;

  }

}
__attribute__((noinline))
__attribute__((assertswarmified))
static void foo() {
  for (uint64_t i = 0; i < 100; i++) {
    ack(i);
  }
}

int main(int argc, const char** argv) {
  SCC_PARALLEL( foo() );

  tests::assert_true(std::all_of(counter, counter + 100,
                                 [] (uint16_t c) {return c == 100;}));

  int j = 0;
  tests::assert_true(std::all_of(table, table + 160000,
                                 [&j] (uint64_t c) {
                                    if(j % 8 != 0) return true;
                                    j++;
                                    return c != 0;
                                 }));

  return 0;
}
