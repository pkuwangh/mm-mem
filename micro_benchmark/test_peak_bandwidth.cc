#include <iomanip>
#include <iostream>
#include <string>
#include <cassert>
#include <cstdint>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>

#include "lib_configuration.h"
#include "lib_mem_region.h"
#include "lib_timing.h"
#include "worker_bandwidth.h"
#include "worker_kernels_bandwidth.h"

void measure_peak_bandwidth(
    mm_utils::Configuration& config,
    std::vector<mm_utils::MemRegion::Handle>& regions,
    std::vector<std::shared_ptr<std::thread>>& workers,
    uint32_t read_write_mix,
    mm_worker::kernel_function& kernel
) {
    std::vector<uint64_t> finished_bytes(config.num_threads, 0);
    std::vector<double> exec_time(config.num_threads, 0);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i].reset();
    }
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i] = std::make_shared<std::thread>(
            mm_worker::bw_sequential_no_ref,
            kernel,
            regions[i],
            read_write_mix,
            config.target_duration_s,
            config.num_total_threads,
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
    double mem_bw = total_bytes / total_exec_time * config.num_threads / 1024 / 1024;
    std::cout << std::setw(20) << config.get_str_rw_mix(read_write_mix);
    std::cout << " : " << std::setprecision(7) << std::setw(10) << mem_bw << " MB/s | ";
    std::cout << std::setprecision(4) << std::setw(7) << mem_bw / 1024 << " GB/s" << std::endl;
}


int main(int argc, char** argv) {
    mm_utils::Configuration config(mm_utils::Testing_Type::BANDWIDTH);
    if (config.parse_options(argc, argv)) {
        return 1;
    }
    config.dump();
    // setup memory regions & workers
    std::vector<mm_utils::MemRegion::Handle> regions(config.num_threads, nullptr);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        regions[i] = std::make_shared<mm_utils::MemRegion>(
            config.region_size_kb * 1024, 4096, 64
        );
    }
    std::vector<std::shared_ptr<std::thread>> workers(config.num_threads, nullptr);
    std::vector<std::tuple<uint32_t, mm_worker::kernel_function>> rwmix_and_kernels;
    mm_worker::get_kernels_with_wrmix(rwmix_and_kernels, config.read_write_mix);
    for (auto& item : rwmix_and_kernels) {
        uint32_t read_write_mix = std::get<0>(item);
        mm_worker::kernel_function& kernel = std::get<1>(item);
        measure_peak_bandwidth(config, regions, workers, read_write_mix, kernel);
    }
    return 0;
}
