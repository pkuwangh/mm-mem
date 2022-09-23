#include "cpu_micro/worker_kernels_common.h"
#include "cpu_micro/worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r2w1_s_n512 (u64& ret, u64*& p) { LP64(LP3(RD32) WR32 LP6(MN512)) }
void k_r2w1_s_n640 (u64& ret, u64*& p) { LP64(LP3(RD32) WR32 LP6(MN640)) }

}
