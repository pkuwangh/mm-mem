#include "worker_kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r1w0_s_n160 (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP160();) }
void k_r1w0_s_n192 (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP192();) }
void k_r1w0_s_n224 (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP224();) }
void k_r1w0_s_n256 (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP256();) }
void k_r1w0_s_n384 (u64& ret, u64*& p) { LOOP256(ret += *p; p += 4; MY_NOP384();) }

}
