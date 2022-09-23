#ifndef __WORKER_BRANCH_THROUGHPUT_H__
#define __WORKER_BRANCH_THROUGHPUT_H__

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include "common/timing.h"
#include "cpu_micro/worker_common.h"

namespace mm_worker {

void branch_tp(
    uint32_t target_duration,
    uint64_t* finished_branches,
    double* exec_time
) {
    // setup checkpoint
    int64_t target_duration_ns = static_cast<int64_t>(target_duration) * 1000000000;
    uint64_t dummy_incr = std::min(target_duration, (uint32_t)1);
    const uint64_t loop_branches = (uint64_t)64 << 20;
    mm_utils::Timer timer_exec;
    // run
    uint64_t i = 0;
    uint64_t ret = 0;
    uint32_t num_chkpts = 0;
    *finished_branches = 0;
    timer_exec.startTimer();
    while (true) {
        for (i = 0; i < loop_branches; i += dummy_incr) {
            ret += i;
        }
        *finished_branches += i;
        timer_exec.endTimer();
        num_chkpts += 1;
        if (timer_exec.getElapsedTimeNs() >= target_duration_ns) {
            break;
        }
        timer_exec.resumeTimer();
    }
    if (timer_exec.getElapsedTime() > target_duration * TIMER_THRESHOLD) {
        std::stringstream ss;
        ss << "elapsed time (s) exec=" << timer_exec.getElapsedTime()
           << " target=" << target_duration
           << " branch throughput num_chkpts=" << num_chkpts
           << " ret=" << ret << "\n";
        std::cout << ss.str();
    }
    *exec_time = timer_exec.getElapsedTime();
}

}

#endif
