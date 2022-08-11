#ifndef __WORKER_KERNELS_BW_H__
#define __WORKER_KERNELS_BW_H__

#include <tuple>
#include <vector>

#include "worker_common.h"

namespace mm_worker {

// peak bandwidth
void kernel_bw_r1w0_sequential(uint64_t& ret, uint64_t*& p) {
    // 256x1
    LP256(RD32)
}

void kernel_bw_r1w1_sequential(uint64_t& ret, uint64_t*& p) {
    // 128x2
    LP128(RD32 WR32)
}

void kernel_bw_r2w1_sequential(uint64_t& ret, uint64_t*& p) {
    // 64x4
    LP64(LP3(RD32) WR32)
}

void kernel_bw_r3w1_sequential(uint64_t& ret, uint64_t*& p) {
    // 42x6 + 4
    LP42(LP5(RD32) WR32)
    LP3(RD32)
    LP1(WR32)
}


void get_kernels_with_wrmix(
    std::vector<std::tuple<uint32_t, kernel_function>>& rwmix_and_kernels,
    uint32_t read_write_mix
) {
    if (read_write_mix == 0) {
        rwmix_and_kernels.push_back({0, kernel_bw_r1w0_sequential});
    } else if (read_write_mix == 1) {
        rwmix_and_kernels.push_back({1, kernel_bw_r1w1_sequential});
    } else if (read_write_mix == 2) {
        rwmix_and_kernels.push_back({2, kernel_bw_r2w1_sequential});
    } else if (read_write_mix == 3) {
        rwmix_and_kernels.push_back({3, kernel_bw_r3w1_sequential});
    } else if (read_write_mix == 100) {
        rwmix_and_kernels.push_back({0, kernel_bw_r1w0_sequential});
        rwmix_and_kernels.push_back({3, kernel_bw_r3w1_sequential});
        rwmix_and_kernels.push_back({2, kernel_bw_r2w1_sequential});
        rwmix_and_kernels.push_back({1, kernel_bw_r1w1_sequential});
    } else {
        rwmix_and_kernels.push_back({0, kernel_dummy});
    }
}

}

#endif
