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

// This test of setjmp is mostly copied from
// http://en.cppreference.com/w/c/program/setjmp

#include <setjmp.h>
#include <stdint.h>

#include "common.h"
#include <scc/autoparallel.h>

#define N (42UL)

jmp_buf jump_buffer;
uint64_t accum;

__attribute__((noinline))
__attribute__((assertswarmified))
void bar(uint64_t count) {
    if (count == 0) return;
    accum += count;
    longjmp(jump_buffer, count+1); // will return count+1 out of setjmp
    accum += 7; // will never execute
}

__attribute__((noinline))
//N.B. this cannot be parallelized, as inserting a task boundary between setjmp
// and longjmp will cause longjmp to fail!
//__attribute__((assertswarmified))
void foo(void)
{
    volatile uint64_t count = 0; // modified local vars in setjmp scope must be volatile
    if (setjmp(jump_buffer) != N) // compare against constant in an if
        bar(++count);
}

int main(int argc, const char** argv) {
    accum = 0;

    SCC_PARALLEL( foo() );

    tests::assert_eq(N * (N - 1) / 2, accum);
    return 0;
}
