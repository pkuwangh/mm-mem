#include "worker_kernels_common.h"
#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r2w1_s_n160 (u64& ret, u64*& p) { LP64(LP3(RD32) WR32 LP6(MN160)) }
void k_r2w1_s_n192 (u64& ret, u64*& p) { LP64(LP3(RD32) WR32 LP6(MN192)) }
void k_r2w1_s_n224 (u64& ret, u64*& p) { LP64(LP3(RD32) WR32 LP6(MN224)) }
void k_r2w1_s_n256 (u64& ret, u64*& p) { LP64(LP3(RD32) WR32 LP6(MN256)) }

}
