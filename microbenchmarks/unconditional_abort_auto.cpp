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

#include <iostream>

#include <scc/autoparallel.h>
#include "common.h"

// Volatile so that the conditional isn't constant propagated away
static volatile bool crash;

// Pass a local stack variable to trigger (and fail) detection of the function's
// return block at compiler commit 4f8260fdc12ce and earlier.
__attribute__((noinline))
//Not parallelizable due to exception-handling terminator
//__attribute__((assertswarmified))
static void error(const std::string& str) {
    std::cerr << str << std::endl;
    std::abort();
}

__attribute__((noinline))
//Not parallelizable due to exception-handling terminator
//__attribute__((assertswarmified))
static void foo() {
    std::string str = "Error";
    if (crash) error(str);
}

int main() {
    crash = false;

    SCC_PARALLEL( foo() );

    tests::assert_true(true);
    return 0;
}
