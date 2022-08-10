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
#include "worker_bandwidth.h"
#include "worker_kernels_bandwidth.h"

int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::BANDWIDTH);
    if (config.parse_options(argc, argv)) {
        return 1;
    }
    config.dump();
    // setup memory regions & workers
    std::vector<mm_utils::MemRegion::Handle> regions(config.num_threads, nullptr);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        regions[i] = std::make_shared<mm_utils::MemRegion>(
            config.region_size_kb * 1024, 4096, 64
        );
    }
    std::vector<std::shared_ptr<std::thread>> workers(config.num_threads, nullptr);
    std::vector<uint64_t> finished_bytes(config.num_threads, 0);
    std::vector<double> exec_time(config.num_threads, 0);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i] = std::make_shared<std::thread>(
            mm_worker::bw_sequential,
            mm_worker::get_kernel_peak_load(config.read_write_mix),
            regions[i],
            config.read_write_mix,
            config.target_duration_s,
            config.num_threads,
            &finished_bytes[i],
            &exec_time[i]
        );
    }
    // done
    uint64_t total_bytes = 0;
    double total_exec_time = 0;
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i]->join();
        total_bytes += finished_bytes[i];
        total_exec_time += exec_time[i];
    }
    double mem_bw = total_bytes / total_exec_time * config.num_threads;
    std::cout << "Total Bandwidth: " << mem_bw / 1024 / 1024 << " MB/s | "
        << mem_bw / 1024 / 1024 / 1024 << " GB/s" << std::endl;
    return 0;
}
