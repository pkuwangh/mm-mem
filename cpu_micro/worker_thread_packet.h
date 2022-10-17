#ifndef __WORKER_THREAD_PACKET_H__
#define __WORKER_THREAD_PACKET_H__

#include "common/mem_region.h"
#include "common/worker_thread_manager.h"
#include "cpu_micro/lib_configuration.h"
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
    ~MemLatBwThreadPacket() {
        mem_region.reset();
        src_mem_region.reset();
    }

    mm_utils::MemRegion::Handle mem_region = nullptr;
    // mem region config
    uint64_t region_size_kb = 0;
    uint32_t access_pattern = 0;
    uint32_t chunk_size_kb = 0;
    uint32_t stride_size_b = 0;
    uint32_t use_hugepage = 0;
    // numa config
    int32_t mem_type = 0;
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

  public:
    void copy_mem_region_config(const mm_utils::Configuration& config) {
        region_size_kb = config.region_size_kb;
        access_pattern = config.access_pattern;
        chunk_size_kb = config.chunk_size_kb;
        stride_size_b = config.stride_size_b;
        use_hugepage = config.use_hugepage;
    }
};


class BrPredThreadPacket : public CpuMicroThreadPacket {
  public:
    // output
    uint64_t finished_branches = 0;
};


using MemLatBwManager =
    mm_utils::WorkerThreadManager<mm_worker::MemLatBwThreadPacket>;

void prepare_mem_lat_bw_thread_packet(
    MemLatBwManager& worker_manager,
    const mm_utils::Configuration& config,
    int node = -1
) {
    // default mem_type is mm_utils::MemType::NATIVE
    int mem_type = static_cast<int>(mm_utils::MemType::NODE0) + node;
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        worker_manager.getPacket(i).copy_mem_region_config(config);
        worker_manager.getPacket(i).mem_type = mem_type;
    }
}

}

#endif
