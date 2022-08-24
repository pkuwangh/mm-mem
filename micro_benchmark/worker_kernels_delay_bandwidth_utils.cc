#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void get_kernels_with_delays(
    std::list<std::tuple<uint32_t, kernel_function>>& delays_and_kernels,
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
        delays_and_kernels.push_back({80,   k_r1w0_s_n80});
        delays_and_kernels.push_back({88,   k_r1w0_s_n88});
        delays_and_kernels.push_back({96,   k_r1w0_s_n96});
        delays_and_kernels.push_back({104,  k_r1w0_s_n104});
        delays_and_kernels.push_back({112,  k_r1w0_s_n112});
        delays_and_kernels.push_back({128,  k_r1w0_s_n128});
        delays_and_kernels.push_back({160,  k_r1w0_s_n160});
        delays_and_kernels.push_back({192,  k_r1w0_s_n192});
        delays_and_kernels.push_back({224,  k_r1w0_s_n224});
        delays_and_kernels.push_back({256,  k_r1w0_s_n256});
        delays_and_kernels.push_back({384,  k_r1w0_s_n384});
        delays_and_kernels.push_back({512,  k_r1w0_s_n512});
        delays_and_kernels.push_back({768,  k_r1w0_s_n768});
        delays_and_kernels.push_back({1024, k_r1w0_s_n1024});
    } else if (read_write_mix == 1) {
        delays_and_kernels.push_back({0,    k_r1w1_s_n0});
        delays_and_kernels.push_back({1,    k_r1w1_s_n1});
        delays_and_kernels.push_back({2,    k_r1w1_s_n2});
        delays_and_kernels.push_back({4,    k_r1w1_s_n4});
        delays_and_kernels.push_back({8,    k_r1w1_s_n8});
        delays_and_kernels.push_back({16,   k_r1w1_s_n16});
        delays_and_kernels.push_back({32,   k_r1w1_s_n32});
        delays_and_kernels.push_back({48,   k_r1w1_s_n48});
        delays_and_kernels.push_back({64,   k_r1w1_s_n64});
        delays_and_kernels.push_back({80,   k_r1w1_s_n80});
        delays_and_kernels.push_back({88,   k_r1w1_s_n88});
        delays_and_kernels.push_back({96,   k_r1w1_s_n96});
        delays_and_kernels.push_back({104,  k_r1w1_s_n104});
        delays_and_kernels.push_back({112,  k_r1w1_s_n112});
        delays_and_kernels.push_back({128,  k_r1w1_s_n128});
        delays_and_kernels.push_back({160,  k_r1w1_s_n160});
        delays_and_kernels.push_back({192,  k_r1w1_s_n192});
        delays_and_kernels.push_back({224,  k_r1w1_s_n224});
        delays_and_kernels.push_back({256,  k_r1w1_s_n256});
        delays_and_kernels.push_back({384,  k_r1w1_s_n384});
        delays_and_kernels.push_back({512,  k_r1w1_s_n512});
        delays_and_kernels.push_back({768,  k_r1w1_s_n768});
        delays_and_kernels.push_back({1024, k_r1w1_s_n1024});
    } else if (read_write_mix == 2) {
        // delays_and_kernels.push_back({0,    k_r2w1_s_n0});
        delays_and_kernels.push_back({1,    k_r2w1_s_n1});
        // delays_and_kernels.push_back({2,    k_r2w1_s_n2});
        // delays_and_kernels.push_back({4,    k_r2w1_s_n4});
        delays_and_kernels.push_back({8,    k_r2w1_s_n8});
        // delays_and_kernels.push_back({16,   k_r2w1_s_n16});
        delays_and_kernels.push_back({32,   k_r2w1_s_n32});
        delays_and_kernels.push_back({48,   k_r2w1_s_n48});
        delays_and_kernels.push_back({64,   k_r2w1_s_n64});
        delays_and_kernels.push_back({80,   k_r2w1_s_n80});
        delays_and_kernels.push_back({88,   k_r2w1_s_n88});
        delays_and_kernels.push_back({96,   k_r2w1_s_n96});
        delays_and_kernels.push_back({104,  k_r2w1_s_n104});
        delays_and_kernels.push_back({112,  k_r2w1_s_n112});
        delays_and_kernels.push_back({128,  k_r2w1_s_n128});
        delays_and_kernels.push_back({160,  k_r2w1_s_n160});
        delays_and_kernels.push_back({192,  k_r2w1_s_n192});
        delays_and_kernels.push_back({224,  k_r2w1_s_n224});
        delays_and_kernels.push_back({256,  k_r2w1_s_n256});
        delays_and_kernels.push_back({320,  k_r2w1_s_n320});
        delays_and_kernels.push_back({384,  k_r2w1_s_n384});
        delays_and_kernels.push_back({448,  k_r2w1_s_n448});
        delays_and_kernels.push_back({512,  k_r2w1_s_n512});
        delays_and_kernels.push_back({640,  k_r2w1_s_n640});
        delays_and_kernels.push_back({768,  k_r2w1_s_n768});
        delays_and_kernels.push_back({1024, k_r2w1_s_n1024});
    } else if (read_write_mix == 3) {
        delays_and_kernels.push_back({0,    k_r3w1_s_n0});
        delays_and_kernels.push_back({1,    k_r3w1_s_n1});
        delays_and_kernels.push_back({2,    k_r3w1_s_n2});
        delays_and_kernels.push_back({4,    k_r3w1_s_n4});
        delays_and_kernels.push_back({8,    k_r3w1_s_n8});
        delays_and_kernels.push_back({16,   k_r3w1_s_n16});
        delays_and_kernels.push_back({32,   k_r3w1_s_n32});
        delays_and_kernels.push_back({48,   k_r3w1_s_n48});
        delays_and_kernels.push_back({64,   k_r3w1_s_n64});
        delays_and_kernels.push_back({80,   k_r3w1_s_n80});
        delays_and_kernels.push_back({88,   k_r3w1_s_n88});
        delays_and_kernels.push_back({96,   k_r3w1_s_n96});
        delays_and_kernels.push_back({104,  k_r3w1_s_n104});
        delays_and_kernels.push_back({112,  k_r3w1_s_n112});
        delays_and_kernels.push_back({128,  k_r3w1_s_n128});
        delays_and_kernels.push_back({160,  k_r3w1_s_n160});
        delays_and_kernels.push_back({192,  k_r3w1_s_n192});
        delays_and_kernels.push_back({224,  k_r3w1_s_n224});
        delays_and_kernels.push_back({256,  k_r3w1_s_n256});
        delays_and_kernels.push_back({384,  k_r3w1_s_n384});
        delays_and_kernels.push_back({512,  k_r3w1_s_n512});
        delays_and_kernels.push_back({768,  k_r3w1_s_n768});
        delays_and_kernels.push_back({1024, k_r3w1_s_n1024});
    } else {
        delays_and_kernels.push_back({0, kernel_dummy});
    }
}

}
