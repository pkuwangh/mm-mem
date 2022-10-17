#ifndef __WORKER_LATENCY_H__
#define __WORKER_LATENCY_H__

#include <iostream>
#include <sstream>
#include <string>

#include "common/timing.h"
#include "cpu_micro/worker_thread_packet.h"

namespace mm_worker {

void* mem_region_alloc_lat(void* ptr) {
    MemLatBwThreadPacket* pkt = static_cast<MemLatBwThreadPacket*>(ptr);
    pkt->mem_region = std::make_shared<mm_utils::MemRegion>(
        pkt->region_size_kb * 1024,
        pkt->chunk_size_kb * 1024,
        pkt->stride_size_b,
        static_cast<mm_utils::MemType>(pkt->mem_type),
        static_cast<mm_utils::HugePageType>(pkt->use_hugepage)
    );
    if (pkt->access_pattern == 0) {
        pkt->mem_region->stride_init();
    } else if (pkt->access_pattern == 1) {
        pkt->mem_region->page_random_init();
    } else if (pkt->access_pattern == 2) {
        pkt->mem_region->all_random_init();
    } else {
        pkt->mem_region->page_random_init();
    }
    return nullptr;
}


void* lat_ptr(void* ptr) {
    MemLatBwThreadPacket* pkt = static_cast<MemLatBwThreadPacket*>(ptr);
    // setup checkpoint
    int64_t target_duration_ns = static_cast<int64_t>(pkt->target_duration) * 1000000000;
    const uint64_t loop_chases = 256;
    const uint64_t loop_bytes = loop_chases * pkt->mem_region->lineSize();
    const uint64_t loop_count = pkt->mem_region->activeSize() / loop_bytes;
    uint64_t chkpt_chases = (4 << 20);  // 4ms checkpoint if 1ns per chase
    if (pkt->ref_latency_ps > 0) {
        chkpt_chases = chkpt_chases / pkt->ref_latency_ps * 1000;
    } else {
        if (pkt->mem_region->activeSize() > 32768 * 1024) {
            chkpt_chases /= 128;    // ~128ns
        } else if (pkt->mem_region->activeSize() > 2048 * 1024) {
            chkpt_chases /= 32;     // ~32ns
        } else if (pkt->mem_region->activeSize() > 32 * 1024) {
            chkpt_chases /= 8;      // ~8ns
        } else {
            chkpt_chases /= 2;      // ~2ns
        }
    }
    mm_utils::Timer timer_exec;
    // run
    uint64_t* const start = (uint64_t*)(pkt->mem_region->getStartPoint());
    uint64_t* p = nullptr;
    uint64_t i = 0;
    uint64_t ret = 0;
    uint32_t num_chkpts = 0;
    uint64_t next_chkpt_chases = chkpt_chases;
    pkt->finished_chases = 0;
    timer_exec.startTimer();
    while (true) {
        p = start;
        for (i = 0; i < loop_count; ++i) {
            pkt->kernel_lat(ret, p);
            pkt->finished_chases += loop_chases;
            if (pkt->finished_chases > next_chkpt_chases) {
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
    pkt->exec_time = timer_exec.getElapsedTime();
    if (pkt->ref_latency_ps > 0 &&
        timer_exec.getElapsedTime() > pkt->target_duration * TIMER_THRESHOLD) {
        std::stringstream ss;
        ss << "elapsed time (s) exec=" << timer_exec.getElapsedTime()
           << " target=" << pkt->target_duration
           << " thread_id=" << pkt->getThreadId()
           << " latency num_chkpts=" << num_chkpts
           << " finished_chases=" << pkt->finished_chases
           << " ret=" << ret << "\n";
        std::cout << ss.str();
    }
    return nullptr;
}

}

#endif
