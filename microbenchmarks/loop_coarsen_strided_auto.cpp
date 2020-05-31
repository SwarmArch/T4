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

#include <scc/autoparallel.h>
#include "common.h"

#include <algorithm>

template <int64_t stride>
__attribute__((noinline))
__attribute__((assertswarmified))
void test(uint64_t* buf, size_t elems) {
  for (int64_t i = 0; i < elems; i++)
    buf[i*stride] = i;
}

int main(int argc, const char* argv[]) {
  size_t elems = atoi(argv[1]);
  size_t offset = atoi(argv[3]);

  // Use several buffers and strides to test prolog and epilog generation
  uint64_t* bufs[8];
  int64_t strides[] = {1, 3, 4, 5, -1, -3, -4, -5};
  int b = 0;
  for (auto stride: strides) {
    size_t absStride = (stride > 0)? stride : -stride;
    size_t bufBytes = elems * absStride * sizeof(uint64_t) + offset;
    char* rawBuf = (char*) calloc(sizeof(char), bufBytes);
    // buf isn't even uint64_t aligned (should be OK)
    bufs[b++] = (uint64_t*) (rawBuf  + ((stride > 0)? offset : bufBytes));
  }

  SCC_PARALLEL({
    test<1>(bufs[0], elems);
    test<3>(bufs[1], elems);
    test<4>(bufs[2], elems);
    test<5>(bufs[3], elems);
    test<-1>(bufs[4], elems);
    test<-3>(bufs[5], elems);
    test<-4>(bufs[6], elems);
    test<-5>(bufs[7], elems);
  });

  size_t i;
  for (b = 0; b < 8; b++)
    for (i = 0; i < elems; i++)
      if (bufs[b][i * strides[b]] != (uint64_t)i) break;
  // If b and i do not reach the end of the loop,
  // the following will indicate the problematic entries
  tests::assert_eq(8, b);
  tests::assert_eq(elems, i);
}
