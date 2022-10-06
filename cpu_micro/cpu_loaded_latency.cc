#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <cassert>
#include <cstdint>
#include <thread>
#include <vector>

#include "common/mem_region.h"
#include "common/timing.h"
#include "cpu_micro/lib_configuration.h"
#include "cpu_micro/worker_bandwidth.h"
#include "cpu_micro/worker_kernels_delay_bandwidth.h"
#include "cpu_micro/worker_kernels_latency.h"
#include "cpu_micro/worker_latency.h"

std::tuple<uint32_t, uint32_t> measure_loaded_latency(
    mm_utils::Configuration& config,
    std::vector<mm_utils::MemRegion::Handle>& regions,
    std::vector<std::shared_ptr<std::thread>>& workers,
    uint32_t last_measured_lat_ps,
    uint32_t last_measured_bw_gbps,
    uint32_t delay,
    mm_worker::func_kernel_bw& kernel
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
        (last_measured_lat_ps > 0) ? config.target_duration_s : 1,
        last_measured_lat_ps,
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
            last_measured_bw_gbps,
            config.num_total_threads,
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
    double mem_bw_gbps = mem_bw / 1024 / 1024 / 1024;
    if (last_measured_lat_ps > 0 || last_measured_bw_gbps > 0) {
        std::cout << std::setw(12) << delay;
        std::cout << std::setw(12) << std::fixed << std::setprecision(1) << mem_bw_gbps;
        std::cout << std::setw(12) << std::fixed << std::setprecision(1) << latency;
        std::cout << std::endl;
    }
    return std::make_tuple(
        static_cast<uint32_t>(latency * 1e3), static_cast<uint32_t>(mem_bw_gbps));
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
    std::list<std::tuple<uint32_t, mm_worker::func_kernel_bw>> delays_and_kernels;
    mm_worker::get_kernels_with_delays(delays_and_kernels, config.read_write_mix);
    std::cout << std::setw(12) << "delay";
    std::cout << std::setw(12) << "bandwidth";
    std::cout << std::setw(12) << "latency" << std::endl;
    uint32_t last_lat_ps = 0;
    uint32_t last_bw_gbps = 0;
    delays_and_kernels.push_front(delays_and_kernels.front());
    for (auto& item : delays_and_kernels) {
        uint32_t delay = std::get<0>(item);
        mm_worker::func_kernel_bw& kernel = std::get<1>(item);
        std::tie(last_lat_ps, last_bw_gbps) = measure_loaded_latency(
            config, regions, workers, last_lat_ps, last_bw_gbps, delay, kernel);
    }
    std::cout << std::endl;
    return 0;
}
