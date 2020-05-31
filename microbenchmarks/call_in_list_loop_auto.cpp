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
#include <cassert>
#include <cstdint>
#include <list>
#include <vector>

__attribute__((noinline, assertswarmified))
static void increment(uint64_t* c) {
    (*c)++;
}

__attribute__((noinline, assertswarmified))
static void foo(const std::list<uint64_t*>& pointers) {
    // This tests two features together:
    // 1) The loop should be late chained by LoopExpansion
    // 2) The continuation after the increment call is empty, apart from some
    //    llvm.dbg.value instructions. These llvm.dbg.value instructions prevent
    //    CFG simplification from removing the dead continuation task.
    for (auto it = pointers.begin(), e = pointers.end();
         it != e && std::next(it) != e;
         it++) {
        uint64_t* p = *it;
        increment(p);
    }
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }

    const uint64_t n = atoi(argv[1]);
    std::vector<uint64_t> counters(n, 0);
    std::list<uint64_t*> pointers;
    for (uint64_t& c : counters) {
        pointers.push_front(&c);
    }
    assert(pointers.size() == n);

    SCC_PARALLEL( foo(pointers) );

    std::vector<uint64_t> expected(n, 1);
    expected[0] = 0;
    tests::assert_eq(expected, counters);
    return 0;
}
