#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r1w0_s_n512 (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP512();) }
void k_r1w0_s_n768 (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP768();) }

}
