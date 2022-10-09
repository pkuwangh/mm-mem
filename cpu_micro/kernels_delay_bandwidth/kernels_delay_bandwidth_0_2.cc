#include "cpu_micro/kernels_common.h"
#include "cpu_micro/kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r1w0_s_n160 (u64& ret, u64*& p) { LP256(RD32 MN160) }
void k_r1w0_s_n192 (u64& ret, u64*& p) { LP256(RD32 MN192) }
void k_r1w0_s_n224 (u64& ret, u64*& p) { LP256(RD32 MN224) }

}
