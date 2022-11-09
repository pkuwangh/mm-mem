#ifndef __WORKER_BANDWIDTH_H__
#define __WORKER_BANDWIDTH_H__

#include <iostream>
#include <sstream>
#include <string>

#include "common/timing.h"
#include "cpu_micro/worker_thread_packet.h"

namespace mm_worker {

void* mem_region_alloc_bw(void* ptr) {
    MemLatBwThreadPacket* pkt = static_cast<MemLatBwThreadPacket*>(ptr);
    pkt->mem_region = std::make_shared<mm_utils::MemRegion>(
        pkt->region_size_kb * 1024,
        4096,
        64,
        static_cast<mm_utils::MemType>(pkt->mem_type)
    );
    return nullptr;
}


void* bw_sequential(void* ptr) {
    MemLatBwThreadPacket* pkt = static_cast<MemLatBwThreadPacket*>(ptr);
    // setup checkpoint
    int64_t target_duration_ns = static_cast<int64_t>(pkt->target_duration) * 1000000000;
    uint64_t per_core_bw_mbps = 0;
    if (pkt->ref_total_bw_gbps > 0) {
        // actual per-core BW that varies with delays
        per_core_bw_mbps = pkt->ref_total_bw_gbps * 1024 / pkt->getNumThreads();
    } else {
        // assume ~2GB/s-per-core provisioned BW
        per_core_bw_mbps = 2 * 1024 * pkt->num_total_threads / pkt->getNumThreads();
    }
    // to get 4ms checkpoint
    const uint64_t chkpt_bytes = per_core_bw_mbps * 1024 * 1024 * 4 / 1000;
    const uint64_t loop_bytes = 256 * 32;
    const uint64_t loop_count = pkt->mem_region->activeSize() / loop_bytes;
    mm_utils::Timer timer_exec;
    // calculate BW
    const float write_fraction = mm_worker::get_write_fraction(pkt->read_write_mix);
    // run
    uint64_t* const start = (uint64_t*)(pkt->mem_region->getStartPoint());
    uint64_t* p = nullptr;
    uint64_t i = 0;
    uint64_t ret = 0;
    uint32_t num_chkpts = 0;
    uint64_t next_chkpt_bytes = chkpt_bytes;
    pkt->finished_bytes = 0;
    timer_exec.startTimer();
    while (true) {
        p = start;
        for (i = 0; i < loop_count; ++i) {
            pkt->kernel_bw(ret, p);
            pkt->finished_bytes += loop_bytes;
            if (pkt->finished_bytes > next_chkpt_bytes) {
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
    pkt->exec_time = timer_exec.getElapsedTime();
    pkt->finished_bytes += pkt->finished_bytes * write_fraction;
    if (pkt->ref_total_bw_gbps > 0 &&
        timer_exec.getElapsedTime() > pkt->target_duration * TIMER_THRESHOLD) {
        std::stringstream ss;
        ss << "elapsed time (s) exec=" << timer_exec.getElapsedTime()
           << " target=" << pkt->target_duration
           << " thread_id=" << pkt->getThreadId()
           << " bandwidth num_chkpts=" << num_chkpts
           << " finished_bytes=" << pkt->finished_bytes
           << " ret=" << ret << "\n";
        std::cerr << ss.str();
    }
    return nullptr;
}

}

#endif
