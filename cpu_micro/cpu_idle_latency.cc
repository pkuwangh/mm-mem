#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "common/mem_region.h"
#include "common/timing.h"
#include "common/worker_thread_manager.h"
#include "cpu_micro/lib_configuration.h"
#include "cpu_micro/kernels_latency.h"
#include "cpu_micro/worker_latency.h"

void setup_memory_regions_idle_latency(
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>& worker_manager,
    const mm_utils::Configuration& config
) {
    mm_worker::prepare_mem_lat_bw_thread_packet(worker_manager, config);
    worker_manager.setRoutineAndRun(mm_worker::mem_region_alloc_lat);
}

uint32_t measure_idle_latency(
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>& worker_manager,
    const mm_utils::Configuration& config,
    uint32_t last_measured_lat_ps
) {
    // init the packet passed into each worker
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        worker_manager.getPacket(i).kernel_lat = mm_worker::kernel_lat;
        worker_manager.getPacket(i).ref_latency_ps = last_measured_lat_ps;
        worker_manager.getPacket(i).target_duration =
            (last_measured_lat_ps > 0) ? config.target_duration_s : 1;
    }
    // set routines
    worker_manager.setRoutine(mm_worker::lat_ptr);
    // start
    worker_manager.create();
    // done
    worker_manager.join();
    uint64_t total_chases = 0;
    double total_exec_time = 0;
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        total_chases += worker_manager.getPacket(i).finished_chases;
        total_exec_time += worker_manager.getPacket(i).exec_time;
    }
    double latency = total_exec_time * 1e9 / total_chases;
    if (last_measured_lat_ps > 0) {
        std::cout << "Idle Latency: " << std::setprecision(4) << latency << " ns";
        std::cout << std::endl;
    }
    return static_cast<uint32_t>(latency * 1e3);
}


int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::LATENCY);
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
    // setup memory regions
    setup_memory_regions_idle_latency(worker_manager, config);
    // start the show
    uint32_t last_lat_ps = 0;
    last_lat_ps = measure_idle_latency(worker_manager, config, last_lat_ps);
    measure_idle_latency(worker_manager, config, last_lat_ps);
    std::cout << std::endl;
    return 0;
}
