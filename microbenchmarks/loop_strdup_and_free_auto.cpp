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
 * The loop in copy_dictionary calls free and strdup.
 * Prior to swarm/benchmarks#155, it reproduced swarm/Swarm-IR#451: i.e. a call
 * of free() received a bad pointer, even in a non-speculative task.
 *
 * The problem was only reproduceable when copy_dictionary(...) was moved to a C
 * module, and critically the problem goes away when -std=c11 is removed as a
 * compile flag. Interestingly, the call of strdup is indirect. Perhaps because
 * the compiler warns:
 *
 * microbenchmarks/scc/loop_strdup_and_free_auto_mod1.c:15:21: warning: implicit declaration of function 'strdup' is invalid in C99 [-Wimplicit-function-declaration]
 *        copies[i] = strdup(dictionary[i]);
 *                    ^
 * microbenchmarks/scc/loop_strdup_and_free_auto_mod1.c:15:19: warning: incompatible integer to pointer conversion assigning to 'char *' from 'int' [-Wint-conversion]
 *        copies[i] = strdup(dictionary[i]);
 *                  ^ ~~~~~~~~~~~~~~~~~~~~~
 *
 * Note that these warnings also go away when -std=c11 is removed and replaced
 * with -std=gnu11.
 */

#include "loop_strdup_and_free.h"

#include <scc/autoparallel.h>
#include "common.h"

#include <algorithm>
#include <cassert>
#include <unordered_set>
#include <vector>

static std::vector<const char*> dictionary;
static std::vector<char*> copies;

__attribute__((assertswarmified))
void free_task(void* ptr) {
    if (ptr) { free(ptr); }
}


int main(int argc, const char** argv) {
    const char* word = "lololol";
    uint64_t n = atoi(argv[1]);
    dictionary.resize(n, word);
    copies.resize(n, nullptr);

    // All words in the dictionary are initially the same pointer
    assert(std::all_of(dictionary.begin(), dictionary.end(),
                       [word] (const char* s) { return s == word; }));

    SCC_PARALLEL({
        copy_dictionary(dictionary.data(), copies.data(), n);
        copy_dictionary(dictionary.data(), copies.data(), n);
        copy_dictionary(dictionary.data(), copies.data(), n);
    });

    // Due to the strdup calls, each pointer in copies should be unique
    std::unordered_set<char*> unique_ptrs(copies.begin(), copies.end());
    tests::assert_eq(copies.size(), unique_ptrs.size());

    return 0;
}
