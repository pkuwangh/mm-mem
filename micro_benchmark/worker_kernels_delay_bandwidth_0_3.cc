#include "worker_kernels_common.h"
#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r1w0_s_n256 (u64& ret, u64*& p) { LP256(RD32 MN256) }
void k_r1w0_s_n384 (u64& ret, u64*& p) { LP256(RD32 MN384) }

}
