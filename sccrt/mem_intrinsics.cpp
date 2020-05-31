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

// Memory instrinsic implementations for use by the Swarm compiler.

#include <scc/rt.h>

#ifndef SCCRT_SERIAL_IMPL

#ifndef SWARM_RUNTIME
#error "Low-level SCC runtime functions expect manual SWARM_RUNTIME tasks"
#endif

#include "swarm/api.h"
#include "swarm/algorithm.h"

#endif

#include <cstring>
#include <cstdlib>
#include <stddef.h>

extern "C" {

void __sccrt_memset(void* dest, uint8_t val, uint64_t count) {
#ifndef SCCRT_SERIAL_IMPL
    if (count >= 16 * SWARM_CACHE_LINE) {
        swarm::deepen();

        uint8_t* bytes = (uint8_t*)dest;
        swarm::fill<EnqFlags::NOHINT, uint8_t*, uint8_t>(
                bytes, bytes + count, val, 0ul);

        swarm::undeepen();

        return;
    }
#endif

    std::memset(dest, val, count);
}


void __sccrt_memcpy(void* dest, const void* src, uint64_t count) {
#ifndef SCCRT_SERIAL_IMPL
    if (count >= 16 * SWARM_CACHE_LINE) {
        swarm::deepen();

        swarm::copy<EnqFlags::NOHINT, const uint8_t*, uint8_t*>(
                (const uint8_t*) src,
                (const uint8_t*) src + count,
                (uint8_t*) dest,
                0ul);

        swarm::undeepen();

        return;
    }
#endif

    std::memcpy(dest, src, count);
}

typedef void (*contFuncPtr)(void* ptr, void* cont);
#ifndef SCCRT_SERIAL_IMPL
static void calloc_cont(swarm::Timestamp, void* ptr, void* cont) {
    (**(contFuncPtr*)cont)(ptr, cont);
}
void __sccrt_calloc(size_t num, size_t size, void* cont) {
    assert(cont);
    const size_t total = num * size;
    void* ptr = std::malloc(total);
    if (ptr) __sccrt_memset(ptr, 0, total);
    swarm::enqueue(calloc_cont, swarm::timestamp() + 1, EnqFlags::NOHINT, ptr, cont);
}
#else   // SCCRT_SERIAL_IMPL
void __sccrt_calloc(size_t num, size_t size, void* cont) {
    void* ptr = std::calloc(num, size);
    (**(contFuncPtr*)cont)(ptr, cont);
}
#endif  // SCCRT_SERIAL_IMPL

}  // extern "C"
