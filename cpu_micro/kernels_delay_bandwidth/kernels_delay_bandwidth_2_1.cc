#include "cpu_micro/kernels_common.h"
#include "cpu_micro/kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r2w1_s_n104 (u64& ret, u64*& p) { LP32(LP4(RD32) LP2(RD32 WR32) LP3(MN104)) }
void k_r2w1_s_n112 (u64& ret, u64*& p) { LP32(LP4(RD32) LP2(RD32 WR32) LP3(MN112)) }
void k_r2w1_s_n128 (u64& ret, u64*& p) { LP32(LP4(RD32) LP2(RD32 WR32) LP3(MN128)) }

}
