#include <algorithm>
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
    mm_worker::MemLatBwManager& worker_manager,
    const mm_utils::Configuration& config,
    int node = -1
) {
    mm_worker::prepare_mem_lat_bw_thread_packet(worker_manager, config, node);
    worker_manager.setRoutineAndRun(mm_worker::mem_region_alloc_lat);
}

uint32_t measure_idle_latency(
    mm_worker::MemLatBwManager& worker_manager,
    const mm_utils::Configuration& config,
    uint32_t last_measured_lat_ps
) {
    // init the packet passed into each worker
    for (uint32_t i = 0; i < worker_manager.getNumThreads(); ++i) {
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
    for (uint32_t i = 0; i < worker_manager.getNumThreads(); ++i) {
        total_chases += worker_manager.getPacket(i).finished_chases;
        total_exec_time += worker_manager.getPacket(i).exec_time;
    }
    double latency = total_exec_time * 1e9 / total_chases;
    if (last_measured_lat_ps > 0) {
        std::cout << std::setw(10) << std::setprecision(4) << latency << std::flush;
    }
    return static_cast<uint32_t>(latency * 1e3);
}

void run(
    mm_worker::MemLatBwManager& worker_manager,
    const mm_utils::Configuration& config
) {
    uint32_t last_lat_ps = 0;
    last_lat_ps = measure_idle_latency(worker_manager, config, last_lat_ps);
    measure_idle_latency(worker_manager, config, last_lat_ps);
}

void setup_and_run(const mm_utils::Configuration& config) {
    std::shared_ptr<mm_worker::MemLatBwManager> worker_manager;
    if (config.latency_matrix) {
        std::cout << std::left << std::setw(25) << "Idle Latency (ns)";
        for (uint32_t j = 0; j < config.numa_config.num_numa_nodes; ++j) {
            std::cout << std::setw(10) << "Node-" + std::to_string(j);
        }
        for (uint32_t i = 0; i < config.numa_config.num_numa_nodes; ++i) {
            if (config.numa_config.node_to_cpus.at(i).size() == 0) {
                continue;
            }
            std::cout << std::endl << std::setw(25) << "Node-" + std::to_string(i);
            std::cout << std::flush;
            for (uint32_t j = 0; j < config.numa_config.num_numa_nodes; ++j) {
                if (config.numa_config.node_to_mem.at(j) < ((int64_t)1 << 30)) {
                    continue;
                }
                worker_manager.reset();
                uint32_t node_cpu_count = config.numa_config.node_to_cpus.at(i).size();
                worker_manager = std::make_shared<mm_worker::MemLatBwManager>(
                    std::min(config.num_threads, node_cpu_count),
                    config.numa_config.node_to_cpus.at(i),
                    true,   // always enable binding
                    config.verbose
                );
                setup_memory_regions_idle_latency(*worker_manager, config, j);
                run(*worker_manager, config);
            }
        }
        std::cout << std::endl;
    } else {
        // setup workers
        worker_manager = std::make_shared<mm_worker::MemLatBwManager>(
            config.num_threads,
            config.numa_config.all_allowed_cpus,
            !config.no_binding,
            config.verbose
        );
        // setup memory regions
        setup_memory_regions_idle_latency(*worker_manager, config);
        // start the show
        std::cout << "Idle Latency: ";
        run(*worker_manager, config);
        std::cout << " ns" << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::LATENCY);
    if (config.parse_options(argc, argv)) {
        return 1;
    }
    config.dump();
    setup_and_run(config);
    return 0;
}
