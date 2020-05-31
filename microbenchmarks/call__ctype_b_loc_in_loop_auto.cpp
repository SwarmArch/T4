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
 * count_between_spaces contains some loops with complex bookkeeping that
 * internally call __ctype_b_loc(), which apparently returns a pointer to
 * thread-local storage.
 * http://refspecs.linuxbase.org/LSB_3.0.0/LSB-PDA/LSB-PDA/baselib---ctype-b-loc.html
 * As of its inception, this test forced the compiler to pass a pointer to a
 * thread-local array from a parent task to its (chain loop) child task. The
 * child task runs on a different thread than the parent, dereferences the
 * pointer, yielding a programming error that crashes the simulator.
 *
 * This test reproduces https://github.mit.edu/swarm/Swarm-IR/issues/408
 *
 * Note that __ctype_b_loc is only called from a C file, not a C++ file, so go
 * see this test's C module.
 */

#include "common.h"

#include <scc/autoparallel.h>

#include <cassert>
#include <ctype.h>
#include <iostream>


extern "C" {

int count_between_spaces(const char*);

}


__attribute__((noinline, assertswarmified))
static uint64_t run(uint64_t n) {
    char* str = new char[n];
    std::fill(str, str + n - 1, 'a');
    str[n - 1] = '\0';
    str[n / 4] = ' ';
    str[n / 2] = ' ';
    return count_between_spaces(str);
}


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <size>" << std::endl;
        return -1;
    }

    uint64_t n = atoi(argv[1]);
    assert(n > 1);

    uint64_t index = -1;
    SCC_PARALLEL({
        index = run(n);
    });

    tests::assert_eq((n / 2) - (n / 4), index);
    return 0;
}
