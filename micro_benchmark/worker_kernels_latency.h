#ifndef __WORKER_KERNELS_LATENCY_H__
#define __WORKER_KERNELS_LATENCY_H__

#include "worker_kernels_common.h"

namespace mm_worker {

// latency
void kernel_lat(uint64_t& ret, uint64_t*& p) {
    LP256(p = reinterpret_cast<uint64_t*>(*p);)
    ret += *p;
}

}

#endif
