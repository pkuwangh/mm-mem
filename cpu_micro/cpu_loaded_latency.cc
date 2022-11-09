#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "common/mem_region.h"
#include "common/timing.h"
#include "common/worker_thread_manager.h"
#include "cpu_micro/lib_configuration.h"
#include "cpu_micro/kernels_delay_bandwidth.h"
#include "cpu_micro/kernels_latency.h"
#include "cpu_micro/worker_bandwidth.h"
#include "cpu_micro/worker_latency.h"

void setup_memory_regions_loaded_latency(
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>& worker_manager,
    const mm_utils::Configuration& config
) {
    mm_worker::prepare_mem_lat_bw_thread_packet(worker_manager, config);
    worker_manager.setRoutine(
        mm_worker::mem_region_alloc_lat,
        [](const uint32_t& idx) { return idx == 0; });
    worker_manager.setRoutine(
        mm_worker::mem_region_alloc_bw,
        [](const uint32_t& idx) { return idx > 0; });
    worker_manager.run();
}

std::tuple<uint32_t, uint32_t> measure_loaded_latency(
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>& worker_manager,
    mm_worker::func_kernel_bw& kernel_bw,
    const mm_utils::Configuration& config,
    uint32_t last_measured_lat_ps,
    uint32_t last_measured_bw_gbps,
    uint32_t delay
) {
    // init the packet passed into each worker
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        worker_manager.getPacket(i).kernel_lat = mm_worker::kernel_lat;
        worker_manager.getPacket(i).ref_latency_ps = last_measured_lat_ps;
        worker_manager.getPacket(i).kernel_bw = kernel_bw;
        worker_manager.getPacket(i).read_write_mix = config.read_write_mix;
        worker_manager.getPacket(i).ref_total_bw_gbps = last_measured_bw_gbps;
        worker_manager.getPacket(i).num_total_threads = config.numa_config.num_cpus;
        if (i == 0) {
            worker_manager.getPacket(i).target_duration =
                (last_measured_lat_ps > 0) ? config.target_duration_s : 1;
        } else {
            worker_manager.getPacket(i).target_duration =
                (last_measured_bw_gbps > 0) ? config.target_duration_s : 1;
        }
    }
    // set routines
    worker_manager.setRoutine(
        mm_worker::lat_ptr, [](const uint32_t& idx) { return idx == 0; });
    worker_manager.setRoutine(
        mm_worker::bw_sequential, [](const uint32_t& idx) { return idx > 0; });
    // start
    worker_manager.create();
    // done
    worker_manager.join();
    uint64_t latency_chases = worker_manager.getPacket(0).finished_chases;
    uint64_t total_bytes = latency_chases * config.stride_size_b;
    double latency_exec_time = worker_manager.getPacket(0).exec_time;
    double total_exec_time = latency_exec_time;
    for (uint32_t i = 1; i < config.num_threads; ++i) {
        total_bytes += worker_manager.getPacket(i).finished_bytes;
        total_exec_time += worker_manager.getPacket(i).exec_time;
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


void build_delays_and_kernels(
    mm_worker::delay_kernel_list& delays_and_kernels,
    const mm_utils::Configuration& config
) {
    mm_worker::delay_kernel_list delays_and_kernels_raw;
    mm_worker::get_kernels_with_delays(delays_and_kernels_raw, config.read_write_mix);
    if (config.load_gen_delay > 0) {
        int32_t gap = config.load_gen_delay;
        for (auto& item : delays_and_kernels_raw) {
            int32_t curr_gap = abs(std::get<0>(item) - config.load_gen_delay);
            if (curr_gap < gap) {
                gap = curr_gap;
                delays_and_kernels.clear();
                delays_and_kernels.push_back(item);
            }
        }
    } else {
        delays_and_kernels = delays_and_kernels_raw;
    }
    delays_and_kernels.push_front(delays_and_kernels.front());
}


int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::LATENCY_BANDWIDTH);
    if (config.parse_options(argc, argv)) {
        return 1;
    }
    config.dump();
    // setup workers
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket> worker_manager(
        config.num_threads,
        config.numa_config.all_allowed_cpus,
        !config.no_binding,
        config.verbose
    );
    // get kernels
    mm_worker::delay_kernel_list delays_and_kernels;
    build_delays_and_kernels(delays_and_kernels, config);
    // setup memory regions
    mm_utils::start_timer("setup");
    setup_memory_regions_loaded_latency(worker_manager, config);
    mm_utils::end_timer("setup", std::cout);
    // start the show
    std::cout << std::setw(12) << "delay";
    std::cout << std::setw(12) << "bandwidth";
    std::cout << std::setw(12) << "latency" << " - ";
    std::cout << config.get_str_access_pattern_short(config.access_pattern) << std::endl;
    uint32_t last_lat_ps = 0;
    uint32_t last_bw_gbps = 0;
    for (auto& item : delays_and_kernels) {
        uint32_t delay = std::get<0>(item);
        mm_worker::func_kernel_bw& kernel = std::get<1>(item);
        std::tie(last_lat_ps, last_bw_gbps) = measure_loaded_latency(
            worker_manager, kernel, config, last_lat_ps, last_bw_gbps, delay);
    }
    std::cout << std::endl;
    return 0;
}
