#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include "common/worker_thread_manager.h"
#include "cpu_micro/lib_configuration.h"
#include "cpu_micro/worker_branch_throughput.h"

int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::BRANCH_THROUGHPUT);
    if (config.parse_options(argc, argv)) return 1;
    if (!config.num_threads || !config.target_duration_s ||
        config.branch_case > mm_worker::branch_case_count) {
        std::cerr << "Threads and duration must be positive; branch_case must be 0-12.\n";
        return 1;
    }
    config.dump();
    const unsigned first = config.branch_case ? config.branch_case - 1 : 0;
    const unsigned end = config.branch_case ? config.branch_case : mm_worker::branch_case_count;
    const double duration = static_cast<double>(config.target_duration_s) / (end - first);
    mm_utils::WorkerThreadManager<mm_worker::BrPredThreadPacket> workers(
        config.num_threads, config.numa_config.all_allowed_cpus,
        !config.no_binding, config.verbose);
    workers.setRoutine(mm_worker::branch_tp);
    std::vector<double> peak_rate(config.num_threads, 0);
    std::vector<double> peak_per_cycle(config.num_threads, 0);
    bool counter_warning = false;
    std::cout << "Predictable taken branches; duration split across cases; rates per worker.\n"
              << "case thread type          chain spacing   GBr/s   Br/cycle cycles/Br\n";
    for (unsigned index = first; index < end; ++index) {
        const auto& kernel = mm_worker::branch_cases()[index];
        for (unsigned tid = 0; tid < config.num_threads; ++tid) {
            auto& pkt = workers.getPacket(tid);
            pkt.case_index = index;
            pkt.sample_duration = duration;
        }
        workers.run();
        for (unsigned tid = 0; tid < config.num_threads; ++tid) {
            const auto& pkt = workers.getPacket(tid);
            const double rate = pkt.finished_branches / pkt.exec_time / 1e9;
            peak_rate[tid] = std::max(peak_rate[tid], rate);
            std::cout << std::setw(4) << index + 1 << std::setw(7) << tid << " "
                      << std::setw(13) << kernel.type << std::setw(6) << kernel.branches
                      << std::setw(8) << kernel.spacing << std::fixed << std::setprecision(3)
                      << std::setw(8) << rate;
            if (pkt.cycles > 0) {
                const double per_cycle = pkt.finished_branches / pkt.cycles;
                peak_per_cycle[tid] = std::max(peak_per_cycle[tid], per_cycle);
                std::cout << std::setw(11) << per_cycle << std::setw(10) << 1 / per_cycle;
            } else {
                std::cout << "        n/a       n/a";
                if (!counter_warning) {
                    std::cerr << "Cycles unavailable: " << pkt.counter_error << ".\n";
                    counter_warning = true;
                }
            }
            std::cout << std::endl;
        }
    }
    for (unsigned tid = 0; tid < config.num_threads; ++tid) {
        std::cout << "Best observed thread " << tid << ": " << peak_rate[tid] << " GBr/s";
        if (peak_per_cycle[tid] > 0) {
            std::cout << "; " << peak_per_cycle[tid] << " Br/cycle";
        }
        std::cout << " (independent maxima across tested cases)\n";
    }
    return 0;
}
