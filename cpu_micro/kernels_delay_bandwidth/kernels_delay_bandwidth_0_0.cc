#include "cpu_micro/kernels_common.h"
#include "cpu_micro/kernels_delay_bandwidth.h"

namespace mm_worker {

void k_r1w0_s_n0   (u64& ret, u64*& p) { LP256(RD32    ) }
void k_r1w0_s_n1   (u64& ret, u64*& p) { LP256(RD32 MN1) }
void k_r1w0_s_n2   (u64& ret, u64*& p) { LP256(RD32 MN2) }
void k_r1w0_s_n4   (u64& ret, u64*& p) { LP256(RD32 MN4) }
void k_r1w0_s_n8   (u64& ret, u64*& p) { LP256(RD32 MN8) }
void k_r1w0_s_n16  (u64& ret, u64*& p) { LP256(RD32 MN16) }
void k_r1w0_s_n32  (u64& ret, u64*& p) { LP256(RD32 MN32) }
void k_r1w0_s_n48  (u64& ret, u64*& p) { LP256(RD32 MN48) }
void k_r1w0_s_n64  (u64& ret, u64*& p) { LP256(RD32 MN64) }
void k_r1w0_s_n80  (u64& ret, u64*& p) { LP256(RD32 MN80) }
void k_r1w0_s_n88  (u64& ret, u64*& p) { LP256(RD32 MN88) }
void k_r1w0_s_n96  (u64& ret, u64*& p) { LP256(RD32 MN96) }

}
