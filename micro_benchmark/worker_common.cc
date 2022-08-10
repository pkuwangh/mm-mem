#include <chrono>
#include <iostream>
#include <thread>

#include "worker_common.h"

namespace mm_worker {

void kernel_dummy(uint64_t& ret, uint64_t*& p) {
    std::cerr << "invalid read/write mix" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::exit(1);
}

}
