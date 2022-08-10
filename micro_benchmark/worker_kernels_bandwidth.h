#ifndef __WORKER_KERNELS_BW_H__
#define __WORKER_KERNELS_BW_H__

#include "worker_common.h"

namespace mm_worker {

// peak bandwidth
void kernel_bw_r1w0_sequential(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4;)
}

void kernel_bw_r1w1_sequential(uint64_t& ret, uint64_t*& p) {
    LOOP128(ret += *p; p += 4; *p = ret; p += 4;)
}

void kernel_bw_r2w1_sequential(uint64_t& ret, uint64_t*& p) {
    LOOP64(ret += *p; p += 4; ret += *p; p += 4; ret += *p; p += 4; *p = ret; p += 4;)
}

kernel_function get_kernel_peak_load(uint32_t read_write_mix) {
    if (read_write_mix == 0) {
        return kernel_bw_r1w0_sequential;
    } else if (read_write_mix == 1) {
        return kernel_bw_r1w1_sequential;
    } else if (read_write_mix == 2) {
        return kernel_bw_r2w1_sequential;
    } else {
        return kernel_dummy;
    }
}

}

#endif
