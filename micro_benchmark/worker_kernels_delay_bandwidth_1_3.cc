#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r1w1_s_n1024(u64& ret, u64*& p) { LP128(RD32 WR32 LP4(MN1024)) }

}
