#include <chrono>
#include <iostream>
#include <thread>

#include "cpu_micro/worker_common.h"

namespace mm_worker {

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
