#include "worker_kernels_common.h"
#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r1w1_s_n768 (u64& ret, u64*& p) { LP128(RD32 WR32 LP4(MN768)) }

}