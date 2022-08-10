#ifndef __WORKER_KERNELS_DELAY_BW_H__
#define __WORKER_KERNELS_DELAY_BW_H__

#include <tuple>
#include <vector>

#include "worker_common.h"

namespace mm_worker {

using u64 = uint64_t;

// variable bandwidth for loaded latency
void get_kernels_with_delays(
    std::vector<std::tuple<uint32_t, kernel_function>>& delays_and_kernels,
    uint32_t read_write_mix
);

void k_r1w0_s_n0(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n1(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n2(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n4(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n8(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n16(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n32(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n48(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n64(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n96(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n128(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n160(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n192(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n224(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n256(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n384(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n512(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n768(uint64_t& ret, uint64_t*& p);
void k_r1w0_s_n1024(uint64_t& ret, uint64_t*& p);

}

#endif