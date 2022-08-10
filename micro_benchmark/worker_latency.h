#ifndef __WORKER_LATENCY_H__
#define __WORKER_LATENCY_H__

#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include "lib_mem_region.h"
#include "lib_timing.h"
#include "worker_common.h"

namespace mm_worker {

void lat_ptr(
    kernel_function kernel,
    mm_utils::MemRegion::Handle mem_region,
    uint32_t target_duration,
    uint64_t* finished_chases,
    double* exec_time
) {
    // setup checkpoint
    int64_t target_duration_ns = static_cast<int64_t>(target_duration) * 1000000000;
    const uint64_t loop_chases = 256;
    const uint64_t loop_bytes = loop_chases * mem_region->lineSize();
    const uint64_t loop_count = mem_region->activeSize() / loop_bytes;
    uint64_t chkpt_chases = (4 << 20);  // 4ms
    if (mem_region->activeSize() > 32768 * 1024) {
        chkpt_chases /= 128;    // ~128ns
    } else if (mem_region->activeSize() > 2048 * 1024) {
        chkpt_chases /= 32;     // ~32ns
    } else if (mem_region->activeSize() > 32 * 1024) {
        chkpt_chases /= 8;      // ~8ns
    } else {
        chkpt_chases /= 2;      // ~2ns
    }
    mm_utils::Timer timer_exec;
    // run
    uint64_t* const start = (uint64_t*)(mem_region->getStartPoint());
    uint64_t* p = nullptr;
    uint64_t i = 0;
    uint64_t ret = 0;
    uint32_t num_chkpts = 0;
    uint64_t next_chkpt_chases = chkpt_chases;
    *finished_chases = 0;
    timer_exec.startTimer();
    while (true) {
        p = start;
        for (i = 0; i < loop_count; ++i) {
            kernel(ret, p);
            *finished_chases += loop_chases;
            if (*finished_chases > next_chkpt_chases) {
                timer_exec.endTimer();
                next_chkpt_chases += chkpt_chases;
                num_chkpts += 1;
                if (timer_exec.getElapsedTimeNs() >= target_duration_ns) {
                    break;
                }
                timer_exec.resumeTimer();
            }
        }
        if (i != loop_count) {
            break;
        }
    }
    const float threshold = 1.01;
    if (timer_exec.getElapsedTime() > target_duration * threshold) {
        std::stringstream ss;
        ss << "elapsed time (s) exec=" << timer_exec.getElapsedTime()
           << " target=" << target_duration
           << " num_chkpts=" << num_chkpts
           << " ret=" << ret << "\n";
        std::cout << ss.str();
    }
    *exec_time = timer_exec.getElapsedTime();
}

}

#endif
