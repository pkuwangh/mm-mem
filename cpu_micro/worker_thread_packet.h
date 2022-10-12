#ifndef __WORKER_THREAD_PACKET_H__
#define __WORKER_THREAD_PACKET_H__

#include "common/mem_region.h"
#include "common/worker_thread_manager.h"
#include "cpu_micro/worker_common.h"

namespace mm_worker {

class CpuMicroThreadPacket : public mm_utils::BaseThreadPacket {
  public:
    // common
    uint32_t target_duration = 0;
    // output
    double exec_time = 0;
};


class MemLatBwThreadPacket : public CpuMicroThreadPacket {
  public:
    mm_utils::MemRegion::Handle mem_region = nullptr;
    // latency thread
    func_kernel_lat kernel_lat;
    uint32_t ref_latency_ps = 0;
    // bandwidth thread
    func_kernel_bw kernel_bw;
    uint32_t read_write_mix = 0;
    uint32_t ref_total_bw_gbps = 0;
    uint32_t num_total_threads = 0;
    // memcpy thread
    mm_utils::MemRegion::Handle src_mem_region = nullptr;
    func_kernel_memcpy kernel_memcpy;
    uint64_t fragment_size = 0;
    uint64_t ref_one_exec_time_ns = 0;
    // output
    uint64_t finished_chases = 0;
    uint64_t finished_bytes = 0;
};


class BrPredThreadPacket : public CpuMicroThreadPacket {
  public:
    // output
    uint64_t finished_branches = 0;
};

}

#endif
