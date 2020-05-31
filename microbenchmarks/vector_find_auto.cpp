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
 * A read-only iteration over a vector.
 * (it doesn't parallelize as of swarm/Swarm-IR@0cff7254a6f)
 */
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "common.h"
#include <scc/autoparallel.h>

using Vector = std::vector<uint64_t>;
static Vector::const_iterator found;

__attribute__((noinline))
__attribute__((assertswarmified))
void finder(const Vector& v, uint64_t value) {
    // TODO(mcj) for extra fun try std::find, but it's manually unrolled
    auto begin = v.begin();
    auto end = v.end();
    for (auto it = begin; it != end; it++) {
        if (*it == value) {
            found = it;
            break;
        }
    }
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <container size>" << std::endl;
        return -1;
    }
    Vector v(atoi(argv[1]));
    assert(!v.empty());
    std::iota(v.begin(), v.end(), 0ul);
    found = v.cend();

    SCC_PARALLEL( finder(v, v.size() / 2) );

    tests::assert_true(found == std::next(v.cbegin(), v.size() / 2));
    return 0;
}
