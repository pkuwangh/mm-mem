#ifndef MM_UTILS_CYCLE_COUNTER_H
#define MM_UTILS_CYCLE_COUNTER_H

#include <cstdint>
#include <cerrno>
#include <cstring>
#include <string>

#ifdef __linux__
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace mm_utils {

// Counts actual user-space CPU cycles for the calling thread, not TSC ticks.
class CycleCounter {
  public:
    CycleCounter() {
#ifdef __linux__
        perf_event_attr attr{};
        attr.type = PERF_TYPE_HARDWARE;
        attr.size = sizeof(attr);
        attr.config = PERF_COUNT_HW_CPU_CYCLES;
        attr.disabled = 1;
        attr.exclude_kernel = 1;
        attr.exclude_hv = 1;
        attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
        fd_ = syscall(SYS_perf_event_open, &attr, 0, -1, -1, PERF_FLAG_FD_CLOEXEC);
        if (fd_ < 0) {
            error_ = std::strerror(errno);
        }
#else
        error_ = "hardware cycle counter unavailable on this platform";
#endif
    }

    ~CycleCounter() {
#ifdef __linux__
        if (fd_ >= 0) close(fd_);
#endif
    }
    CycleCounter(const CycleCounter&) = delete;
    CycleCounter& operator=(const CycleCounter&) = delete;

    bool start() {
#ifdef __linux__
        if (fd_ >= 0) {
            if (ioctl(fd_, PERF_EVENT_IOC_RESET, 0) == 0 &&
                ioctl(fd_, PERF_EVENT_IOC_ENABLE, 0) == 0) return true;
            error_ = std::strerror(errno);
        }
#endif
        return false;
    }

    double stop() {
#ifdef __linux__
        if (fd_ >= 0) {
            struct { uint64_t cycles, enabled, running; } result{};
            if (ioctl(fd_, PERF_EVENT_IOC_DISABLE, 0) != 0 ||
                read(fd_, &result, sizeof(result)) != static_cast<ssize_t>(sizeof(result))) {
                error_ = "could not read hardware cycles";
            } else if (!result.cycles || !result.running) {
                error_ = "hardware cycle counter did not run";
            } else if (result.running != result.enabled) {
                // Do not present multiplexed estimates as a measured maximum.
                error_ = "hardware cycle counter was multiplexed";
            } else {
                return static_cast<double>(result.cycles);
            }
        }
#endif
        return 0;
    }

    const std::string& error() const { return error_; }

  private:
#ifdef __linux__
    int fd_ = -1;
#endif
    std::string error_;
};

} // namespace mm_utils
#endif
