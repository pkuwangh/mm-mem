#ifndef __WORKER_BANDWIDTH_H__
#define __WORKER_BANDWIDTH_H__

#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include "lib_mem_region.h"
#include "lib_timing.h"
#include "worker_common.h"


namespace mm_worker {

void kernel_bw_r1w0_sequential(uint64_t& ret, uint64_t*& p) {
    LOOP256(ret += *p; p += 4;)
}

void kernel_bw_r1w1_sequential(uint64_t& ret, uint64_t*& p) {
    LOOP128(ret += *p; p += 4; *p = ret; p += 4;)
}

void kernel_bw_r2w1_sequential(uint64_t& ret, uint64_t*& p) {
    LOOP64(ret += *p; p += 4; ret += *p; p += 4; ret += *p; p += 4; *p = ret; p += 4;)
}


void bw_sequential(
    const mm_utils::MemRegion::Handle mem_region,
    uint32_t read_write_mix,
    uint32_t target_duration,
    uint32_t num_total_threads,
    uint64_t* finished_bytes,
    double* exec_time
) {
    // setup checkpoint
    int64_t target_duration_ns = static_cast<int64_t>(target_duration) * 1000000000;
    const uint64_t chkpt_bytes = 256 * 1024 * 1024 / num_total_threads;
    const uint64_t loop_bytes = 256 * 32;
    const uint64_t loop_count = mem_region->activeSize() / loop_bytes;
    mm_utils::Timer timer_exec;
    // the action
    std::function<void(uint64_t&, uint64_t*&)> kernel;
    float write_fraction = 0;
    if (read_write_mix == 0) {
        kernel = kernel_bw_r1w0_sequential;
    } else if (read_write_mix == 1) {
        kernel = kernel_bw_r1w1_sequential;
        write_fraction = 1;
    } else if (read_write_mix == 2) {
        kernel = kernel_bw_r2w1_sequential;
        write_fraction = 0.5;
    } else {
      return;
    }
    // run
    uint64_t* const start = (uint64_t*)(mem_region->getStartPoint());
    uint64_t* p = nullptr;
    uint64_t i = 0;
    uint64_t ret = 0;
    uint32_t num_ckpts = 0;
    uint64_t next_ckpt_bytes = chkpt_bytes;
    *finished_bytes = 0;
    timer_exec.startTimer();
    while (true) {
        p = start;
        for (i = 0; i < loop_count; ++i) {
            kernel(ret, p);
            *finished_bytes += loop_bytes;
            if (*finished_bytes > next_ckpt_bytes) {
                timer_exec.endTimer();
                next_ckpt_bytes += chkpt_bytes;
                num_ckpts += 1;
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
           << " num_ckpts=" << num_ckpts
           << " ret=" << ret << "\n";
        std::cout << ss.str();
    }
    *exec_time = timer_exec.getElapsedTime();
    *finished_bytes += *finished_bytes * write_fraction;
}

}

#endif
