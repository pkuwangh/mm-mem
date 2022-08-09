#include <iomanip>
#include <iostream>
#include <string>
#include <cassert>
#include <cstdint>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>

#include "lib_configuration.h"
#include "lib_mem_region.h"
#include "lib_timing.h"
#include "worker_latency.h"

int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::LATENCY);
    if (config.parse_options(argc, argv)) {
        return 1;
    }
    config.dump();
    // setup memory regions & workers
    std::vector<mm_utils::MemRegion::Handle> regions(config.num_threads, nullptr);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        regions[i] = std::make_shared<mm_utils::MemRegion>(
            config.region_size_kb * 1024,
            config.chunk_size_kb * 1024,
            config.stride_size_b,
            static_cast<mm_utils::HugePageType>(config.use_hugepage)
        );
        if (config.access_pattern == 0) {
            regions[i]->stride_init();
        } else if (config.access_pattern == 1) {
            regions[i]->page_random_init();
        } else if (config.access_pattern == 2) {
            regions[i]->all_random_init();
        } else {
            std::cerr << "invalid access pattern" << std::endl;
            return 1;
        }
        // regions[i]->dump();
    }
    std::vector<std::shared_ptr<std::thread>> workers(config.num_threads, nullptr);
    std::vector<uint64_t> finished_chases(config.num_threads, 0);
    std::vector<double> exec_time(config.num_threads, 0);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i] = std::make_shared<std::thread>(
            mm_worker::lat_ptr,
            regions[i],
            config.target_duration_s,
            &finished_chases[i],
            &exec_time[i]
        );
    }
    // done
    uint64_t total_chases = 0;
    double total_exec_time = 0;
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i]->join();
        total_chases += finished_chases[i];
        total_exec_time += exec_time[i];
    }
    double latency = total_exec_time * 1e9 / total_chases;
    std::cout << "Idle Latency: " << std::setprecision(4) << latency << " ns" << std::endl;
    return 0;
}
