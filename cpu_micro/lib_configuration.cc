#include <iostream>
#include <thread>

#include "cpu_micro/lib_configuration.h"

namespace mm_utils {

Configuration::Configuration(Testing_Type testing_type) :
    testing_type_ (testing_type)
{
    num_total_threads = std::thread::hardware_concurrency();
    std::string test_name;
    if (testing_type == Testing_Type::LATENCY) {
        test_name = "Idle latency";
    } else if (testing_type == Testing_Type::BANDWIDTH) {
        test_name = "Peak bandwidth";
    } else if (testing_type == Testing_Type::LATENCY_BANDWIDTH) {
        test_name = "Loaded latency; 1 latency thread + n-1 bandwidth threads";
    } else if (testing_type == Testing_Type::MEMCPY) {
        test_name = "Memcpy (glibc)";
    } else if (testing_type == Testing_Type::BRANCH_THROUGHPUT) {
        test_name = "Branch taken throughput";
    }
    desc_ = std::make_shared<po::options_description>(test_name);

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
    uint32_t default_num_threads = num_total_threads;
    uint64_t default_region_size_kb = 128 * 1024;
    if (testing_type_ == Testing_Type::LATENCY) {
        default_num_threads = 1;
        default_region_size_kb = 512 * 1024;
    }
    if (testing_type_ == Testing_Type::BRANCH_THROUGHPUT) {
        default_num_threads = 1;
    }
    po::options_description generic_options("Generic options");
    generic_options.add_options()
        ("help,h", "print usage message")
        ("target_duration,t",
            po::value(&target_duration_s)->default_value(10), "duration")
        ("num_threads,n",
            po::value(&num_threads)->default_value(default_num_threads),
            "total number of threads")
       ;
    if (testing_type_ < Testing_Type::BRANCH_THROUGHPUT) {
        generic_options.add_options()
            ("region_size,b",
                po::value(&region_size_kb)->default_value(default_region_size_kb),
                "region size in KB")
            ;
    }
    desc_->add(generic_options);
}

void Configuration::add_latency_options_() {
    po::options_description latency_options("Latency thread options");
    latency_options.add_options()
        ("access_pattern,p",
            po::value(&access_pattern)->default_value(1),
            ("access pattern\n  0 - " +
             get_str_access_pattern(0) + "\n  1 - " +
             get_str_access_pattern(1) + "\n  2 - " +
             get_str_access_pattern(2)).c_str())
        ("chunk_size,c",
            po::value(&chunk_size_kb)->default_value(128),
            "chunk size in KB - target L1TLB can cover")
        ("stride_size,s",
            po::value(&stride_size_b)->default_value(64),
            "stride size in byte")
        ("use_hugepage,H",
            po::value(&use_hugepage)->default_value(0),
            ("use huge pages\n  0 - " +
             get_str_huge_page(0) + "\n  1 - " +
             get_str_huge_page(1) + "\n  2 - " +
             get_str_huge_page(2)).c_str())
        ;
    desc_->add(latency_options);
}

void Configuration::add_bandwidth_options_() {
    uint32_t default_read_write_mix = 2;
    std::string additional_msg = "";
    if (testing_type_ == Testing_Type::BANDWIDTH) {
        default_read_write_mix = 100;
        additional_msg = "\n 100 - " + get_str_rw_mix(100);
    }
    po::options_description bandwidth_options("Bandwidth threads options");
    bandwidth_options.add_options()
        ("read_write_mix,m",
            po::value(&read_write_mix)->default_value(default_read_write_mix),
            ("read/write mix\n  0 - " +
             get_str_rw_mix(0) + "\n  1 - " +
             get_str_rw_mix(1) + "\n  2 - " +
             get_str_rw_mix(2) + "\n  3 - " +
             get_str_rw_mix(3) + additional_msg).c_str())
        ;
    desc_->add(bandwidth_options);
}

void Configuration::add_memcpy_options_() {
    uint64_t default_fragment_size = 0;
    po::options_description memcpy_options("Memcpy options");
    memcpy_options.add_options()
        ("fragment_size,f",
            po::value(&fragment_size_b)->default_value(default_fragment_size),
            ("fragment size in byte of each memcpy invocation"
             "\n  0 - same as region size"
             "\n  e.g. 4096 - copy each 4KB fragment one by one"))
        ;
    desc_->add(memcpy_options);
}

int Configuration::parse_options(int argc, char** argv) {
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(*desc_).run(), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cerr << *desc_ << std::endl;
        return 1;
    }
    return 0;
}

std::string Configuration::get_str_access_pattern(uint32_t x_access_pattern) {
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

std::string Configuration::get_str_huge_page(uint32_t x_huge_page) {
    if (x_huge_page == 0) {
        return "No huge page";
    } else if (x_huge_page == 1) {
        return "2MB huge page";
    } else if (x_huge_page == 2) {
        return "1GB huge page";
    } else {
        return "invalid";
    }
}

std::string Configuration::get_str_rw_mix(uint32_t x_rw_mix) {
    if (x_rw_mix == 0) {
        return "all reads";
    } else if (x_rw_mix == 1) {
        return "1:1 read/write";
    } else if (x_rw_mix == 2) {
        return "2:1 read/write";
    } else if (x_rw_mix == 3) {
        return "3:1 read/write";
    } else if (x_rw_mix == 100) {
        return "sweep read/write ratio";
    } else {
        return "invalid";
    }
}

void Configuration::dump() {
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
        std::cout << "fragment size in B: " << fragment_size_b << std::endl;
    }
    std::cout << "target duration:   " << target_duration_s << std::endl;
}

}
