#include <iomanip>
#include <iostream>
#include <string>
#include <cassert>
#include <cstdint>
#include <thread>
#include <vector>

#include "lib_configuration.h"
#include "lib_mem_region.h"
#include "lib_timing.h"
#include "worker_bandwidth.h"
#include "worker_kernels_delay_bandwidth.h"
#include "worker_kernels_latency.h"
#include "worker_latency.h"

void measure_loaded_latency(
    mm_utils::Configuration& config,
    std::vector<mm_utils::MemRegion::Handle>& regions,
    std::vector<std::shared_ptr<std::thread>>& workers,
    uint32_t delay,
    mm_worker::kernel_function& kernel
) {
    std::vector<uint64_t> finished_bytes(config.num_threads, 0);
    std::vector<double> exec_time(config.num_threads, 0);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i].reset();
    }
    // latency thread
    workers[0] = std::make_shared<std::thread>(
        mm_worker::lat_ptr,
        mm_worker::kernel_lat,
        regions[0],
        config.target_duration_s,
        &finished_bytes[0],
        &exec_time[0]
    );
    // bandwidth thread
    for (uint32_t i = 1; i < config.num_threads; ++i) {
        workers[i] = std::make_shared<std::thread>(
            mm_worker::bw_sequential,
            kernel,
            regions[i],
            config.read_write_mix,
            config.target_duration_s,
            config.num_threads,
            &finished_bytes[i],
            &exec_time[i]
        );
    }
    // done
    workers[0]->join();
    uint64_t latency_chases = finished_bytes[0];
    uint64_t total_bytes = latency_chases * config.stride_size_b;
    double latency_exec_time = exec_time[0];
    double total_exec_time = latency_exec_time;
    for (uint32_t i = 1; i < config.num_threads; ++i) {
        workers[i]->join();
        total_bytes += finished_bytes[i];
        total_exec_time += exec_time[i];
    }
    double latency = latency_exec_time * 1e9 / latency_chases;
    double mem_bw = total_bytes / total_exec_time * config.num_threads;
    std::cout << std::setw(12) << delay;
    std::cout << std::setw(12) << std::setprecision(6) << mem_bw / 1024 / 1024;
    std::cout << std::setw(12) << std::setprecision(4) << latency << std::endl;
}


int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::LATENCY_BANDWIDTH);
    if (config.parse_options(argc, argv)) {
        return 1;
    }
    config.dump();
    // setup memory regions
    std::vector<mm_utils::MemRegion::Handle> regions(config.num_threads, nullptr);
    // latency thread
    regions[0] = std::make_shared<mm_utils::MemRegion>(
        config.region_size_kb * 1024,
        config.chunk_size_kb * 4096,
        config.stride_size_b,
        static_cast<mm_utils::HugePageType>(config.use_hugepage)
    );
    if (config.access_pattern == 0) {
        regions[0]->stride_init();
    } else if (config.access_pattern == 1) {
        regions[0]->page_random_init();
    } else if (config.access_pattern == 2) {
        regions[0]->all_random_init();
    } else {
        return 1;
    }
    // bandwidth thread
    for (uint32_t i = 1; i < config.num_threads; ++i) {
        regions[i] = std::make_shared<mm_utils::MemRegion>(
            config.region_size_kb * 1024, 4096, 64
        );
    }
    // setup workers
    std::vector<std::shared_ptr<std::thread>> workers(config.num_threads, nullptr);
    std::vector<std::tuple<uint32_t, mm_worker::kernel_function>> delays_and_kernels;
    mm_worker::get_kernels_with_delays(delays_and_kernels, config.read_write_mix);
    std::cout << std::setw(12) << "delay";
    std::cout << std::setw(12) << "bandwidth";
    std::cout << std::setw(12) << "latency" << std::endl;
    for (auto& item : delays_and_kernels) {
        uint32_t delay = std::get<0>(item);
        mm_worker::kernel_function& kernel = std::get<1>(item);
        measure_loaded_latency(config, regions, workers, delay, kernel);
    }
    return 0;
}
