#ifndef __WORKER_BRANCH_THROUGHPUT_H__
#define __WORKER_BRANCH_THROUGHPUT_H__

#include "common/cycle_counter.h"
#include "common/timing.h"
#include "cpu_micro/kernels_branch.h"
#include "cpu_micro/worker_thread_packet.h"

namespace mm_worker {

void* branch_tp(void* ptr) {
    auto* pkt = static_cast<BrPredThreadPacket*>(ptr);
    const auto& kernel = branch_cases()[pkt->case_index];
    constexpr uint64_t batch_rounds = 65536;
    const uint64_t batch_branches = taken_branches(kernel.branches, batch_rounds);
    // Warm the instruction cache and branch predictor before measuring.
    kernel.run(batch_rounds);
    mm_utils::CycleCounter cycles;
    mm_utils::Timer timer;
    pkt->finished_branches = 0;
    const bool counting = cycles.start();
    timer.startTimer();
    do {
        kernel.run(batch_rounds);
        pkt->finished_branches += batch_branches;
        timer.endTimer();
        if (timer.getElapsedTime() >= pkt->sample_duration) break;
        timer.resumeTimer();
    } while (true);
    pkt->exec_time = timer.getElapsedTime();
    pkt->cycles = counting ? cycles.stop() : 0;
    pkt->counter_error = cycles.error();
    return nullptr;
}

} // namespace mm_worker
#endif
