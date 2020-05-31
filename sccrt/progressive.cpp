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

// Implementation of spawners for progressive expansion

#include <scc/rt.h>

#include <cassert>
#include <cstdio>
#include <stdint.h>

#ifndef SCCRT_SERIAL_IMPL

#ifndef SWARM_RUNTIME
#error "Low-level SCC runtime functions expect manual SWARM_RUNTIME tasks"
#endif
#include "swarm/api.h"
#include "swarm/aligned.h"

#else

namespace swarm { using Timestamp = uint64_t; }

#endif

#undef DEBUG
#define DEBUG(...) //std::printf("[sccrt_progressive]" __VA_ARGS__)


namespace {

static constexpr const uint32_t EnqueuesPerTask = 4;

// Only use even timestamps, so the compiler knows it can spawn a latch task
// with an odd timestamp to fall between two iterations.
static constexpr const uint32_t TimestampsPerIter = 2;

#ifndef SCCRT_SERIAL_IMPL
// Loads of this global must never suffer false sharing conflicts
static const swarm::aligned<uint32_t> MaxStride =
        4 * swarm::num_threads() * EnqueuesPerTask * TimestampsPerIter;
#else
static const uint32_t MaxStride = EnqueuesPerTask * TimestampsPerIter;
#endif

template<typename TimestampType>
class ProgressiveEnqueue {
    typedef void (*iterTaskTy)(TimestampType ts, uint32_t* done, void* closure);
    const iterTaskTy iterTask;
    uint32_t* const doneFlag;
    void* const closure;

  public:
    ProgressiveEnqueue(iterTaskTy it, uint32_t* done, void* c)
        : iterTask(it), doneFlag(done), closure(c) {}


    inline void run() const {
        DEBUG("Starting progressive expansion with done flag %p, closure %p, "
              "iterTask %p\n",
              doneFlag, closure, iterTask);

        assert(!*doneFlag);

        DEBUG("    Enqueuing first enqueuer\n");
#ifndef SCCRT_SERIAL_IMPL
        swarm::enqueueLambda(*this,
                           0,
                           EnqFlags(NOHINT | PRODUCER),
                           EnqueuesPerTask * TimestampsPerIter);
#else
        enqueueThis(0, EnqueuesPerTask * TimestampsPerIter);
#endif
    }

    // It would be nice for this to be private, but the callLambdaFunc stuff
    // in the implementation of enqueueLambda needs access to this.
    inline void operator()(swarm::Timestamp ts, uint32_t stride) const {
        DEBUG("Progressive enqueuer w/ done flag %p, closure %p, iterTask %p: "
              "ts %lu, stride %u\n",
              doneFlag, closure, iterTask, ts, stride);

        if (*doneFlag) {
          DEBUG("    Progressive expansion done!\n");
          return;
        }

        DEBUG("    Enqueuing children...\n");

        swarm::Timestamp end = ts + EnqueuesPerTask * TimestampsPerIter;
#ifndef SCCRT_SERIAL_IMPL
        // Enqueue the first child with SAMEHINT so it runs as early as possible
        enqueueIterTask<EnqFlags::SAMEHINT>(ts);
        for (swarm::Timestamp i = ts + TimestampsPerIter; i < end; i += TimestampsPerIter) {
             enqueueIterTask<EnqFlags::NOHINT>(i);
        }
#else
        for (swarm::Timestamp i = ts; i < end; i += TimestampsPerIter) {
             enqueueIterTask(i);
        }
#endif

        if (stride < MaxStride) {
            reenqueue(ts + stride, 2 * stride);
            reenqueue(ts + 2 * stride, 2 * stride);
        } else {
            reenqueue(ts + stride, stride);
        }
    }

  private:

#ifndef SCCRT_SERIAL_IMPL
    template<EnqFlags Flags>
#endif
    inline void enqueueIterTask(swarm::Timestamp ts) const {
        DEBUG("        %lu\n", ts);
#ifndef SCCRT_SERIAL_IMPL
        // Perform a bare enqueue to avoid unnecessary indirect branches
        // and jumps/calls from a bareRunner.
        swarm::__enqueue_task_skipargs(
                swarm::enqueueMagicOp(2, Flags),
                reinterpret_cast<uint64_t>(iterTask),
                /* ts = */ ts,
                /* hint = */ 0xcafed00d,
                reinterpret_cast<uint64_t>(doneFlag),
                reinterpret_cast<uint64_t>(closure));
#else
        __sccrt_serial_enqueue(
                reinterpret_cast<void*>(iterTask),
                /* ts = */ ts,
                reinterpret_cast<uint64_t>(doneFlag),
                reinterpret_cast<uint64_t>(closure),
                0,0,0);
#endif
    }

    inline void reenqueue(swarm::Timestamp ts, uint32_t stride) const {
        DEBUG("    Enqueuing enqueuer %lu, stride %u\n", ts, stride);
#ifndef SCCRT_SERIAL_IMPL
        swarm::enqueueLambda(*this,
                           ts,
                           EnqFlags(NOHINT | PRODUCER | SAMETASK),
                           stride);
#else
        enqueueThis(ts, stride);
#endif
    }

#ifdef SCCRT_SERIAL_IMPL
    static void enqueuer(swarm::Timestamp ts,
                         uint32_t stride,
                         iterTaskTy it, uint32_t* done, void* c) {
        ProgressiveEnqueue<TimestampType>(it, done, c)(ts, stride);
    }
    inline void enqueueThis(swarm::Timestamp ts, uint32_t stride) const {
        __sccrt_serial_enqueue(reinterpret_cast<void*>(enqueuer), ts, stride,
                               reinterpret_cast<uint64_t>(iterTask),
                               reinterpret_cast<uint64_t>(doneFlag),
                               reinterpret_cast<uint64_t>(closure),
                               0);
    }
#endif
};

}  // anonymous namespace


extern "C" {

void __sccrt_enqueue_progressive_64(void (*iterTask)(uint64_t, uint32_t*, void*),
                                    uint32_t* done,
                                    void* closure) {
    ProgressiveEnqueue<uint64_t>(iterTask, done, closure).run();
}


void __sccrt_enqueue_progressive_32(void (*iterTask)(uint32_t, uint32_t*, void*),
                                    uint32_t* done,
                                    void* closure) {
    ProgressiveEnqueue<uint32_t>(iterTask, done, closure).run();
}

}  // extern "C"
