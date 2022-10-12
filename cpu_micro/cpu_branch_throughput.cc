#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include "common/timing.h"
#include "common/worker_thread_manager.h"
#include "cpu_micro/lib_configuration.h"
#include "cpu_micro/worker_branch_throughput.h"

int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::BRANCH_THROUGHPUT);
    if (config.parse_options(argc, argv)) {
        return 1;
    }
    config.dump();
    // setup workers
    mm_utils::WorkerThreadManager<mm_worker::BrPredThreadPacket> worker_manager(
        config.num_threads,
        {},
        false
    );
    // start the show
    // init the packet passed into each worker
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        worker_manager.getPacket(i).target_duration = config.target_duration_s;
    }
    // set routines
    worker_manager.setRoutine(mm_worker::branch_tp);
    // start
    worker_manager.create();
    // done
    worker_manager.join();
    uint64_t total_branches = 0;
    double total_exec_time = 0;
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        total_branches += worker_manager.getPacket(i).finished_branches;
        total_exec_time += worker_manager.getPacket(i).exec_time;
    }
    double taken_br_tp = total_branches / total_exec_time / 1e9;
    std::cout << "Branch throughput: " << std::setprecision(4) << taken_br_tp;
    std::cout << " B/s | " << total_branches << " / ";
    std::cout << std::setprecision(4) << total_exec_time;
    std::cout << std::endl;
    return 0;
}
