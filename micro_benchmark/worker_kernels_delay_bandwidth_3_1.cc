#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r3w1_s_n160 (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN160)) LP3(RD32) LP1(WR32) LP4(MN160) }
void k_r3w1_s_n192 (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN192)) LP3(RD32) LP1(WR32) LP4(MN192) }
void k_r3w1_s_n224 (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN224)) LP3(RD32) LP1(WR32) LP4(MN224) }
void k_r3w1_s_n256 (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN256)) LP3(RD32) LP1(WR32) LP4(MN256) }
void k_r3w1_s_n384 (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN384)) LP3(RD32) LP1(WR32) LP4(MN384) }

}
