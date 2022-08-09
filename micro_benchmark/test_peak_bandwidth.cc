#include <iostream>
#include <string>
#include <cassert>
#include <cstdint>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>

#include "lib_mem_region.h"
#include "lib_timing.h"
#include "worker_bandwidth.h"


class Configuration {
  public:
    Configuration() {
        num_threads = std::thread::hardware_concurrency();
    }
    uint32_t num_threads;
    uint64_t region_size_kb = 128 * 1024;
    uint32_t read_write_mix = 0;
    uint32_t target_duration_s = 10;
};


int parse_options(Configuration& config, int argc, char** argv) {
    namespace po = boost::program_options;
    po::options_description desc("Peak bandwidth");
    desc.add_options()
        ("help,h", "print usage message")
        ("num_threads,n", po::value(&config.num_threads), "number of threads")
        ("region_size,b", po::value(&config.region_size_kb), "region size in KB")
        ("read_write_mix,m", po::value(&config.read_write_mix),
            "read/write mix\n"
            "0 - all read\n"
            "1 - 1:1 read/write\n"
            "2 - 2:1 read/write")
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
    std::cout << "read/write mix:    " << config.read_write_mix << std::endl;
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
            config.region_size_kb * 1024, 4096, 64
        );
    }
    std::vector<std::shared_ptr<std::thread>> workers(config.num_threads, nullptr);
    std::vector<uint64_t> finished_bytes(config.num_threads, 0);
    std::vector<double> exec_time(config.num_threads, 0);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i] = std::make_shared<std::thread>(
            mm_worker::bw_sequential,
            mm_worker::get_kernel_peak_load(config.read_write_mix),
            regions[i],
            config.read_write_mix,
            config.target_duration_s,
            config.num_threads,
            &finished_bytes[i],
            &exec_time[i]
        );
    }
    // done
    uint64_t total_bytes = 0;
    double total_exec_time = 0;
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i]->join();
        total_bytes += finished_bytes[i];
        total_exec_time += exec_time[i];
    }
    double mem_bw = total_bytes / total_exec_time * config.num_threads;
    std::cout << "Total Bandwidth: " << mem_bw / 1024 / 1024 << " MB/s | "
        << mem_bw / 1024 / 1024 / 1024 << " GB/s" << std::endl;
    return 0;
}
