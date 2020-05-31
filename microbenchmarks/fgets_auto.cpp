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
 * Open and read the first line of run_all.py
 */
#include "fgets_auto.h"

#include "common.h"

#include <scc/autoparallel.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

constexpr uint64_t NC = 256;

static const char* SHEBANG = "#!/usr/bin/python";
static bool correct;


__attribute__((noinline, assertswarmified))
void decode(char* buffer, int count) {
    // Decode the secret cypher
    size_t len = strlen(buffer);
    std::reverse(buffer, buffer + len);
}


__attribute__((noinline, assertswarmified))
void foo(const char* fname, int count) {
    char buffer[NC];
    std::fill(buffer, buffer + count, 0);
    read_and_encode(fname, buffer, count);
    decode(buffer, count);
    correct = (strncmp(buffer, SHEBANG, count) == 0);
}


int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input path>" << std::endl;
        return -1;
    }
    const char* fname = argv[1];
    uint64_t shebang_length = strlen(SHEBANG);
    assert(shebang_length < NC);
    correct = false;

    SCC_PARALLEL( foo(fname, NC) );

    tests::assert_true(correct);
    return 0;
}
