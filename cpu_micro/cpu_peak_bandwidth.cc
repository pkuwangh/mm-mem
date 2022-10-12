#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "common/mem_region.h"
#include "common/timing.h"
#include "common/worker_thread_manager.h"
#include "cpu_micro/lib_configuration.h"
#include "cpu_micro/kernels_bandwidth.h"
#include "cpu_micro/worker_bandwidth.h"

void setup_memory_regions_peak_bandwidth(
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>& worker_manager,
    const mm_utils::Configuration& config
) {
    mm_worker::prepare_mem_lat_bw_thread_packet(worker_manager, config);
    worker_manager.setRoutineAndRun(mm_worker::mem_region_alloc_bw);
}

uint32_t measure_peak_bandwidth(
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>& worker_manager,
    mm_worker::func_kernel_bw& kernel,
    const mm_utils::Configuration& config,
    uint32_t read_write_mix,
    uint32_t last_measured_bw_gbps
) {
    // init the packet passed into each worker
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        worker_manager.getPacket(i).kernel_bw = kernel;
        worker_manager.getPacket(i).read_write_mix = read_write_mix;
        worker_manager.getPacket(i).ref_total_bw_gbps = last_measured_bw_gbps;
        worker_manager.getPacket(i).num_total_threads = config.num_total_threads;
        worker_manager.getPacket(i).target_duration =
            (last_measured_bw_gbps > 0) ? config.target_duration_s : 1;
    }
    // set routines
    worker_manager.setRoutine(mm_worker::bw_sequential);
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
    double mem_bw = total_bytes / total_exec_time * config.num_threads;
    double mem_bw_mbps = mem_bw / 1024 / 1024;
    double mem_bw_gbps = mem_bw_mbps / 1024;
    if (last_measured_bw_gbps > 0) {
        std::cout << std::setw(20) << config.get_str_rw_mix(read_write_mix) << " :";
        std::cout << std::setprecision(7) << std::setw(10) << mem_bw_mbps << " MB/s | ";
        std::cout << std::setprecision(4) << std::setw(7) << mem_bw_gbps << " GB/s";
        std::cout << std::endl;
    }
    return static_cast<uint32_t>(mem_bw_gbps);
}


int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::BANDWIDTH);
    if (config.parse_options(argc, argv)) {
        return 1;
    }
    config.dump();
    // setup workers
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket> worker_manager(
        config.num_threads,
        {},
        false
    );
    // get kernels
    mm_worker::rwmix_kernel_list rwmix_and_kernels;
    mm_worker::get_kernels_with_wrmix(rwmix_and_kernels, config.read_write_mix);
    rwmix_and_kernels.push_front(rwmix_and_kernels.front());
    // setup memory regions
    mm_utils::start_timer("setup");
    setup_memory_regions_peak_bandwidth(worker_manager, config);
    mm_utils::end_timer("setup", std::cout);
    // start the show
    uint32_t last_bw_gbps = 0;
    for (auto& item : rwmix_and_kernels) {
        uint32_t read_write_mix = std::get<0>(item);
        mm_worker::func_kernel_bw& kernel = std::get<1>(item);
        last_bw_gbps = measure_peak_bandwidth(
            worker_manager, kernel,
            config, read_write_mix, last_bw_gbps);
    }
    std::cout << std::endl;
    return 0;
}
