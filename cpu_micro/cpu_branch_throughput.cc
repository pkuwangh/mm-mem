#include <cstdint>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>

#include "common/timing.h"
#include "cpu_micro/lib_configuration.h"
#include "cpu_micro/worker_branch_throughput.h"

int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::BRANCH_THROUGHPUT);
    if (config.parse_options(argc, argv)) {
        return 1;
    }
    config.dump();
    // setup workers
    std::vector<std::shared_ptr<std::thread>> workers(config.num_threads, nullptr);
    std::vector<uint64_t> finished_branches(config.num_threads, 0);
    std::vector<double> exec_time(config.num_threads, 0);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i] = std::make_shared<std::thread>(
            mm_worker::branch_tp,
            config.target_duration_s,
            &finished_branches[i],
            &exec_time[i]
        );
    }
    // done
    uint64_t total_branches = 0;
    double total_exec_time = 0;
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i]->join();
        total_branches += finished_branches[i];
        total_exec_time += exec_time[i];
    }
    double taken_br_tp = total_branches / total_exec_time / 1e9;
    std::cout << "Branch throughput: " << std::setprecision(4) << taken_br_tp;
    std::cout << " B/s | " << total_branches << " / ";
    std::cout << std::setprecision(4) << total_exec_time;
    std::cout << std::endl;
    return 0;
}
