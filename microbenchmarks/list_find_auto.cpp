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
 * A read-only iteration over a linked list
 * (i.e. has a serial dependence between loop iterations)
 */
#include "common.h"

#include <scc/autoparallel.h>

#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>

using List = std::list<uint64_t>;
static List::const_iterator found;

__attribute__((noinline))
__attribute__((assertswarmified))
void finder(const List& l, uint64_t value) {
    found = std::find(l.begin(), l.end(), value);
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <list size>" << std::endl;
        return -1;
    }

    List l;
    uint64_t size = atoi(argv[1]);
    assert(size);
    uint64_t i = size / 2;
    while (i) {
        i--;
        l.push_back(i);
        l.push_front(i);
    }
    if (size % 2) l.push_back(size / 2);

    SCC_PARALLEL( finder(l, size / 4) );

    tests::assert_true(found == std::next(l.cbegin(), l.size() / 4));
    return 0;
}
