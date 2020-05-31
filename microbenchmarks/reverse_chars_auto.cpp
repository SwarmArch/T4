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
 * Reverse a string
 */

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

static std::vector<char> chars;

__attribute__((noinline, assertswarmified))
void reverse(char* buffer) {
    size_t len = strlen(buffer);
    assert(len);
    for (char *b = buffer, *e = buffer + len - 1; b < e; b++, e--) {
        char tmp = *b;
        *b = *e;
        *e = tmp;
    }
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <string length>" << std::endl;
        return -1;
    }

    uint64_t size = atoi(argv[1]) + 1;
    std::vector<char> reference(size, 0);
    assert(reference.size() > 0);
    reference[0] = 'e';
    reference[1] = 'l';
    reference[2] = 'g';
    reference[size - 2] = 'g';
    reference[size - 1] = '\0';
    std::fill(&reference[3], &reference[size - 2], 'o');

    chars.resize(reference.size());
    std::copy(reference.begin(), reference.end(), chars.begin());
    assert(strlen(chars.data()) == chars.size() - 1);

    SCC_PARALLEL( reverse(chars.data()) );

    std::reverse(reference.begin(), std::prev(reference.end()));
    bool correct = strcmp(reference.data(), chars.data()) == 0;
    if (!correct) std::cout << "reversed " << chars.data() << '\n';
    tests::assert_true(correct);
    return 0;
}
