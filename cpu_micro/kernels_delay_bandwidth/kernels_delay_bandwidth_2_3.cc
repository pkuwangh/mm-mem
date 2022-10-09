#include "cpu_micro/kernels_common.h"
#include "cpu_micro/kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r2w1_s_n320 (u64& ret, u64*& p) { LP64(LP3(RD32) WR32 LP6(MN320)) }
void k_r2w1_s_n384 (u64& ret, u64*& p) { LP64(LP3(RD32) WR32 LP6(MN384)) }
void k_r2w1_s_n448 (u64& ret, u64*& p) { LP64(LP3(RD32) WR32 LP6(MN448)) }

}
