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
#include "worker_bandwidth.h"
#include "worker_latency.h"


class Configuration {
  public:
    Configuration() {
        num_threads = std::thread::hardware_concurrency();
    }
    uint32_t num_threads;
    uint64_t region_size_kb = 128 * 1024;
    uint32_t access_pattern = 1;
    uint32_t chunk_size_kb = 128;
    uint32_t stride_size_b = 64;
    uint32_t use_hugepage = 0;
    uint32_t read_write_mix = 0;
    uint32_t target_duration_s = 10;
};


int parse_options(Configuration& config, int argc, char** argv) {
    namespace po = boost::program_options;
    po::options_description desc("Loaded latency; 1 latency thread + n-1 bandwidth threads");
    desc.add_options()
        ("help,h", "print usage message")
        ("num_threads,n", po::value(&config.num_threads), "total number of threads")
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
    std::cout << "chunk size in KB:  " << config.chunk_size_kb << std::endl;
    std::cout << "stride size in B:  " << config.stride_size_b << std::endl;
    std::cout << "access pattern:    " << config.access_pattern << std::endl;
    std::cout << "use hugepage:      " << config.use_hugepage << std::endl;
    std::cout << "read/write mix:    " << config.read_write_mix << std::endl;
    std::cout << "target duration:   " << config.target_duration_s << std::endl;
}


void measure_loaded_latency(
    Configuration& config,
    std::vector<mm_utils::MemRegion::Handle>& regions,
    std::vector<std::shared_ptr<std::thread>>& workers,
    uint32_t delay,
    mm_worker::kernel_function& kernel
) {
    std::vector<uint64_t> finished_bytes(config.num_threads, 0);
    std::vector<double> exec_time(config.num_threads, 0);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i].reset();
    }
    // latency thread
    workers[0] = std::make_shared<std::thread>(
        mm_worker::lat_ptr,
        regions[0],
        config.target_duration_s,
        &finished_bytes[0],
        &exec_time[0]
    );
    // bandwidth thread
    for (uint32_t i = 1; i < config.num_threads; ++i) {
        workers[i] = std::make_shared<std::thread>(
            mm_worker::bw_sequential,
            kernel,
            regions[i],
            config.read_write_mix,
            config.target_duration_s,
            config.num_threads,
            &finished_bytes[i],
            &exec_time[i]
        );
    }
    // done
    workers[0]->join();
    uint64_t latency_chases = finished_bytes[0];
    uint64_t total_bytes = latency_chases * config.stride_size_b;
    double latency_exec_time = exec_time[0];
    double total_exec_time = latency_exec_time;
    for (uint32_t i = 1; i < config.num_threads; ++i) {
        workers[i]->join();
        total_bytes += finished_bytes[i];
        total_exec_time += exec_time[i];
    }
    double latency = latency_exec_time * 1e9 / latency_chases;
    double mem_bw = total_bytes / total_exec_time * config.num_threads;
    std::cout << std::setw(12) << delay;
    std::cout << std::setw(12) << std::setprecision(6) << mem_bw / 1024 / 1024;
    std::cout << std::setw(12) << std::setprecision(4) << latency << std::endl;
}


int main(int argc, char** argv) {
    Configuration config;
    if (parse_options(config, argc, argv)) {
        return 1;
    }
    dump_configs(config);
    // setup memory regions
    std::vector<mm_utils::MemRegion::Handle> regions(config.num_threads, nullptr);
    // latency thread
    regions[0] = std::make_shared<mm_utils::MemRegion>(
        config.region_size_kb * 1024,
        config.chunk_size_kb * 4096,
        config.stride_size_b,
        static_cast<mm_utils::HugePageType>(config.use_hugepage)
    );
    if (config.access_pattern == 0) {
        regions[0]->stride_init();
    } else if (config.access_pattern == 1) {
        regions[0]->page_random_init();
    } else if (config.access_pattern == 2) {
        regions[0]->all_random_init();
    } else {
        return 1;
    }
    // bandwidth thread
    for (uint32_t i = 1; i < config.num_threads; ++i) {
        regions[i] = std::make_shared<mm_utils::MemRegion>(
            config.region_size_kb * 1024, 4096, 64
        );
    }
    // setup workers
    std::vector<std::shared_ptr<std::thread>> workers(config.num_threads, nullptr);
    std::vector<std::tuple<uint32_t, mm_worker::kernel_function>> delays_and_kernels;
    mm_worker::get_kernels_with_delays(delays_and_kernels, config.read_write_mix);
    std::cout << std::setw(12) << "delay";
    std::cout << std::setw(12) << "bandwidth";
    std::cout << std::setw(12) << "latency" << std::endl;
    for (auto& item : delays_and_kernels) {
        uint32_t delay = std::get<0>(item);
        mm_worker::kernel_function& kernel = std::get<1>(item);
        measure_loaded_latency(config, regions, workers, delay, kernel);
    }
    return 0;
}
