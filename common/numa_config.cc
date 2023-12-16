#include <iostream>
#include <thread>

#include "common/kmg_parser.h"
#include "common/numa_config.h"

namespace mm_utils {

NumaConfig::NumaConfig() {
#if __linux__
    num_numa_nodes = numa_max_node() + 1;
    num_numa_nodes_configured = numa_num_configured_nodes();
    num_numa_nodes_possible = numa_num_possible_nodes();
    num_cpus = std::thread::hardware_concurrency();
    num_cpus_possible = numa_num_possible_cpus();
    for (uint32_t i = 0; i < num_cpus; ++i) {
        all_cpus.push_back(i);
    }
    for (uint32_t j = 0; j < num_cpus_possible; ++j) {
        if (numa_bitmask_isbitset(numa_all_cpus_ptr, j)) {
            all_allowed_cpus.push_back(j);
        }
    }
    for (uint32_t i = 0 ; i < num_numa_nodes; ++i) {
        node_to_mem[i] = numa_node_size64(i, nullptr);
    }
    cpumask_ = numa_allocate_cpumask();
    for (uint32_t i = 0 ; i < num_numa_nodes; ++i) {
        node_to_cpus[i] = std::vector<uint32_t>();
        int err = numa_node_to_cpus(i, cpumask_);
        if (err != 0) {
            std::cerr << "error querying CPU mask for node " << i << std::endl;
            continue;
        }
        for (uint32_t j = 0; j < num_cpus_possible; ++j) {
            if (numa_bitmask_isbitset(cpumask_, j) > 0) {
                node_to_cpus[i].push_back(j);
            }
        }
    }
#endif
}


NumaConfig::~NumaConfig() {
#if __linux__
    numa_free_cpumask(cpumask_);
#endif
}


void NumaConfig::dump() const {
    std::cout << "# NUMA nodes: " << num_numa_nodes << " / "
              << num_numa_nodes_configured << " / "
              << num_numa_nodes_possible << std::endl;
    std::cout << "# CPUs: " << num_cpus << " / "
              << num_cpus_possible << std::endl;
    std::cout << "CPUs" << std::endl;
    std::cout << "\tAll CPUs:";
    for (const auto& idx: all_cpus) {
        std::cout << " " << idx;
    }
    std::cout << std::endl;
    std::cout << "\t My CPUs:";
    for (const auto& idx: all_allowed_cpus) {
        std::cout << " " << idx;
    }
    std::cout << std::endl;
    for (const auto& item : node_to_cpus) {
        std::cout << "\tNode " << item.first << ":";
        for (const auto& idx : item.second) {
            std::cout << " " << idx;
        }
        std::cout << std::endl;
    }

    std::cout << "Memory" << std::endl;
    for (const auto& item : node_to_mem) {
        std::cout << "\tNode " << item.first << ": ";
        std::cout << item.second << " B / "
                  << get_byte_str(item.second) << "B" << std::endl;
    }
}

}
