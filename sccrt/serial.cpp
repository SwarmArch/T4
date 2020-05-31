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

// Serial software queuing of tasks

#include "scc/serial.h"
#include <scc/rt.h>

#include "swarm/impl/callfunc.h"

// Always enable assertions in this file.
#undef NDEBUG

#include <array>
#include <cassert>
#include <cstdio>
#include <queue>
#include <random>
#include <stdint.h>
#include <tuple>
#include <vector>

#undef DEBUG
#define DEBUG(...) //std::printf("[sccrt_serial] " __VA_ARGS__)

namespace {

using Timestamp = uint64_t;

static std::uniform_int_distribution<uint64_t> randomTiebreaker(0, UINT64_MAX);
static std::mt19937_64 gen(0);

template<typename... Args>
struct TaskImpl {
    typedef void (*TaskFn)(Timestamp, Args...);
    const TaskFn taskfn;

    static_assert(sizeof...(Args) == __SCCRT_SERIAL_MAX_ARGS, "");
    //const std::tuple<Timestamp, Args...> args;
    const std::array<uint64_t, __SCCRT_SERIAL_MAX_ARGS+1> args;

    const uint64_t tiebreaker;

    TaskImpl(void* taskfn, Timestamp ts, Args... args)
        : taskfn(reinterpret_cast<TaskFn>(taskfn)),
          args{{ts, args...}},
          tiebreaker(randomTiebreaker(gen)) {}

    Timestamp ts() const {
        return args[0];
    }

    void run() {
        DEBUG("Running task %p: ts %lu func %p\n", this, ts(), taskfn);

        // std::apply is a C++17 library feature equivalent to swarm::callFuncTuple
        //std::apply(taskfn, args);
        swarm::callFuncTuple(taskfn, args);
    }
};

using Task = TaskImpl<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t>;

struct CompareTasks {
    bool operator()(const Task* a, const Task* b) {
        return std::make_tuple(a->ts(), a->tiebreaker) >
               std::make_tuple(b->ts(), b->tiebreaker);
    }
};
using PrioQueue = std::priority_queue<Task*, std::vector<Task*>, CompareTasks>;

}  // anonymous namespace

static std::vector<PrioQueue> pqs(1);
static std::vector<Timestamp> superTsStack = {-1ULL};
static uint32_t currDomainDepth = 0;
static Task* currTask = nullptr;
static bool currTaskUndeepened = false;

template <bool ToSuper>
static void enqueue(Task* task) {
    unsigned upLevels = ToSuper + currTaskUndeepened;
    assert(pqs.size() > upLevels);
    unsigned level = pqs.size() - 1 - upLevels;
    DEBUG("Enqueuing task %p: fn %p, ts %lu, domain depth %u (%+d from parent)\n",
          task, task->taskfn, task->ts(), level, level - currDomainDepth);
    assert(level != currDomainDepth ||
           task->ts() >= (currTask ? currTask->ts() : 0));
    assert(level >= currDomainDepth || task->ts() >= superTsStack[level+1]);
    pqs[level].push(task);
}

template <bool Super>
static Timestamp getTimestamp() {
    unsigned upLevels = Super + currTaskUndeepened;
    assert(upLevels <= superTsStack.size());
    unsigned level = superTsStack.size() - 1 - upLevels;
    Timestamp ts;
    if (level == currDomainDepth) {
        ts = currTask->ts();
    } else if (level > currDomainDepth) {
        // We must have deepened, this is asking about the subdomain.
        assert(upLevels == 0);
        assert(level == currDomainDepth+1);
        ts = 0;
    } else { // level < currDomainDepth
        // This is actually asking for a superdomain timestamp;
        assert(upLevels);
        assert(level == currDomainDepth-1);
        ts = superTsStack[level+1];
        if (level == -1U) {
            // We're in the root domain.
            assert(currDomainDepth == 0);
            assert(ts == -1ULL);
        }
    }
    DEBUG("Got timestamp %lu at depth %d\n", ts, (signed)level);
    return ts;
}

void sccrt::runSerial() {
    std::printf("[sccrt_serial] Beginning serial task execution.\n");
    std::fflush(stdout);
    uint64_t numTasks = 0;
    while(!pqs.empty()) {
        PrioQueue* currQueue;
        while (!(currQueue = &pqs.back())->empty()) {
            currDomainDepth = pqs.size() - 1;
            DEBUG("Running at domain depth %u\n", currDomainDepth);

            currTaskUndeepened = false;
            currTask = currQueue->top();
            currQueue->pop();

            currTask->run();
            delete currTask;
            currTask = nullptr;
            currTaskUndeepened = false;

            numTasks++;
        }
        DEBUG("Finished domain at depth %lu\n", pqs.size() - 1);
        assert(pqs.size() == superTsStack.size());
        pqs.pop_back();
        superTsStack.pop_back();
    }
    std::printf("[sccrt_serial] Finished serial execution of %lu tasks.\n",
                numTasks);
    std::fflush(stdout);
}

extern "C" {

void __sccrt_serial_enqueue(void* taskfn,
                            Timestamp ts,
                            uint64_t arg0,
                            uint64_t arg1,
                            uint64_t arg2,
                            uint64_t arg3,
                            uint64_t arg4) {
    enqueue<false>(new Task(taskfn, ts, arg0, arg1, arg2, arg3, arg4));
}
void __sccrt_serial_enqueue_super(void* taskfn,
                                  Timestamp ts,
                                  uint64_t arg0,
                                  uint64_t arg1,
                                  uint64_t arg2,
                                  uint64_t arg3,
                                  uint64_t arg4) {
    enqueue<true>(new Task(taskfn, ts, arg0, arg1, arg2, arg3, arg4));
}
void __sccrt_serial_deepen() {
    assert(!currTaskUndeepened);
    DEBUG("Deepen: creating new domain at depth %lu\n", pqs.size());
    pqs.emplace_back();
    superTsStack.push_back(currTask->ts());
}
void __sccrt_serial_undeepen() {
    DEBUG("Undeepen: returning to depth %u\n", currDomainDepth);
    assert(!currTaskUndeepened);
    assert(superTsStack.back() == currTask->ts());
    currTaskUndeepened = true;
}
void __sccrt_serial_heartbeat() {
    //TODO(victory): Implement heartbeats.
}
Timestamp __sccrt_serial_get_timestamp() {
    return getTimestamp<false>();
}
Timestamp __sccrt_serial_get_timestamp_super() {
    return getTimestamp<true>();
}

}  // extern "C"
