#ifndef __LIB_CONFIGURATION_H__
#define __LIB_CONFIGURATION_H__

#include <memory>
#include <string>
#include <boost/program_options.hpp>

#include "common/numa_config.h"

namespace mm_utils {

namespace {
    namespace po = boost::program_options;
}

enum class Testing_Type : int {
    LATENCY,
    BANDWIDTH,
    LATENCY_BANDWIDTH,
    MEMCPY,
    BRANCH_THROUGHPUT,
};


class Configuration {
  public:
    Configuration(Testing_Type testing_type);
    ~Configuration() = default;

    int parse_options(int argc, char** argv);
    void dump() const;

    std::string get_str_access_pattern(uint32_t x_access_pattern) const;
    std::string get_str_huge_page(uint32_t x_huge_page) const;
    std::string get_str_rw_mix(uint32_t x_rw_mix) const;

    const mm_utils::NumaConfig numa_config;

    uint32_t num_threads = 1;
    uint64_t region_size_kb = 128 * 1024;
    bool     no_binding = false;
    bool     verbose = false;
    uint32_t access_pattern = 1;
    uint32_t chunk_size_kb = 128;
    uint32_t stride_size_b = 64;
    uint64_t fragment_size_b = 4096;
    uint32_t use_hugepage = 0;
    uint32_t read_write_mix = 0;
    int32_t  load_gen_delay = 0;
    uint32_t target_duration_s = 10;
    bool     latency_matrix = false;
    bool     bandwidth_matrix = false;
    bool     memcpy_matrix = false;

    const uint32_t read_write_mix_sweep = 100;

  private:
    Testing_Type testing_type_;
    std::shared_ptr<po::options_description> desc_ = nullptr;

    void add_generic_options_();
    void add_latency_options_();
    void add_bandwidth_options_();
    void add_memcpy_options_();
};

}

#endif
