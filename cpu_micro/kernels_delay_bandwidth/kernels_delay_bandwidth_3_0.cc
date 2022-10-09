#include "cpu_micro/kernels_common.h"
#include "cpu_micro/kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r3w1_s_n0   (u64& ret, u64*& p) { LP42(LP5(RD32) WR32           ) LP3(RD32) LP1(WR32) }
void k_r3w1_s_n1   (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN1)  ) LP3(RD32) LP1(WR32) LP4(MN1) }
void k_r3w1_s_n2   (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN2)  ) LP3(RD32) LP1(WR32) LP4(MN2) }
void k_r3w1_s_n4   (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN4)  ) LP3(RD32) LP1(WR32) LP4(MN4) }
void k_r3w1_s_n8   (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN8)  ) LP3(RD32) LP1(WR32) LP4(MN8) }
void k_r3w1_s_n16  (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN16) ) LP3(RD32) LP1(WR32) LP4(MN16) }
void k_r3w1_s_n32  (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN32) ) LP3(RD32) LP1(WR32) LP4(MN32) }
void k_r3w1_s_n48  (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN48) ) LP3(RD32) LP1(WR32) LP4(MN48) }
void k_r3w1_s_n64  (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN64) ) LP3(RD32) LP1(WR32) LP4(MN64) }
void k_r3w1_s_n80  (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN80) ) LP3(RD32) LP1(WR32) LP4(MN80) }
void k_r3w1_s_n88  (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN88) ) LP3(RD32) LP1(WR32) LP4(MN88) }
void k_r3w1_s_n96  (u64& ret, u64*& p) { LP42(LP5(RD32) WR32 LP8(MN96) ) LP3(RD32) LP1(WR32) LP4(MN96) }

}
