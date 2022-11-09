#ifndef __WORKER_BRANCH_THROUGHPUT_H__
#define __WORKER_BRANCH_THROUGHPUT_H__

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include "common/timing.h"
#include "cpu_micro/worker_thread_packet.h"

namespace mm_worker {

void* branch_tp(void* ptr) {
    BrPredThreadPacket* pkt = static_cast<BrPredThreadPacket*>(ptr);
    // setup checkpoint
    int64_t target_duration_ns = static_cast<int64_t>(pkt->target_duration) * 1000000000;
    uint64_t dummy_incr = std::min(pkt->target_duration, (uint32_t)1);
    const uint64_t loop_branches = (uint64_t)64 << 20;
    mm_utils::Timer timer_exec;
    // run
    uint64_t i = 0;
    uint64_t ret = 0;
    uint32_t num_chkpts = 0;
    pkt->finished_branches = 0;
    timer_exec.startTimer();
    while (true) {
        for (i = 0; i < loop_branches; i += dummy_incr) {
            ret += i;
        }
        pkt->finished_branches += i;
        timer_exec.endTimer();
        num_chkpts += 1;
        if (timer_exec.getElapsedTimeNs() >= target_duration_ns) {
            break;
        }
        timer_exec.resumeTimer();
    }
    pkt->exec_time = timer_exec.getElapsedTime();
    if (timer_exec.getElapsedTime() > pkt->target_duration * TIMER_THRESHOLD) {
        std::stringstream ss;
        ss << "elapsed time (s) exec=" << timer_exec.getElapsedTime()
           << " target=" << pkt->target_duration
           << " branch throughput num_chkpts=" << num_chkpts
           << " finished_branches=" << pkt->finished_branches
           << " ret=" << ret << "\n";
        std::cerr << ss.str();
    }
    return nullptr;
}

}

#endif
