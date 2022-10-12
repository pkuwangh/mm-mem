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

uint32_t measure_idle_latency(
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>& worker_manager,
    std::vector<mm_utils::MemRegion::Handle>& regions,
    const mm_utils::Configuration& config,
    uint32_t last_measured_lat_ps
) {
    // init the packet passed into each worker
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        worker_manager.getPacket(i).mem_region = regions[i];
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
        {},
        false
    );
    // setup memory regions
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
    // start the show
    uint32_t last_lat_ps = 0;
    last_lat_ps = measure_idle_latency(worker_manager, regions, config, last_lat_ps);
    measure_idle_latency(worker_manager, regions, config, last_lat_ps);
    std::cout << std::endl;
    return 0;
}
