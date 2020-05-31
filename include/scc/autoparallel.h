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

/* Macros to allow an application to be autoparallelized or run serially.
 * This is intended to be used with SCC-compiled programs.
 * Please avoid dependences with Swarm code in non-Swarm runtimes---may not be
 * linked against runtime or simalloc.
 */

#pragma once


/* SCC annotation to prevent auto-parallelization */
#if defined(SCC_RUNTIME) || defined(SCC_SERIAL_RUNTIME)
#define __serial __attribute__((noswarmify))
#else
#define __serial
#endif


/* Parallel region begin/end macros. */

#if defined(SCC_RUNTIME) || defined(SCC_SERIAL_RUNTIME)
#include "swarm/api.h"
#include <functional>

// Programmers should never directly call anything in this namespace.
// Only use the SCC_PARALLEL_* macros.
namespace scc {

// There are two implementations here:
//  - one that heavily uses templates and requires passing an aggregate by
//    value to the initial task function.
//  - one that avoids passing arguments to the initial task function and
//    instead stores things in a global variable, requiring indirect/virtual
//    function calling.
//  If you run into trouble with one, try the other instead.
#define __AUTOPARALLEL_TEMPLATED_INIT_TASKFUNC 0
#if __AUTOPARALLEL_TEMPLATED_INIT_TASKFUNC

// This version of callROILambdaFunc is derived from swarm::enqueueLambda.
// Note that it passes the lambda directly by value, and the lambda may be
// an aggregate containing captured values.  This may require LLVM's
// -argpromotion pass (which runs only with -O3, not -O2) to break up the
// aggregate to avoid ABI issues to do with LLVM's byval parameter attribute.

template<typename L>
__attribute__((noinline, assertswarmified))
void callROILambdaFunc(swarm::Timestamp, L lfunc) {
    lfunc();
}

template<typename L>
static inline void __enqueueROILambda(L lfunc) {
    swarm::enqueue(callROILambdaFunc<L>, 0, EnqFlags::NOHINT, lfunc);
}

#else  // __AUTOPARALLEL_TEMPLATED_INIT_TASKFUNC

// This version of callROILambdaFunc uses less templates and requires no
// arguments.  Instead, it uses std::function (whose implimentation uses
// virtual function calls and possibly more indirection).

static std::function<void()> __ROILambdaFunc;

// Since it's not a template, we could disable name mangling (`extern "C"`) on
// this version of callROILambdaFunc if we wanted a way for SCC or other tools
// to portably recognize this special initial-task function.
__attribute__((noinline, assertswarmified))
void callROILambdaFunc(swarm::Timestamp) {
    __ROILambdaFunc();
}

template<typename L>
static inline void __enqueueROILambda(L lfunc) {
    __ROILambdaFunc = lfunc;
    swarm::enqueue(callROILambdaFunc, 0, EnqFlags::NOHINT);
}

#endif  // __AUTOPARALLEL_TEMPLATED_INIT_TASKFUNC

} // end namespace scc

// dsm: In this particular case pass-by-reference is OK, because objects live
// in the caller's stack, which is not a Swarm thread stack and is left alone.
// This avoids copying large objects and errors due to discarding qualifiers.
#define SCC_PARALLEL_BEGIN() \
        { \
            scc::__enqueueROILambda(\
                    [&]() __attribute__((assertswarmified)) -> void {

#define SCC_PARALLEL_END() \
                    }); \
            swarm::__record_main_fsgs_addresses(); \
            swarm::run(); \
        }

#else  // non-SCC runtime

#include "swarm/hooks.h"

#define SCC_PARALLEL_BEGIN() zsim_roi_begin();

#define SCC_PARALLEL_END() zsim_roi_end();

#endif

// [victory] Directly using the BEGIN and END macros is error-prone.
//           Here's some syntatic sugar that should help:
#define SCC_PARALLEL(X) \
    do { SCC_PARALLEL_BEGIN(); { X; } SCC_PARALLEL_END(); } while (false)
