#include "cpu_micro/kernels_common.h"
#include "cpu_micro/kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r2w1_s_n320 (u64& ret, u64*& p) { LP32(LP4(RD32) LP2(RD32 WR32) LP3(MN320)) }
void k_r2w1_s_n384 (u64& ret, u64*& p) { LP32(LP4(RD32) LP2(RD32 WR32) LP3(MN384)) }
void k_r2w1_s_n448 (u64& ret, u64*& p) { LP32(LP4(RD32) LP2(RD32 WR32) LP3(MN448)) }

}
