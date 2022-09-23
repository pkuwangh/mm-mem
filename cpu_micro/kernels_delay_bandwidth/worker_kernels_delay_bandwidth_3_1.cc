#include "cpu_micro/worker_kernels_common.h"
#include "cpu_micro/worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r3w1_s_n104 (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN104)) LP3(RD32) LP1(WR32) LP4(MN104) }
void k_r3w1_s_n112 (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN112)) LP3(RD32) LP1(WR32) LP4(MN112) }
void k_r3w1_s_n128 (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN128)) LP3(RD32) LP1(WR32) LP4(MN128) }

}
