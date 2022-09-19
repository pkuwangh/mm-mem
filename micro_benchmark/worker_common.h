#ifndef __WORKER_COMMON_H__
#define __WORKER_COMMON_H__

#include <cstdint>
#include <functional>

namespace mm_worker {

#define TIMER_THRESHOLD (double)1.002

using kernel_function = std::function<void(uint64_t&, uint64_t*&)>;

// dummy one for invalid configs
void kernel_dummy(uint64_t& ret, uint64_t*& p);

// calculate write bandwidth
float get_write_fraction(uint32_t read_write_mix);

using kernel_func_memcpy = std::function<void*(void*, const void*, std::size_t)>;

}

#endif
