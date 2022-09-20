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
#include <numaif.h>     // move_pages
#endif

#include "lib_mem_region.h"

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
    // std::cout << "OS page size: " << os_page_size_ << std::endl;

    // allocate & init region
    if (size_ > 0) {
        if (mem_type_ == MemType::NATIVE) {
            addr1_ = allocNative_(size_, raw_addr1_, raw_size1_);
        } else if (mem_type_ >= MemType::NODE0 && mem_type_ <= MemType::INTERLEAVE) {
            addr1_ = allocNumaAware_(size_, raw_addr1_, raw_size1_);
        }
        // std::cout << "Region-1 addr=0x" << std::hex << reinterpret_cast<uint64_t>(addr1_)
        //     << " raw_addr=0x" << reinterpret_cast<uint64_t>(raw_addr1_)
        //     << " end_addr=0x" << reinterpret_cast<uint64_t>(addr1_ + size_)
        //     << " size=" << std::dec << size_
        //     << " raw_size=" << std::dec << raw_size1_ << std::endl;
        memset(addr1_, 0, size_);
    }
    // use a fixed seed
    srand(0);
}

MemRegion::~MemRegion() {
    if (addr1_) {
        if (mem_type_ == MemType::NATIVE) {
            if (hugepage_type_ > HugePageType::NONE) {
                munmap(addr1_, size_);
            } else {
                free(raw_addr1_);
            }
        } else if (mem_type_ >= MemType::NODE0 &&
                   mem_type_ == MemType::INTERLEAVE) {
            // numa_free(raw_addr1_, raw_size1_);
            free(raw_addr1_);
        }
    }
    addr1_ = nullptr;
    raw_addr1_ = nullptr;
}

void MemRegion::error_(std::string message) {
    std::cout << message << std::endl;
    exit(1);
}

char* MemRegion::allocNative_(const uint64_t& size, char*& raw_addr, uint64_t& raw_size) {
    char* addr = NULL;
    if (hugepage_type_ > HugePageType::NONE) {
#if __linux__
        int hugepage_size = 0;
        if (hugepage_type_ == HugePageType::HGPG_2MB) {
            hugepage_size = 21;
        } else if (hugepage_type_ == HugePageType::HGPG_1GB) {
            hugepage_size = 30;
        }
        addr = ((char*)mmap(
            0x0, size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | (hugepage_size << MAP_HUGE_SHIFT),
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
        // std::cout << "native hugepage" << std::endl;
        if ((int64_t)addr == (int64_t)-1) {
            error_("mmap failed for hugepage");
        }
        raw_addr = addr;
        raw_size = size;
    } else {
        raw_size = size + os_page_size_;
        raw_addr = (char*)malloc(raw_size);
        addr = raw_addr + os_page_size_ - (uint64_t)raw_addr % os_page_size_;
        // std::cout << "native malloc" << std::endl;
    }
    return addr;
}

char* MemRegion::allocNumaAware_(const uint64_t& size, char*& raw_addr, uint64_t& raw_size) {
    char* addr = NULL;
    if (hugepage_type_ > HugePageType::NONE) {
        error_("not yet support hugepage to be numa-aware");
    } else {
        raw_size = size + os_page_size_;
#if __linux__
        raw_addr = (char*)aligned_alloc(numa_pagesize(), raw_size);
        unsigned long num_nodes = numa_max_node() + 1;
        unsigned long maxnode = num_nodes + 1;  // tihs seems to be a kernel/documentaion bug
        int mbind_ret = 0;
        if (mem_type_ == MemType::NODE0) {
            // raw_addr = (char*)numa_alloc_onnode(raw_size, 0);
            unsigned long nodemask = ((unsigned long)1 << 0);
            mbind_ret = mbind(raw_addr, raw_size, MPOL_PREFERRED, &nodemask, maxnode, 0);
        } else if (mem_type_ == MemType::NODE1) {
            // raw_addr = (char*)numa_alloc_onnode(raw_size, 1);
            unsigned long nodemask = ((unsigned long)1 << 1);
            mbind_ret = mbind(raw_addr, raw_size, MPOL_PREFERRED, &nodemask, maxnode, 0);
        } else if (mem_type_ == MemType::INTERLEAVE) {
            // raw_addr = (char*)numa_alloc_interleaved(raw_size);
            unsigned long nodemask = ((unsigned long)1 << (numa_max_node() + 1)) - 1;
            mbind_ret = mbind(raw_addr, raw_size, MPOL_INTERLEAVE, &nodemask, maxnode, 0);
        } else {
          error_("unknown mem type");
        }
        if (mbind_ret < 0) {
            error_("mbind error");
        }
#else
        raw_addr = (char*)aligned_alloc(os_page_size_, raw_size);
        // TODO
#endif
        addr = raw_addr + os_page_size_ - (uint64_t)raw_addr % os_page_size_;
        // std::cout << "numa_malloc" << std::endl;
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

// migrate pages to another node
void MemRegion::migratePages_(char*& addr, uint64_t size, int target_node) {
#if __linux__
    const bool verbose = true;
    uint32_t num_os_pages = size / os_page_size_;
    void** pages = (void**)malloc(sizeof(char*) * num_os_pages);
    int* nodes = (int*)malloc(sizeof(int*) * num_os_pages);
    int* status = (int*)malloc(sizeof(int*) * num_os_pages);
    for (uint32_t i = 0; i < num_os_pages; ++i) {
        pages[i] = addr + i * os_page_size_;
        nodes[i] = target_node;
        status[i] = 0;
    }
    // int ret = numa_move_pages(0, num_os_pages, pages, nodes, status, 0);
    int ret = move_pages(0, num_os_pages, pages, nodes, status, 0);
    if (ret != 0) {
        std::cout << "Page migration failed with retcode=" << ret << std::endl;
    }
    if (verbose || ret != 0) {
        for (uint32_t i = 0; i < num_os_pages; ++i) {
            bool to_print = (status[i] < 0 || status[i] != target_node ||
                             i <= 2 || i >= num_os_pages - 2);
            if (to_print) {
                std::cout << std::hex << "0x" << reinterpret_cast<uint64_t>(pages[i])
                    << std::dec << "\t" << i << "\t";
            }
            if (status[i] >= 0) {
                if (to_print) {
                    std::cout << "to node " << status[i];
                }
            } else if (status[i] == -EACCES) {
                std::cout << "mapped by multiple procs";
            } else if (status[i] == -EBUSY) {
                std::cout << "page busy";
            } else if (status[i] == -EFAULT) {
                std::cout << "fault b/c zero page or not mapped";
            } else if (status[i] == -EIO) {
                std::cout << "unable to write-back page";
            } else if (status[i] == -EINVAL) {
                std::cout << "the dirty page cannot be moved";
            } else if (status[i] == -ENOENT) {
                std::cout << "page is not present";
            } else if (status[i] == -ENOMEM) {
                std::cout << "unable to allocate on target node";
            } else {
                std::cout << "unknown errno";
            }
            if (to_print) {
                std::cout << std::endl;
            }
        }
    }
#endif
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
