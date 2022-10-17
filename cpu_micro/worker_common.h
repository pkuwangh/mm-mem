#ifndef __WORKER_COMMON_H__
#define __WORKER_COMMON_H__

#include <cstdint>
#include <functional>

namespace mm_worker {

#define TIMER_THRESHOLD (double)1.01

using func_kernel_lat = std::function<void(uint64_t&, uint64_t*&)>;
using func_kernel_bw = std::function<void(uint64_t&, uint64_t*&)>;
using func_kernel_memcpy = std::function<void*(void*, const void*, std::size_t)>;

// calculate write bandwidth
float get_write_fraction(uint32_t read_write_mix);

}

#endif
