#include <iomanip>
#include <iostream>
#include <string>
#include <cassert>
#include <cstdint>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>

#include "lib_mem_region.h"
#include "lib_timing.h"
#include "worker_latency.h"


class Configuration {
  public:
    Configuration() = default;
    uint32_t num_threads = 1;
    uint64_t region_size_kb = 128 * 1024;
    uint32_t access_pattern = 1;
    uint32_t chunk_size_kb = 128;
    uint32_t stride_size_b = 64;
    uint32_t use_hugepage = 0;
    uint32_t target_duration_s = 10;
};


int parse_options(Configuration& config, int argc, char** argv) {
    namespace po = boost::program_options;
    po::options_description desc("Idle latency");
    desc.add_options()
        ("help,h", "print usage message")
        ("num_threads,n", po::value(&config.num_threads), "number of threads")
        ("region_size,b", po::value(&config.region_size_kb), "region size in KB")
        ("access_pattern,p", po::value(&config.access_pattern),
            "access pattern\n"
            "0 - sequential\n"
            "1 - random in chunk\n"
            "2 - random in region")
        ("chunk_size,c", po::value(&config.chunk_size_kb), "chunk size in KB")
        ("stride_size,s", po::value(&config.stride_size_b), "stride size in byte")
        ("use_hugepage,H", po::value(&config.use_hugepage),
            "use huge pages\n"
            "0 - NOT to use huge pages\n"
            "1 - use 2MB huge pages\n"
            "2 - use 1GB huge pages")
        ("target_duration,t", po::value(&config.target_duration_s), "duration");
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cerr << desc << std::endl;
        return 1;
    }
    return 0;
}


void dump_configs(const Configuration& config) {
    std::cout << "threads:           " << config.num_threads << std::endl;
    std::cout << "region size in KB: " << config.region_size_kb << std::endl;
    std::cout << "chunk size in KB:  " << config.chunk_size_kb << std::endl;
    std::cout << "stride size in B:  " << config.stride_size_b << std::endl;
    std::cout << "access pattern:    " << config.access_pattern << std::endl;
    std::cout << "use hugepage:      " << config.use_hugepage << std::endl;
    std::cout << "target duration:   " << config.target_duration_s << std::endl;
}


int main(int argc, char** argv) {
    Configuration config;
    if (parse_options(config, argc, argv)) {
        return 1;
    }
    dump_configs(config);
    // setup memory regions & workers
    std::vector<mm_utils::MemRegion::Handle> regions(config.num_threads, nullptr);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        regions[i] = std::make_shared<mm_utils::MemRegion>(
            config.region_size_kb * 1024,
            config.chunk_size_kb * 1024,
            config.stride_size_b,
            static_cast<mm_utils::HugePageType>(config.use_hugepage)
        );
        if (config.access_pattern == 0) {
            regions[i]->stride_init();
        } else if (config.access_pattern == 1) {
            regions[i]->page_random_init();
        } else if (config.access_pattern == 2) {
            regions[i]->all_random_init();
        } else {
            std::cerr << "invalid access pattern" << std::endl;
            return 1;
        }
        // regions[i]->dump();
    }
    std::vector<std::shared_ptr<std::thread>> workers(config.num_threads, nullptr);
    std::vector<uint64_t> finished_chases(config.num_threads, 0);
    std::vector<double> exec_time(config.num_threads, 0);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i] = std::make_shared<std::thread>(
            mm_worker::lat_ptr,
            regions[i],
            config.target_duration_s,
            &finished_chases[i],
            &exec_time[i]
        );
    }
    // done
    uint64_t total_chases = 0;
    double total_exec_time = 0;
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i]->join();
        total_chases += finished_chases[i];
        total_exec_time += exec_time[i];
    }
    double latency = total_exec_time * 1e9 / total_chases;
    std::cout << "Idle Latency: " << std::setprecision(4) << latency << " ns" << std::endl;
    return 0;
}
