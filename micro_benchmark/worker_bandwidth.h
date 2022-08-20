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

void bw_sequential(
    kernel_function kernel,
    mm_utils::MemRegion::Handle mem_region,
    uint32_t read_write_mix,
    uint32_t target_duration,
    uint32_t ref_total_bw_gbps,
    uint32_t num_total_threads,
    uint32_t num_threads,
    uint64_t* finished_bytes,
    double* exec_time
) {
    // setup checkpoint
    int64_t target_duration_ns = static_cast<int64_t>(target_duration) * 1000000000;
    uint64_t per_core_bw_mbps = 0;
    if (ref_total_bw_gbps > 0) {
        // actual per-core BW that varies with delays
        per_core_bw_mbps = ref_total_bw_gbps * 1024 / num_threads;
    } else {
        // assume ~2GB/s-per-core provisioned BW
        per_core_bw_mbps = 2 * 1024 * num_total_threads / num_threads;
    }
    // to get 4ms checkpoint
    const uint64_t chkpt_bytes = per_core_bw_mbps * 1024 * 1024 * 4 / 1000;
    const uint64_t loop_bytes = 256 * 32;
    const uint64_t loop_count = mem_region->activeSize() / loop_bytes;
    mm_utils::Timer timer_exec;
    // calculate BW
    const float write_fraction = mm_worker::get_write_fraction(read_write_mix);
    // run
    uint64_t* const start = (uint64_t*)(mem_region->getStartPoint());
    uint64_t* p = nullptr;
    uint64_t i = 0;
    uint64_t ret = 0;
    uint32_t num_chkpts = 0;
    uint64_t next_chkpt_bytes = chkpt_bytes;
    *finished_bytes = 0;
    timer_exec.startTimer();
    while (true) {
        p = start;
        for (i = 0; i < loop_count; ++i) {
            kernel(ret, p);
            *finished_bytes += loop_bytes;
            if (*finished_bytes > next_chkpt_bytes) {
                timer_exec.endTimer();
                next_chkpt_bytes += chkpt_bytes;
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
    if (timer_exec.getElapsedTime() > target_duration * TIMER_THRESHOLD) {
        std::stringstream ss;
        ss << "elapsed time (s) exec=" << timer_exec.getElapsedTime()
           << " target=" << target_duration
           << " bandwidth num_chkpts=" << num_chkpts
           << " ret=" << ret << "\n";
        std::cout << ss.str();
    }
    *exec_time = timer_exec.getElapsedTime();
    *finished_bytes += *finished_bytes * write_fraction;
}


void bw_sequential_no_ref(
    kernel_function kernel,
    mm_utils::MemRegion::Handle mem_region,
    uint32_t read_write_mix,
    uint32_t target_duration,
    uint32_t num_total_threads,
    uint32_t num_threads,
    uint64_t* finished_bytes,
    double* exec_time
) {
    const uint32_t ref_total_bw_gbps = 0;
    bw_sequential(
        kernel,
        mem_region,
        read_write_mix,
        target_duration,
        ref_total_bw_gbps,
        num_total_threads,
        num_threads,
        finished_bytes,
        exec_time);
}

}

#endif
