#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include "common/mem_region.h"
#include "common/timing.h"
#include "common/worker_thread_manager.h"
#include "cpu_micro/lib_configuration.h"
#include "cpu_micro/kernels_memcpy.h"
#include "cpu_micro/worker_memcpy.h"

uint64_t measure_mempcy_bandwidth(
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>& worker_manager,
    const mm_worker::func_kernel_memcpy& kernel,
    std::vector<mm_utils::MemRegion::Handle>& src_regions,
    std::vector<mm_utils::MemRegion::Handle>& dst_regions,
    const mm_utils::Configuration& config,
    uint64_t last_measured_exec_time_ns
) {
    // init the packet passed into each worker
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        worker_manager.getPacket(i).mem_region = dst_regions[i];
        worker_manager.getPacket(i).kernel_memcpy = kernel;
        worker_manager.getPacket(i).src_mem_region = src_regions[i];
        worker_manager.getPacket(i).fragment_size = config.fragment_size_b;
        worker_manager.getPacket(i).ref_one_exec_time_ns = last_measured_exec_time_ns;
        worker_manager.getPacket(i).target_duration =
            (last_measured_exec_time_ns > 0) ? config.target_duration_s : 1;
    }
    // set routines
    worker_manager.setRoutine(mm_worker::copy_fragment);
    // start
    worker_manager.create();
    // done
    worker_manager.join();
    uint64_t total_bytes = 0;
    double total_exec_time = 0;
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        total_bytes += worker_manager.getPacket(i).finished_bytes;
        total_exec_time += worker_manager.getPacket(i).exec_time;
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
    // setup workers
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket> worker_manager(
        config.num_threads,
        {},
        false
    );
    // setup memory regions
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
    // start the show
    uint64_t last_exec_time_ns = 0;
    last_exec_time_ns = measure_mempcy_bandwidth(
        worker_manager, std::move(mm_worker::glibc_memcpy),
        src_regions, dst_regions, config, last_exec_time_ns);
    measure_mempcy_bandwidth(
        worker_manager, std::move(mm_worker::glibc_memcpy),
        src_regions, dst_regions, config, last_exec_time_ns);
    std::cout << std::endl;
    return 0;
}
