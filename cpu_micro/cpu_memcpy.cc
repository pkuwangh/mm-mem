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

void setup_memory_regions_memcpy(
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>& worker_manager,
    const mm_utils::Configuration& config
) {
    mm_worker::prepare_mem_lat_bw_thread_packet(worker_manager, config);
    worker_manager.setRoutineAndRun(mm_worker::mem_region_alloc_memcpy);
}

uint64_t measure_mempcy_bandwidth(
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>& worker_manager,
    const mm_worker::func_kernel_memcpy& kernel,
    const mm_utils::Configuration& config,
    uint64_t last_measured_exec_time_ns
) {
    // init the packet passed into each worker
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        worker_manager.getPacket(i).kernel_memcpy = kernel;
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
        config.numa_config.all_allowed_cpus,
        !config.no_binding,
        config.verbose
    );
    // setup memory regions
    mm_utils::start_timer("setup");
    setup_memory_regions_memcpy(worker_manager, config);
    mm_utils::end_timer("setup", std::cout);
    // start the show
    uint64_t last_exec_time_ns = 0;
    last_exec_time_ns = measure_mempcy_bandwidth(
        worker_manager, std::move(mm_worker::glibc_memcpy),
        config, last_exec_time_ns);
    measure_mempcy_bandwidth(
        worker_manager, std::move(mm_worker::glibc_memcpy),
        config, last_exec_time_ns);
    std::cout << std::endl;
    return 0;
}
