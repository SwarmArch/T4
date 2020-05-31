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

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define NSP (2)

__attribute__((assertswarmified))
int count_between_spaces(const char* chars) {
    // In a loop with complex bookkeeping (i.e. eagerly turned into a chain),
    // call __ctype_b_loc in each iteration. This is achieved under the hood via
    // isspace(). Since __ctype_b_loc uses thread-local storage, the goal is to
    // make the parent task grab a thread-local pointer, then pass that pointer
    // to its child (chain) iteration task which dereferences the pointer. This
    // requires careful manipulation to put the __ctype_b_loc in the loop
    // preheader, or very close to it. I wandered my way into this test
    // successfully reproducing the crash in swarm/Swarm-IR#408. Unfortunately
    // it does not reproduce the case that constant PHI nodes take on the value
    // returned by __ctype_b_loc.

    // An array that tracks all the spaces that have been found in chars.
    // It seems to get this test to reproduce the error we need an AllocaInst.
    const char* space_ptrs[NSP] = {NULL};
    // The current space tracker that we hope to fill.
    const char** current_space = space_ptrs;

    // The current character under study.
    const char* c = chars;

    // These over-reaction error handling branches seem necessary to cause the
    // thread-local table pointer to be passed to child tasks. Their
    // UnreachableInsts may affect the preheader of the loop.
    if (!c) exit(1);
    if (iscntrl(*c)) exit(1);
    while (*c != '\0' && current_space != &space_ptrs[NSP]) {
        while (*c != '\0' && !isspace(*c)) c++;
        if (isspace(*c)) {
            if (current_space && !*current_space) {
                *current_space = c;
                current_space++;
            }
            c++;
        }
    }

    return space_ptrs[1] - space_ptrs[0];
}
