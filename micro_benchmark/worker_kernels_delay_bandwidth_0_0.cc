#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void get_kernels_with_delays(
    std::vector<std::tuple<uint32_t, kernel_function>>& delays_and_kernels,
    uint32_t read_write_mix
) {
    if (read_write_mix == 0) {
        delays_and_kernels.push_back({0,    k_r1w0_s_n0});
        delays_and_kernels.push_back({1,    k_r1w0_s_n1});
        delays_and_kernels.push_back({2,    k_r1w0_s_n2});
        delays_and_kernels.push_back({4,    k_r1w0_s_n4});
        delays_and_kernels.push_back({8,    k_r1w0_s_n8});
        delays_and_kernels.push_back({16,   k_r1w0_s_n16});
        delays_and_kernels.push_back({32,   k_r1w0_s_n32});
        delays_and_kernels.push_back({48,   k_r1w0_s_n48});
        delays_and_kernels.push_back({64,   k_r1w0_s_n64});
        delays_and_kernels.push_back({96,   k_r1w0_s_n96});
        delays_and_kernels.push_back({128,  k_r1w0_s_n128});
        delays_and_kernels.push_back({160,  k_r1w0_s_n160});
        delays_and_kernels.push_back({192,  k_r1w0_s_n192});
        delays_and_kernels.push_back({224,  k_r1w0_s_n224});
        delays_and_kernels.push_back({256,  k_r1w0_s_n256});
        delays_and_kernels.push_back({384,  k_r1w0_s_n384});
        delays_and_kernels.push_back({512,  k_r1w0_s_n512});
        delays_and_kernels.push_back({768,  k_r1w0_s_n768});
        delays_and_kernels.push_back({1024, k_r1w0_s_n1024});
    } else {
        delays_and_kernels.push_back({0, kernel_dummy});
    }
}

void k_r1w0_s_n0   (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4;) }
void k_r1w0_s_n1   (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP1();) }
void k_r1w0_s_n2   (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP2();) }
void k_r1w0_s_n4   (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP4();) }
void k_r1w0_s_n8   (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP8();) }
void k_r1w0_s_n16  (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP16();) }
void k_r1w0_s_n32  (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP32();) }
void k_r1w0_s_n48  (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP48();) }
void k_r1w0_s_n64  (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP64();) }
void k_r1w0_s_n96  (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP96();) }
void k_r1w0_s_n128 (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP128();) }

}
