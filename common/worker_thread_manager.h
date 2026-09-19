#ifndef __WORKER_THREAD_MANAGER_H__
#define __WORKER_THREAD_MANAGER_H__

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <pthread.h>

namespace mm_utils {

class BaseThreadPacket {
  public:
    BaseThreadPacket() = default;
    virtual ~BaseThreadPacket() = default;

    void setThreadId(uint32_t num_threads, uint32_t tid);
    const uint32_t& getNumThreads() const { return num_threads_; }
    const uint32_t& getThreadId() const { return thread_id_; }

  private:
    uint32_t num_threads_ = 0;
    uint32_t thread_id_ = 0;
};


template <class Packet>
class WorkerThreadManager {
  public:
    WorkerThreadManager(
        uint32_t num_threads,
        const std::vector<uint32_t>& cpu_core_id,
        bool do_binding,
        bool verbose);

    virtual ~WorkerThreadManager() {
        for (uint32_t i = 0; i < num_threads_; ++i) {
            pthread_attr_destroy(&attrs_[i]);
        }
    }

    std::string getAlignedIndex(uint32_t idx) const;

    uint32_t getNumThreads() const { return num_threads_; }

    Packet& getPacket(const uint32_t& idx) { return packets_[idx]; }
    const Packet& getPacket(const uint32_t& idx) const { return packets_[idx]; }

    template <class UnaryPredicate>
    void setRoutine(void *(*start_routine)(void *), UnaryPredicate pred);
    void setRoutine(void *(*start_routine)(void *));

    void create();
    void join();

    void run() {
        create();
        join();
    }
    void setRoutineAndRun(void *(*start_routine)(void *)) {
        setRoutine(start_routine);
        run();
    }

  private:
    // Seconds join() waits for a worker before declaring it stuck. Override with
    // MM_MEM_JOIN_TIMEOUT_S; 0 disables the watchdog and restores a plain join.
    static uint64_t joinTimeoutSec_();
    std::string describeThread_(uint32_t i) const;

    // An enumerator, not a static data member: the vector fill constructor below
    // takes it by reference, which would ODR-use a static member and need an
    // out-of-line definition under C++14.
    enum : uint32_t { kNotBound = UINT32_MAX };

    const uint32_t num_threads_;
    const uint32_t num_cpus_ = 0;
    const std::vector<uint32_t> cpu_core_id_;

