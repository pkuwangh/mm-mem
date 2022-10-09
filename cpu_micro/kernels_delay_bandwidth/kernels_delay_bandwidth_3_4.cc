#include "cpu_micro/kernels_common.h"
#include "cpu_micro/kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r3w1_s_n512 (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN512)) LP3(RD32) LP1(WR32) LP4(MN512) }

}
