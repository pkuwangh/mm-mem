#ifndef __KERNELS_MEMCPY_H__
#define __KERNELS_MEMCPY_H__

#include <cstring>

namespace mm_worker {

// memcpy
static inline __attribute__ (( always_inline ))
void* glibc_memcpy(void* dest, const void* src, std::size_t count) {
    return ::memcpy(dest, src, count);
}

}

#endif
