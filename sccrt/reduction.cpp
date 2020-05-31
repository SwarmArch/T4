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

// A reduction implementation for use by the Swarm compiler.

#include <scc/rt.h>

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <stdint.h>

#ifndef SCCRT_SERIAL_IMPL

#ifndef SWARM_RUNTIME
#error "Low-level SCC runtime functions expect manual SWARM_RUNTIME tasks"
#endif

#include "swarm/api.h"
#include "swarm/algorithm.h"

#else

namespace swarm { using Timestamp = uint64_t; }

#endif

#define DEBUG(args...) //swarm::info(args)

namespace {

template <typename T>
struct Reducer {
    // TODO(mcj) We could look into per-Reducer sizes, e.g. when the loop bound
    // is say 16, we only need 16 or fewer intermediates.
    static const uint32_t size;

#ifndef SCCRT_SERIAL_IMPL
    // Thread-private intermediates, indexed using swarm::tid().
    // We'll dynamically size it; one T per cache line.
    swarm::aligned<T> intermediates[0];
#else
    T intermediates[1];
#endif
};


// initial   is the original value assigned to the accumulator (an artifact of
//           LLVM's approach to reductions) e.g. the initial loaded value
// identity  is the BinaryOp's identity value (e.g. 0 for + or 1 for *)
template <typename T>
void* init(T initial, T identity) {
    using Reducer = Reducer<T>;
    Reducer* r = (Reducer*)malloc(
#ifndef SCCRT_SERIAL_IMPL
            SWARM_CACHE_LINE * Reducer::size
#else
            sizeof(Reducer)
#endif
            );
    r->intermediates[0] = initial;
    // These stores can all be issued independently on a scoreboarded core. But
    // when the enclosing task ends, it must wait to relinquish the core until
    // all stores receive conflict detection ACKs
    std::fill(r->intermediates + 1,
              r->intermediates + Reducer::size,
              identity);
    return r;
}


#ifndef SCCRT_SERIAL_IMPL

template <typename T, typename BinaryOp>
static void update_task(swarm::Timestamp,
                        Reducer<T>* r,
                        T value,
                        BinaryOp o) {
    DEBUG("tid %ld", swarm::tid());
    auto* intermediate = &r->intermediates[swarm::tid()];
    *intermediate = o(*intermediate, value);
}


template <typename T, typename BinaryOp>
static inline void update(swarm::Timestamp ts, void* r, T value, BinaryOp o) {
    swarm::enqueue((update_task<T, BinaryOp>), ts, EnqFlags::NOHINT,
                 reinterpret_cast<Reducer<T>*>(r), value, o);
}

#else

template <typename T, typename BinaryOp>
static inline void update(swarm::Timestamp ts, void* r, T value, BinaryOp o) {
    (void)ts;
    using Reducer = Reducer<T>;
    auto* reducer = reinterpret_cast<Reducer*>(r);
    auto* intermediate = &reducer->intermediates[0];
    *intermediate = o(*intermediate, value);
}

#endif

template <typename T, typename BinaryOp>
static inline T collapse(void* r, BinaryOp o) {
    using Reducer = Reducer<T>;
    auto* reducer = reinterpret_cast<Reducer*>(r);
    auto* begin = &reducer->intermediates[0];
    auto* end = &reducer->intermediates[Reducer::size];

    // Force an 8-leaf tree summation to exploit MLP since most loads will miss.
    // The alternative (see the epilog below) compiles the loop such that all
    // loads serialize on an in-order processor, due to dependences on the sole
    // accumulator register.
    T accumulator = *begin;
    auto it = begin + 1;
    while (it < end - 8) {
        T v[8];
        v[0] = *(it++);
        v[1] = *(it++);
        v[2] = *(it++);
        v[3] = *(it++);
        v[4] = *(it++);
        v[5] = *(it++);
        v[6] = *(it++);
        v[7] = *(it++);
#ifndef SCCRT_SERIAL_IMPL
        COMPILER_BARRIER();
#endif
        v[0] = o(v[0], v[1]);
        v[2] = o(v[2], v[3]);
        v[4] = o(v[4], v[5]);
        v[6] = o(v[6], v[7]);
        v[0] = o(v[0], v[2]);
        v[4] = o(v[4], v[6]);
        accumulator = o(accumulator, v[0]);
        accumulator = o(accumulator, v[4]);
    }
    while (it != end) accumulator = o(accumulator, *(it++));
    free(r);
    return accumulator;
}


} // anonymous namespace


template <typename T>
const uint32_t Reducer<T>::size =
#ifndef SCCRT_SERIAL_IMPL
        swarm::num_threads();
#else
        1;
#endif


#define SCCRT_REDUCTION_INIT_DEFINITION(type)                                  \
    void* __sccrt_reduction_##type##_init(type initial, type identity) {       \
        return init<type>(initial, identity);                                  \
    }

#define SCCRT_REDUCTION_DEFINITIONS(type, op)                                  \
    void __sccrt_reduction_##type##_##op(uint64_t ts, void* r, type value) {   \
        update<type>(ts, r, value, std::op<type>());                           \
    }                                                                          \
    type __sccrt_reduction_##type##_##op##_collapse(void* r) {                 \
        return collapse<type>(r, std::op<type>());                             \
    }

// The following definitions are necessary because, unlike with std::plus,
// std::multiplies, etc., std::min and std::max are not functors
#define SCCRT_REDUCTION_MINMAX_DEFINITIONS(type, op)                           \
    void __sccrt_reduction_##type##_##op(uint64_t ts, void* r, type value) {   \
        update<type>(ts, r, value,                                             \
                [] (type l, type r) { return std::op<type>(l,r); });           \
    }                                                                          \
    type __sccrt_reduction_##type##_##op##_collapse(void* r) {                 \
        return collapse<type>(r,                                               \
                [] (type l, type r) { return std::op<type>(l,r); });           \
    }


extern "C" {

SCCRT_REDUCTION_INIT_DEFINITION(uint64_t)
SCCRT_REDUCTION_INIT_DEFINITION(float)
SCCRT_REDUCTION_INIT_DEFINITION(double)

SCCRT_REDUCTION_DEFINITIONS(uint64_t, plus)
SCCRT_REDUCTION_DEFINITIONS(uint64_t, multiplies)
SCCRT_REDUCTION_DEFINITIONS(uint64_t, bit_or)
SCCRT_REDUCTION_DEFINITIONS(uint64_t, bit_and)
SCCRT_REDUCTION_DEFINITIONS(uint64_t, bit_xor)
SCCRT_REDUCTION_DEFINITIONS(float, plus)
SCCRT_REDUCTION_DEFINITIONS(float, multiplies)
SCCRT_REDUCTION_DEFINITIONS(double, plus)
SCCRT_REDUCTION_DEFINITIONS(double, multiplies)

SCCRT_REDUCTION_MINMAX_DEFINITIONS(uint64_t, min)
SCCRT_REDUCTION_MINMAX_DEFINITIONS(uint64_t, max)
SCCRT_REDUCTION_MINMAX_DEFINITIONS(int64_t, min)
SCCRT_REDUCTION_MINMAX_DEFINITIONS(int64_t, max)

SCCRT_REDUCTION_MINMAX_DEFINITIONS(float, min)
SCCRT_REDUCTION_MINMAX_DEFINITIONS(float, max)
SCCRT_REDUCTION_MINMAX_DEFINITIONS(double, min)
SCCRT_REDUCTION_MINMAX_DEFINITIONS(double, max)

}  // extern "C"
