#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>

#include "common/mem_region.h"
#include "common/timing.h"
#include "cpu_micro/lib_configuration.h"
#include "cpu_micro/worker_bandwidth.h"
#include "cpu_micro/worker_kernels_bandwidth.h"

uint32_t measure_peak_bandwidth(
    mm_utils::Configuration& config,
    std::vector<mm_utils::MemRegion::Handle>& regions,
    std::vector<std::shared_ptr<std::thread>>& workers,
    uint32_t read_write_mix,
    uint32_t last_measured_bw_gbps,
    mm_worker::func_kernel_bw& kernel
) {
    std::vector<uint64_t> finished_bytes(config.num_threads, 0);
    std::vector<double> exec_time(config.num_threads, 0);
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i].reset();
    }
    for (uint32_t i = 0; i < config.num_threads; ++i) {
        workers[i] = std::make_shared<std::thread>(
            mm_worker::bw_sequential,
            kernel,
            regions[i],
            read_write_mix,
            (last_measured_bw_gbps > 0) ? config.target_duration_s : 1,
            last_measured_bw_gbps,
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
    double mem_bw = total_bytes / total_exec_time * config.num_threads;
    double mem_bw_mbps = mem_bw / 1024 / 1024;
    double mem_bw_gbps = mem_bw_mbps / 1024;
    if (last_measured_bw_gbps > 0) {
        std::cout << std::setw(20) << config.get_str_rw_mix(read_write_mix) << " :";
        std::cout << std::setprecision(7) << std::setw(10) << mem_bw_mbps << " MB/s | ";
        std::cout << std::setprecision(4) << std::setw(7) << mem_bw_gbps << " GB/s";
        std::cout << std::endl;
    }
    return static_cast<uint32_t>(mem_bw_gbps);
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
    std::list<std::tuple<uint32_t, mm_worker::func_kernel_bw>> rwmix_and_kernels;
    mm_worker::get_kernels_with_wrmix(rwmix_and_kernels, config.read_write_mix);
    uint32_t last_bw_gbps = 0;
    rwmix_and_kernels.push_front(rwmix_and_kernels.front());
    for (auto& item : rwmix_and_kernels) {
        uint32_t read_write_mix = std::get<0>(item);
        mm_worker::func_kernel_bw& kernel = std::get<1>(item);
        last_bw_gbps = measure_peak_bandwidth(
            config, regions, workers, read_write_mix, last_bw_gbps, kernel);
    }
    std::cout << std::endl;
    return 0;
}
