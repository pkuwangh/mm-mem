#include <iostream>

#include "cpu_micro/lib_configuration.h"

namespace mm_utils {

Configuration::Configuration(Testing_Type testing_type) :
    testing_type_ (testing_type)
{
    std::string test_name;
    if (testing_type == Testing_Type::LATENCY) {
        test_name = "Idle latency";
    } else if (testing_type == Testing_Type::BANDWIDTH) {
        test_name = "Peak bandwidth";
    } else if (testing_type == Testing_Type::LATENCY_BANDWIDTH) {
        test_name = "Loaded latency; 1 latency thread + n-1 load generation threads";
    } else if (testing_type == Testing_Type::MEMCPY) {
        test_name = "Memcpy (glibc)";
    } else if (testing_type == Testing_Type::BRANCH_THROUGHPUT) {
        test_name = "Branch taken throughput";
    }
    options_ = CommandLine(test_name);

    add_generic_options_();
    if (testing_type == Testing_Type::LATENCY || testing_type == Testing_Type::LATENCY_BANDWIDTH) {
        add_latency_options_();
    }
    if (testing_type == Testing_Type::BANDWIDTH || testing_type == Testing_Type::LATENCY_BANDWIDTH) {
        add_bandwidth_options_();
    }
    if (testing_type == Testing_Type::MEMCPY) {
        add_memcpy_options_();
    }
}

void Configuration::add_generic_options_() {
    uint32_t default_num_threads = numa_config.all_allowed_cpus.size();
    uint64_t default_region_size_kb = 128 * 1024;
    if (testing_type_ == Testing_Type::LATENCY) {
        default_num_threads = 1;
        default_region_size_kb = 512 * 1024;
    }
    if (testing_type_ == Testing_Type::BRANCH_THROUGHPUT) {
        default_num_threads = 1;
    }
    options_.group("Generic options");
    options_.flag("help,h", &help_, "print usage message");
    options_.value("target_duration,t", &target_duration_s, 10, "duration");
    options_.value("num_threads,n", &num_threads, default_num_threads,
            "total number of threads");
    if (testing_type_ < Testing_Type::BRANCH_THROUGHPUT) {
        options_.value("region_size,b", &region_size_kb, default_region_size_kb,
                "region size in KB");
    }
    options_.flag("no_binding", &no_binding,
            "NOT to do thread-to-core binding");
    options_.flag("verbose,v", &verbose,
            "enable verbose output");
}

void Configuration::add_latency_options_() {
    uint32_t default_chunk_size_kb = 128;
    if (testing_type_ == Testing_Type::LATENCY_BANDWIDTH) {
        default_chunk_size_kb = 64;
    }
    options_.group("Latency thread options");
    options_.value("access_pattern,p", &access_pattern, 1,
            ("access pattern\n  0 - " +
             get_str_access_pattern(0) + "\n  1 - " +
             get_str_access_pattern(1) + "\n  2 - " +
             get_str_access_pattern(2)).c_str());
    options_.value("chunk_size,c", &chunk_size_kb, default_chunk_size_kb,
            "chunk size in KB - target L1TLB can cover");
    options_.value("stride_size,s", &stride_size_b, 128,
            "stride size in byte");
    options_.value("use_hugepage,H", &use_hugepage, 0,
            ("use huge pages\n  0 - " + get_str_huge_page(0) +
             "\n  1 - " + get_str_huge_page(1) +
             "\n  2 - " + get_str_huge_page(2) +
             "\n  3 - " + get_str_huge_page(3) +
             "\n  4 - " + get_str_huge_page(4)).c_str()
        );
    if (testing_type_ == Testing_Type::LATENCY) {
        options_.flag("latency_matrix", &latency_matrix,
                "Measure local and cross-socket latencies");
    }
}

void Configuration::add_bandwidth_options_() {
    uint32_t default_read_write_mix = 2;
    std::string additional_msg = "";
    if (testing_type_ == Testing_Type::BANDWIDTH) {
        default_read_write_mix = read_write_mix_sweep;
        additional_msg = (
            "\n" + std::to_string(read_write_mix_sweep) +
            " - " + get_str_rw_mix(read_write_mix_sweep));
    }
    options_.group("Bandwidth threads options");
    options_.value("read_write_mix,m", &read_write_mix, default_read_write_mix,
            ("read/write mix\n  0 - " +
             get_str_rw_mix(0) + "\n  1 - " +
             get_str_rw_mix(1) + "\n  2 - " +
             get_str_rw_mix(2) + "\n  3 - " +
             get_str_rw_mix(3) + additional_msg).c_str());
    if (testing_type_ == Testing_Type::LATENCY_BANDWIDTH) {
        int32_t default_load_gen_delay = 0;
        options_.value("load_gen_delay,d", &load_gen_delay, default_load_gen_delay,
                "delay slots b/w memory requests for load generation threads");
    }
    if (testing_type_ == Testing_Type::BANDWIDTH) {
        options_.flag("bandwidth_matrix", &bandwidth_matrix,
                "Measure local and cross-socket bandwidth");
    }
}

void Configuration::add_memcpy_options_() {
    uint64_t default_fragment_size = 0;
    options_.group("Memcpy options");
    options_.value("fragment_size,f", &fragment_size_b, default_fragment_size,
            ("fragment size in byte of each memcpy invocation"
             "\n  0 - same as region size"
             "\n  e.g. 4096 - copy each 4KB fragment one by one"));
    options_.flag("memcpy_matrix", &memcpy_matrix,
            "Measure local and cross-socket memcpy bandwidth");
}

int Configuration::parse_options(int argc, char** argv) {
    try {
        options_.parse(argc, argv);
    } catch (const std::invalid_argument& error) {
        std::cerr << "Error: " << error.what() << "\nUse --help for usage.\n";
        return 1;
    }
    if (help_) {
        options_.help(std::cerr);
        return 1;
    }
    // auto corrections
    if (use_hugepage > 0) {
        chunk_size_kb = region_size_kb;
    }
    return 0;
}

std::string Configuration::get_str_access_pattern(uint32_t x_access_pattern) const {
    if (x_access_pattern == 0) {
        return "sequential";
    } else if (x_access_pattern == 1) {
        return "random in chunk";
    } else if (x_access_pattern == 2) {
        return "random in full region";
    } else {
        return "invalid";
    }
}

std::string Configuration::get_str_access_pattern_short(uint32_t x_access_pattern) const {
    if (x_access_pattern == 0) {
        return "Sequential";
    } else if (x_access_pattern == 1) {
        return "RandomInChunk";
    } else if (x_access_pattern == 2) {
        return "FullRandom";
    } else {
        return "invalid";
    }
}

std::string Configuration::get_str_huge_page(uint32_t x_huge_page) const {
    if (x_huge_page == 0) {
        return "No huge page";
    } else if (x_huge_page == 1) {
        return "2MB huge page";
    } else if (x_huge_page == 2) {
        return "512MB huge page";
    } else if (x_huge_page == 3) {
        return "1GB huge page";
    } else if (x_huge_page == 4) {
        return "16GB huge page";
    } else {
        return "invalid";
    }
}

std::string Configuration::get_str_rw_mix(uint32_t x_rw_mix) const {
    if (x_rw_mix == 0) {
        return "all reads";
    } else if (x_rw_mix == 1) {
        return "1:1 read/write";
    } else if (x_rw_mix == 2) {
        return "2:1 read/write";
    } else if (x_rw_mix == 3) {
        return "3:1 read/write";
    } else if (x_rw_mix == read_write_mix_sweep) {
        return "sweep read/write ratio";
    } else {
        return "invalid";
    }
}

void Configuration::dump() const {
    std::cout << "threads:           " << num_threads << std::endl;
    if (testing_type_ < Testing_Type::BRANCH_THROUGHPUT) {
        std::cout << "region size in KB: " << region_size_kb << std::endl;
    }
    if (testing_type_ == Testing_Type::LATENCY || testing_type_ == Testing_Type::LATENCY_BANDWIDTH) {
        std::cout << "chunk size in KB:  " << chunk_size_kb << std::endl;
        std::cout << "stride size in B:  " << stride_size_b << std::endl;
        std::cout << "access pattern:    " << access_pattern << " - ";
        std::cout << get_str_access_pattern(access_pattern) << std::endl;
        std::cout << "use hugepage:      " << use_hugepage << " - ";
        std::cout << get_str_huge_page(use_hugepage) << std::endl;
    }
    if (testing_type_ == Testing_Type::BANDWIDTH || testing_type_ == Testing_Type::LATENCY_BANDWIDTH) {
        std::cout << "read/write mix:    " << read_write_mix << " - ";
        std::cout << get_str_rw_mix(read_write_mix) << std::endl;
    }
    if (testing_type_ == Testing_Type::MEMCPY) {
        if (fragment_size_b % 1024 == 0) {
            std::cout << "copy size in KB:   " << fragment_size_b / 1024;
        } else {
            std::cout << "copy size in B:    " << fragment_size_b;
        }
        std::cout << std::endl;
    }
    std::cout << "target duration:   " << target_duration_s << std::endl;
}

}
