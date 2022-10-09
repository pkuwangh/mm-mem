#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>

#include "common/mem_region.h"
#include "common/timing.h"
#include "cpu_micro/lib_configuration.h"
#include "cpu_micro/worker_memcpy.h"
#include "cpu_micro/kernels_memcpy.h"

uint64_t measure_mempcy_bandwidth(
    const mm_utils::Configuration& config,
    std::vector<mm_utils::MemRegion::Handle>& src_regions,
    std::vector<mm_utils::MemRegion::Handle>& dst_regions,
    std::vector<std::shared_ptr<std::thread>>& workers,
    uint64_t last_measured_exec_time_ns,
    const mm_worker::func_kernel_memcpy& kernel
) {
    std::vector<uint64_t> finished_bytes(config.num_threads, 0);
    std::vector<double> exec_time(config.num_threads, 0);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i].reset();
    }
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i] = std::make_shared<std::thread>(
            mm_worker::copy_fragment,
            kernel,
            src_regions[i],
            dst_regions[i],
            config.fragment_size_b,
            (last_measured_exec_time_ns > 0) ? config.target_duration_s : 1,
            last_measured_exec_time_ns,
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
    double copy_bw = total_bytes / total_exec_time * config.num_threads;
    double copy_bw_mbps = copy_bw / 1024 / 1024;
    double copy_bw_gbps = copy_bw_mbps / 1024;
    if (last_measured_exec_time_ns > 0) {
        std::cout << "Memcpy Bandwidth: ";
        std::cout << std::setprecision(7) << std::setw(10) << copy_bw_mbps << " MB/s | ";
        std::cout << std::setprecision(4) << std::setw(7) << copy_bw_gbps << " GB/s";
        std::cout << std::endl;
    }
    return static_cast<uint64_t>(
        config.fragment_size_b * config.num_threads / copy_bw_gbps);
}


int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::MEMCPY);
    if (config.parse_options(argc, argv)) {
        return 1;
    }
    if (config.fragment_size_b == 0) {
        config.fragment_size_b = config.region_size_kb * 1024;
    }
    config.dump();
    // setup memory regions & workers
    std::vector<mm_utils::MemRegion::Handle> src_regions(config.num_threads, nullptr);
    std::vector<mm_utils::MemRegion::Handle> dst_regions(config.num_threads, nullptr);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        src_regions[i] = std::make_shared<mm_utils::MemRegion>(
            config.region_size_kb * 1024, 4096, 64
        );
        dst_regions[i] = std::make_shared<mm_utils::MemRegion>(
            config.region_size_kb * 1024, 4096, 64
        );
    }
    std::vector<std::shared_ptr<std::thread>> workers(config.num_threads, nullptr);
    uint64_t last_exec_time_ns = 0;
    last_exec_time_ns = measure_mempcy_bandwidth(
        config, src_regions, dst_regions, workers,
        last_exec_time_ns, std::move(mm_worker::glibc_memcpy));
    measure_mempcy_bandwidth(
        config, src_regions, dst_regions, workers,
        last_exec_time_ns, std::move(mm_worker::glibc_memcpy));
    std::cout << std::endl;
    return 0;
}
