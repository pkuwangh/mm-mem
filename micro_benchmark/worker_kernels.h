#ifndef __WORKER_KERNELS_H__
#define __WORKER_KERNELS_H__

#include <iostream>
#include <chrono>
#include <thread>
#include <tuple>
#include <vector>

#include "worker_common.h"

namespace mm_worker {

using kernel_function = std::function<void(uint64_t&, uint64_t*&)>;

// latency
void kernel_lat(uint64_t& ret, uint64_t*& p) {
    LOOP256(p = reinterpret_cast<uint64_t*>(*p);)
    ret += *p;
}

// dummy one for invalid configs
void kernel_dummy(uint64_t& ret, uint64_t*& p) {
    std::cerr << "invalid read/write mix" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::exit(1);
}

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

// variable bandwidth for loaded latency
void kernel_bw_r1w0_sequential_nop1024(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP1024();)
}

void kernel_bw_r1w0_sequential_nop768(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP768();)
}

void kernel_bw_r1w0_sequential_nop512(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP512();)
}

void kernel_bw_r1w0_sequential_nop384(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP384();)
}

void kernel_bw_r1w0_sequential_nop256(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP256();)
}

void kernel_bw_r1w0_sequential_nop224(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP224();)
}

void kernel_bw_r1w0_sequential_nop192(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP192();)
}

void kernel_bw_r1w0_sequential_nop160(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP160();)
}

void kernel_bw_r1w0_sequential_nop128(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP128();)
}

void kernel_bw_r1w0_sequential_nop96(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP96();)
}

void kernel_bw_r1w0_sequential_nop64(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP64();)
}

void kernel_bw_r1w0_sequential_nop48(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP48();)
}

void kernel_bw_r1w0_sequential_nop32(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP32();)
}

void kernel_bw_r1w0_sequential_nop16(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP16();)
}

void kernel_bw_r1w0_sequential_nop8(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP8();)
}

void kernel_bw_r1w0_sequential_nop4(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP4();)
}

void kernel_bw_r1w0_sequential_nop2(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP2();)
}

void kernel_bw_r1w0_sequential_nop1(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4; MY_NOP1();)
}

void kernel_bw_r1w0_sequential_nop0(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4;)
}

void get_kernels_with_delays(
    std::vector<std::tuple<uint32_t, kernel_function>>& delays_and_kernels,
    uint32_t read_write_mix
) {
    if (read_write_mix == 0) {
        delays_and_kernels.push_back({0,    kernel_bw_r1w0_sequential_nop0});
        delays_and_kernels.push_back({1,    kernel_bw_r1w0_sequential_nop1});
        delays_and_kernels.push_back({2,    kernel_bw_r1w0_sequential_nop2});
        delays_and_kernels.push_back({4,    kernel_bw_r1w0_sequential_nop4});
        delays_and_kernels.push_back({8,    kernel_bw_r1w0_sequential_nop8});
        delays_and_kernels.push_back({16,   kernel_bw_r1w0_sequential_nop16});
        delays_and_kernels.push_back({32,   kernel_bw_r1w0_sequential_nop32});
        delays_and_kernels.push_back({48,   kernel_bw_r1w0_sequential_nop48});
        delays_and_kernels.push_back({64,   kernel_bw_r1w0_sequential_nop64});
        delays_and_kernels.push_back({96,   kernel_bw_r1w0_sequential_nop96});
        delays_and_kernels.push_back({128,  kernel_bw_r1w0_sequential_nop128});
        delays_and_kernels.push_back({160,  kernel_bw_r1w0_sequential_nop160});
        delays_and_kernels.push_back({192,  kernel_bw_r1w0_sequential_nop192});
        delays_and_kernels.push_back({224,  kernel_bw_r1w0_sequential_nop224});
        delays_and_kernels.push_back({256,  kernel_bw_r1w0_sequential_nop256});
        delays_and_kernels.push_back({384,  kernel_bw_r1w0_sequential_nop384});
        delays_and_kernels.push_back({512,  kernel_bw_r1w0_sequential_nop512});
        delays_and_kernels.push_back({768,  kernel_bw_r1w0_sequential_nop768});
        delays_and_kernels.push_back({1024, kernel_bw_r1w0_sequential_nop1024});
    } else {
        delays_and_kernels.push_back({0, kernel_dummy});
    }
}

}

#endif