    std::vector<pthread_t> workers_;
    std::vector<pthread_attr_t> attrs_;
    std::vector<void *(*)(void *)> start_routines_;
    std::vector<Packet> packets_;
    // Which CPU each thread was pinned to, and whether it actually started.
    std::vector<uint32_t> bound_cpu_;
    std::vector<char> created_;
};


void BaseThreadPacket::setThreadId(uint32_t num_threads, uint32_t tid) {
    num_threads_ = num_threads;
    thread_id_ = tid;
}


template <class Packet>
WorkerThreadManager<Packet>::WorkerThreadManager(
        uint32_t num_threads,
        const std::vector<uint32_t>& cpu_core_id,
        bool do_binding,
        bool verbose) :
    num_threads_ (num_threads),
    num_cpus_ (std::thread::hardware_concurrency()),
    cpu_core_id_ (cpu_core_id),
    workers_ (num_threads),
    attrs_ (num_threads),
    start_routines_ (num_threads, nullptr),
    packets_ (num_threads),
    bound_cpu_ (num_threads, kNotBound),
    created_ (num_threads, 0)
{
    if (do_binding && cpu_core_id_.size() == 0) {
        std::cerr << "CPU Core ID list empty; "
                  << "so will NOT do binding" << std::endl;
        do_binding = false;
    }
    if (do_binding && verbose) {
        std::cout << "\nthread ID: [ ";
        for (uint32_t i = 0; i < num_threads_; ++i) {
            std::cout << getAlignedIndex(i);
        }
        std::cout << "]\n core  ID: [ ";
    }
    for (uint32_t i = 0; i < num_threads_; ++i) {
        // prepare thread attrs
        pthread_attr_init(&attrs_[i]);
        if (do_binding) {
            uint32_t idx = (num_threads_ == 1) ? (cpu_core_id_.size() / 4) : i;
            uint32_t j = cpu_core_id[idx % cpu_core_id_.size()];
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(j, &cpuset);
            // set thread attribute
            pthread_attr_setaffinity_np(&attrs_[i], sizeof(cpu_set_t), &cpuset);
            bound_cpu_[i] = j;
            if (verbose) {
                std::cout << getAlignedIndex(j);
            }
        }
        // thread packet basics
        packets_[i].setThreadId(num_threads_, i);
    }
    if (do_binding && verbose) {
        std::cout << "]" << std::endl;
    }
}

template <class Packet>
std::string WorkerThreadManager<Packet>::getAlignedIndex(uint32_t idx) const {
    std::stringstream ss;
    ss << idx;
    if (idx < 100 && (num_threads_ > 100 || num_cpus_ > 100)) ss << " ";
    if (idx < 10) ss << " ";
    ss << " ";
    return ss.str();
}

template <class Packet>
template <class UnaryPredicate>
void WorkerThreadManager<Packet>::setRoutine(
        void *(*start_routine)(void *), UnaryPredicate pred) {
    for (uint32_t i = 0; i < num_threads_; ++i) {
        if (pred(i)) {
            start_routines_[i] = start_routine;
        }
    }
}

template <class Packet>
void WorkerThreadManager<Packet>::setRoutine(void *(*start_routine)(void *)) {
    for (uint32_t i = 0; i < num_threads_; ++i) {
        start_routines_[i] = start_routine;
    }
}

template <class Packet>
std::string WorkerThreadManager<Packet>::describeThread_(uint32_t i) const {
    std::stringstream ss;
    ss << "thread " << i;
    if (bound_cpu_[i] != kNotBound) {
        ss << " (pinned to cpu " << bound_cpu_[i] << ")";
    } else {
        ss << " (unpinned)";
    }
    return ss.str();
}

template <class Packet>
uint64_t WorkerThreadManager<Packet>::joinTimeoutSec_() {
    const char* env = getenv("MM_MEM_JOIN_TIMEOUT_S");
    if (env == nullptr || *env == '\0') {
        return 300;
    }
    return strtoull(env, nullptr, 10);
}

template <class Packet>
void WorkerThreadManager<Packet>::create() {
    uint32_t failed = 0;
    for (uint32_t i = 0; i < num_threads_; ++i) {
        created_[i] = 0;
        if (start_routines_[i]) {
            // An ignored return here is not harmless: workers_[i] stays
            // uninitialised and the matching join() then waits on garbage.
            int rc = pthread_create(
                &workers_[i],
                &attrs_[i],
                start_routines_[i],
                (void*)(&packets_[i]));
            if (rc != 0) {
                std::cerr << "pthread_create failed for " << describeThread_(i)
                          << ": " << strerror(rc) << std::endl;
                ++failed;
            } else {
                created_[i] = 1;
            }
        }
    }
    if (failed > 0) {
        std::cerr << "could not start " << failed << " of " << num_threads_
                  << " worker threads; results would be wrong, so stopping."
                  << std::endl;
        exit(1);
    }
}

template <class Packet>
void WorkerThreadManager<Packet>::join() {
    // Only threads create() actually started are joinable.
    const uint64_t timeout_s = joinTimeoutSec_();

#if defined(__linux__) && defined(__GLIBC__)
    if (timeout_s > 0) {
        struct timespec deadline;
        clock_gettime(CLOCK_REALTIME, &deadline);
        deadline.tv_sec += static_cast<time_t>(timeout_s);

        uint32_t stuck = 0;
        for (uint32_t i = 0; i < num_threads_; ++i) {
            if (!created_[i]) {
                continue;
            }
            int rc = pthread_timedjoin_np(workers_[i], nullptr, &deadline);
            if (rc == ETIMEDOUT) {
                if (stuck == 0) {
                    std::cerr << "\nworker threads did not finish within "
                              << timeout_s << "s." << std::endl;
                }
                std::cerr << "  stuck: " << describeThread_(i) << std::endl;
                ++stuck;
            } else if (rc != 0) {
                std::cerr << "pthread_join failed for " << describeThread_(i)
                          << ": " << strerror(rc) << std::endl;
                ++stuck;
            }
        }
        if (stuck > 0) {
            std::cerr
                << stuck << " of " << num_threads_ << " threads never finished. "
                << "A pinned thread that makes no progress usually means its cpu "
                << "is already owned by a busy-polling thread (an OVS-DPDK pmd-cNN "
                << "thread, for example), so the worker is never scheduled there. "
                << "Check with: ps -eLo psr,pcpu,comm | awk '$1==<cpu>'. "
                << "Re-run with --no_binding, or restrict the run to cpus that are "
                << "free, to confirm. Set MM_MEM_JOIN_TIMEOUT_S to change this "
                << "timeout (0 waits forever)." << std::endl;
            exit(1);
        }
        return;
    }
#endif

    for (uint32_t i = 0; i < num_threads_; ++i) {
        if (created_[i]) {
            pthread_join(workers_[i], nullptr);
        }
    }
}

}

#endif
