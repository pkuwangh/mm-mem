#ifndef __WORKER_LATENCY_H__
#define __WORKER_LATENCY_H__

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include "common/timing.h"
#include "cpu_micro/worker_thread_packet.h"

namespace mm_worker {

void* copy_fragment(void* ptr) {
    MemLatBwThreadPacket* pkt = static_cast<MemLatBwThreadPacket*>(ptr);
    // setup checkpoint
    int64_t target_duration_ns = static_cast<int64_t>(pkt->target_duration) * 1000000000;
    uint64_t chkpt_calls = 1;
    if (pkt->ref_one_exec_time_ns > 0) {
        chkpt_calls = std::max(5 * 1000000 / pkt->ref_one_exec_time_ns, (uint64_t)1);
    }
    const uint64_t loop_count = pkt->mem_region->activeSize() / pkt->fragment_size;
    mm_utils::Timer timer_exec;
    // run
    void* const src_buffer = (void*)(pkt->src_mem_region->getStartPoint());
    void* const dst_buffer = (void*)(pkt->mem_region->getStartPoint());
    uint64_t i = 0;
    uint32_t num_chkpts = 0;
    uint64_t next_chkpt_calls = chkpt_calls;
    uint64_t finished_calls = 0;
    timer_exec.startTimer();
    while (true) {
        for (i = 0; i < loop_count; ++i) {
            void* dst = (void*)((uint64_t)dst_buffer + pkt->fragment_size * i);
            void* src = (void*)((uint64_t)src_buffer + pkt->fragment_size * i);
            pkt->kernel_memcpy(dst, src, pkt->fragment_size);
            finished_calls += 1;
            if (finished_calls > next_chkpt_calls) {
                timer_exec.endTimer();
                next_chkpt_calls += chkpt_calls;
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
    pkt->finished_bytes = finished_calls * pkt->fragment_size;
    pkt->exec_time = timer_exec.getElapsedTime();
    if (pkt->ref_one_exec_time_ns > 0 &&
        ((chkpt_calls > 1 &&
          timer_exec.getElapsedTime() > pkt->target_duration * TIMER_THRESHOLD) ||
         (timer_exec.getElapsedTime() > pkt->target_duration * 1.05)
        )
    ) {
        std::stringstream ss;
        ss << "elapsed time (s) exec=" << timer_exec.getElapsedTime()
           << " target=" << pkt->target_duration
           << " thread_id=" << pkt->getThreadId()
           << " memcpy num_chkpts=" << num_chkpts
           << " finished_bytes=" << pkt->finished_bytes << "\n";
        std::cout << ss.str();
    }
    return nullptr;
}

}

#endif
