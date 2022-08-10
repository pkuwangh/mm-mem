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


float get_write_fraction(uint32_t read_write_mix) {
    if (read_write_mix == 1) {
        return 1.0;
    } else if (read_write_mix == 2) {
        return 0.5;
    } else if (read_write_mix == 3) {
        return 0.3333;
    } else {
        return 0;
    }
}

}
