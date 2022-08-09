#ifndef __LIB_CONFIGURATION_H__
#define __LIB_CONFIGURATION_H__

#include <memory>
#include <string>
#include <boost/program_options.hpp>

namespace mm_utils {

namespace {
    namespace po = boost::program_options;
}

enum class Testing_Type : int {
    LATENCY,
    BANDWIDTH,
    LATENCY_BANDWIDTH,
};


class Configuration {
  public:
    Configuration(Testing_Type testing_type);
    ~Configuration() = default;

    int parse_options(int argc, char** argv);
    void dump();

    uint32_t num_threads = 1;
    uint64_t region_size_kb = 128 * 1024;
    uint32_t access_pattern = 1;
    uint32_t chunk_size_kb = 128;
    uint32_t stride_size_b = 64;
    uint32_t use_hugepage = 0;
    uint32_t read_write_mix = 0;
    uint32_t target_duration_s = 10;

  private:
    Testing_Type testing_type_;
    std::shared_ptr<po::options_description> desc_ = nullptr;

    void add_generic_options_();
    void add_latency_options_();
    void add_bandwidth_options_();

    std::string get_str_access_pattern_(uint32_t x_access_pattern);
    std::string get_str_huge_page_(uint32_t x_huge_page);
    std::string get_str_rw_mix_(uint32_t x_rw_mix);
};

}

#endif
