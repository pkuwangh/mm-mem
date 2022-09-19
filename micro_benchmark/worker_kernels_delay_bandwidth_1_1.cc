#include "worker_kernels_common.h"
#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r1w1_s_n104 (u64& ret, u64*& p) { LP128(RD32 WR32 LP4(MN104)) }
void k_r1w1_s_n112 (u64& ret, u64*& p) { LP128(RD32 WR32 LP4(MN112)) }
void k_r1w1_s_n128 (u64& ret, u64*& p) { LP128(RD32 WR32 LP4(MN128)) }

}
