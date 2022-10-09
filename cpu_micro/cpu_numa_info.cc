#include <cstdint>
#include <iostream>

#include "common/numa_config.h"

int main(int argc, char** argv) {
    mm_utils::NumaConfig numa_config;
    numa_config.dump();
    return 0;
}
