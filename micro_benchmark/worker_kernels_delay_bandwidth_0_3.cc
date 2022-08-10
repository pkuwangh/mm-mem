#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r1w0_s_n1024(u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP1024();) }

}
