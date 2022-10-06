#ifndef __WORKER_KERNELS_DELAY_BW_H__
#define __WORKER_KERNELS_DELAY_BW_H__

#include <list>
#include <tuple>
#include <vector>

#include "cpu_micro/worker_common.h"
#include "cpu_micro/worker_kernels_common.h"

namespace mm_worker {

#define DELAY_N   n = delay; while (n > 0) { MN1; --n; }

// variable bandwidth for loaded latency
void kernel_r1w0_seq(uint64_t& ret, uint64_t*& p, uint32_t delay) {
    uint32_t n = 0;
    LP256(RD32 DELAY_N);
}

void kernel_r1w1_seq(uint64_t& ret, uint64_t*& p, uint32_t delay) {
    uint32_t n = 0;
    LP128(RD32 WR32 LP4(DELAY_N));
}

void kernel_r2w1_seq(uint64_t& ret, uint64_t*& p, uint32_t delay) {
    uint32_t n = 0;
    LP64(LP3(RD32) WR32 LP6(DELAY_N));
}

void kernel_r3w1_seq(uint64_t& ret, uint64_t*& p, uint32_t delay) {
    uint32_t n = 0;
    LP42(LP5(RD32) WR32 LP8(DELAY_N));
    LP3(RD32) LP1(WR32) LP4(DELAY_N);
}

void get_kernels_with_delays(
    std::list<std::tuple<uint32_t, func_kernel_bw>>& delays_and_kernels,
    uint32_t read_write_mix
) {
    std::vector<uint32_t> delays = {
        1, 8, 32, 48, 64, 80, 88, 96,
        104, 112, 128, 160, 192,
        224, 256, 320, 384, 484,
        512, 640, 768, 1024,
    };
    for (auto delay : delays) {
        if (read_write_mix == 0) {
            delays_and_kernels.push_back({delay, kernel_r1w0_seq});
        } else if (read_write_mix == 1) {
            delays_and_kernels.push_back({delay, kernel_r1w1_seq});
        } else if (read_write_mix == 2) {
            delays_and_kernels.push_back({delay, kernel_r2w1_seq});
        } else if (read_write_mix == 3) {
            delays_and_kernels.push_back({delay, kernel_r3w1_seq});
        } else {
            delays_and_kernels.push_back({delay, kernel_r2w1_seq});
        }
    }
}

}

#endif
