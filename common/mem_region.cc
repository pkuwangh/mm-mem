#include <cassert>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <fcntl.h>      // open
#include <unistd.h>     // getpaegsize
#include <sys/mman.h>   // mmap

#if __linux__
#include <numa.h>       // numa_*
#include <numaif.h>     // mbind
#endif

#include "common/mem_region.h"

namespace mm_utils {

MemRegion::MemRegion(
        uint64_t size,
        uint64_t active_size,
        uint64_t page_size,
        uint64_t line_size,
        MemType mem_type,
        HugePageType hugepage_type) :
    size_ (size),
    active_size_ (active_size),
    page_size_ (page_size),
    line_size_ (line_size),
    mem_type_ (mem_type),
    hugepage_type_ (hugepage_type),
    num_all_pages_ (size_ / page_size_),
    num_active_pages_ (active_size_ / page_size_),
    num_lines_in_page_ (page_size_ / line_size_)
{
    if (page_size_ > active_size_) {
        page_size_ = 4096;
    }
    num_all_pages_ = size_ / page_size_;
    num_active_pages_ = active_size_ / page_size_;
    num_lines_in_page_ = page_size_ / line_size_;

    os_page_size_ = getpagesize();
    // std::cout << "OS page size: " << getpagesize() << std::endl;

    // allocate & init region
    if (size_ > 0) {
        addr1_ = allocNative_(size_);
        // std::cout << "Region-1"
        //     << " addr=0x" << std::hex << reinterpret_cast<uint64_t>(addr1_)
        //     << " end_addr=0x" << reinterpret_cast<uint64_t>(addr1_ + size_)
        //     << " size=" << std::dec << size_ << std::endl;
        memset(addr1_, 0, size_);
    }
    // use a fixed seed
    srand(0);
}

MemRegion::~MemRegion() {
    if (addr1_) {
        if (hugepage_type_ > HugePageType::NONE) {
            munmap(addr1_, real_size_);
        } else {
            free(addr1_);
        }
    }
    addr1_ = nullptr;
}

void MemRegion::error_(std::string message) {
    std::cout << message << std::endl;
    exit(1);
}

char* MemRegion::allocNative_(const uint64_t& size) {
    char* addr = nullptr;
    // set mem policy for numa aware allocation
    if (mem_type_ >= MemType::NODE0 && mem_type_ <= MemType::INTERLEAVE) {
#if __linux__
        int mode = MPOL_DEFAULT;
        unsigned long nodemask = 0;
        unsigned long maxnode = (numa_max_node() + 1) + 1;  // additional +1 ?
        if (mem_type_ == MemType::INTERLEAVE) {
            mode = MPOL_INTERLEAVE;
            nodemask = ((unsigned long)1 << (numa_max_node() + 1)) - 1;
        } else {
            mode = MPOL_BIND;
            int node = static_cast<int>(mem_type_) - static_cast<int>(MemType::NODE0);
            nodemask = ((unsigned long)1 << node);
        }
        long ret = set_mempolicy(mode, &nodemask, maxnode);
        if (ret < 0) {
            error_("set_mempolicy error");
        }
#else
        // TODO
#endif
    }
    // hugepage or regular page
    if (hugepage_type_ > HugePageType::NONE) {
        int hugepage_size_log2 = 0;
        if (hugepage_type_ == HugePageType::HGPG_2MB) {
            hugepage_size_log2 = 21;
        } else if (hugepage_type_ == HugePageType::HGPG_1GB) {
            hugepage_size_log2 = 30;
        }
        uint64_t padding = (size % ((uint64_t)1 << hugepage_size_log2) > 0) ? 1 : 0;
        real_size_ = ((size >> hugepage_size_log2) + padding) << hugepage_size_log2;
#if __linux__
        addr = ((char*)mmap(
            0x0, size,
            PROT_READ | PROT_WRITE,
            (MAP_PRIVATE | MAP_ANONYMOUS |
             MAP_HUGETLB | (hugepage_size_log2 << MAP_HUGE_SHIFT)),
            0, 0
        ));
#else
        addr = ((char*)mmap(
            0x0, size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,    // TODO
            0, 0
        ));
#endif
        if ((int64_t)addr == (int64_t)-1) {
            error_("mmap failed for hugepage");
        }
    } else {
        real_size_ = size;
#if __linux__
        addr = (char*)aligned_alloc(numa_pagesize(), size);
#else
        addr = (char*)aligned_alloc(os_page_size_, size);
#endif
    }
    return addr;
}

void MemRegion::randomizeSequence_(
    std::vector<uint64_t>& sequence, uint64_t size, uint64_t unit, bool in_order)
{
    // initialize to sequential pattern
    for (uint64_t i = 0; i < size; ++i) {
        sequence[i] = i * unit;
    }
    if (in_order) {
      return;
    }
    // randomize by swapping
    uint64_t r = rand() ^ ((uint64_t)(rand()) << 10);
    for (uint64_t i = size-1; i> 0; --i) {
        r = (r << 1) ^ rand();
        uint64_t tmp = sequence[r % (i+1)];
        sequence[r % (i+1)] = sequence[i];
        sequence[i] = tmp;
    }
    // start should still be start
    for (uint32_t i = 1; i < size; ++i) {
        if (sequence[i] == 0) {
            sequence[i] = sequence[0];
            sequence[0] = 0;
            break;
        }
    }
}

char* MemRegion::getOffsetAddr_(uint64_t offset) const {
    return &addr1_[offset];
}

// create a circular list of pointers with sequential stride
void MemRegion::stride_init() {
    uint64_t i = 0;
    for (i = line_size_; i < active_size_; i += line_size_) {
        *(char**)getOffsetAddr_(i - line_size_) = (char*)getOffsetAddr_(i);
    }
    *(char**)getOffsetAddr_(i - line_size_) = (char*)getOffsetAddr_(0);
}

// create a circular list of pointers with random-in-page
void MemRegion::page_random_init() {
    std::vector<uint64_t> pages_(num_active_pages_, 0);
    std::vector<uint64_t> linesInPage_(num_lines_in_page_, 0);
    randomizeSequence_(pages_, num_active_pages_, page_size_, true);
    randomizeSequence_(linesInPage_, num_lines_in_page_, line_size_);
    // run through the pages
    for (uint64_t i = 0; i < num_active_pages_; ++i) {
        // run through the lines within a page
        for (uint64_t j = 0; j < num_lines_in_page_ - 1; ++j) {
            *(char**)getOffsetAddr_(pages_.at(i) + linesInPage_.at(j))
                = (char*)getOffsetAddr_(pages_.at(i) + linesInPage_.at(j + 1));
        }
        // jump the next page
        uint64_t next_page = (i == num_active_pages_ - 1) ? 0 : (i + 1);
        *(char**)getOffsetAddr_(pages_.at(i) + linesInPage_.at(num_lines_in_page_ - 1))
            = (char*)getOffsetAddr_(pages_.at(next_page) + linesInPage_.at(0));
    }
}

// create a circular list of pointers with all-random
void MemRegion::all_random_init() {
    const uint64_t num_lines = numActiveLines();
    std::vector<uint64_t> lines_(num_lines, 0);
    randomizeSequence_(lines_, num_lines, line_size_);
    // run through the lines
    for (uint64_t i = 0; i < num_lines - 1; ++i) {
        *(char**)getOffsetAddr_(lines_.at(i)) = (char*)getOffsetAddr_(lines_.at(i + 1));
    }
    *(char**)getOffsetAddr_(lines_.at(num_lines - 1)) = (char*)getOffsetAddr_(lines_.at(0));
}

void MemRegion::dump()
{
    std::cout << "================================" << std::endl;
    std::cout << "size=" << size_ << ", page=" << page_size_ << ", line=" << line_size_
        << ", numPage=" << num_active_pages_ << "/" << num_all_pages_
        << ", numLinesInPage=" << num_lines_in_page_ << std::endl;
    const uint64_t start_addr = reinterpret_cast<uint64_t>(addr1_);
    for (uint64_t i = 0; i < size_; i += line_size_) {
        uint64_t curr = reinterpret_cast<uint64_t>(getOffsetAddr_(i));
        uint64_t next = reinterpret_cast<uint64_t>(*(char**)getOffsetAddr_(i));
        uint64_t curr_offset = (curr - start_addr) / line_size_;
        uint64_t next_offset = (next - start_addr) / line_size_;
        std::cout << std::hex << "[0x" << curr << "]: 0x" << next << std::dec << "  ";
        std::cout << std::setw(8) << std::right << curr_offset << ": " << std::setw(8) << std::left << next_offset;
        std::cout << std::endl;
    }
    //std::cout << "--------------------------------" << std::endl;
    //char** p = (char**)addr1_;
    //for (uint64_t i = 0; i < numActiveLines(); ++i) {
    //    uint64_t curr = reinterpret_cast<uint64_t>(p);
    //    uint64_t next = reinterpret_cast<uint64_t>(*p);
    //    uint64_t curr_offset = (curr - start_addr)) / line_size_;
    //    uint64_t next_offset = (next - start_addr)) / line_size_;
    //    std::cout << std::hex << "[0x" << curr << "]: 0x" << next << std::dec << "  ";
    //    std::cout << std::setw(8) << std::right << curr_offset << ": " << std::setw(8) << std::left << next_offset;
    //    if (next > curr) {
    //        std::cout << "+" << next_offset - curr_offset;
    //    } else {
    //        std::cout << "-" << curr_offset - next_offset;
    //    }
    //    std::cout << std::endl;
    //    p = (char**)(*p);
    //}
    std::cout << "================================" << std::endl;
}

}
