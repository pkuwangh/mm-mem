#ifndef __COMMON_NUMA_CONFIG_H__
#define __COMMON_NUMA_CONFIG_H__

#include <cstdint>
#include <vector>
#include <map>
#include <memory>

#if __linux__
#include <numa.h>       // numa_*
#endif

namespace mm_utils {

class NumaConfig {
  public:
    NumaConfig();
    ~NumaConfig();

    NumaConfig(const NumaConfig&) = delete;
    NumaConfig& operator= (const NumaConfig&) = delete;
    NumaConfig(NumaConfig&&) = delete;
    NumaConfig& operator= (NumaConfig&&) = delete;

    void dump() const;

    uint32_t num_numa_nodes = 0;
    uint32_t num_numa_nodes_configured = 0;
    uint32_t num_numa_nodes_possible = 0;
    uint32_t num_cpus = 0;
    uint32_t num_cpus_possible = 0;

    std::vector<uint32_t> all_cpus;
    std::vector<uint32_t> all_allowed_cpus;
    std::map<uint32_t, int64_t> node_to_mem;
    std::map<uint32_t, std::vector<uint32_t>> node_to_cpus;

  private:
    struct bitmask* cpumask_ = nullptr;
};

}

#endif
