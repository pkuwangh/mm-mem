#ifndef __WORKER_THREAD_MANAGER_H__
#define __WORKER_THREAD_MANAGER_H__

#include <cstdint>
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

    std::string getAlignedIndex(uint32_t idx) const;

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
    const uint32_t num_threads_;
    const uint32_t num_cpus_ = 0;
    const std::vector<uint32_t> cpu_core_id_;

    std::vector<pthread_t> workers_;
    std::vector<pthread_attr_t> attrs_;
    std::vector<void *(*)(void *)> start_routines_;
    std::vector<Packet> packets_;
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
    packets_ (num_threads)
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
void WorkerThreadManager<Packet>::create() {
    for (uint32_t i = 0; i < num_threads_; ++i) {
        if (start_routines_[i]) {
            pthread_create(
                &workers_[i],
                &attrs_[i],
                start_routines_[i],
                (void*)(&packets_[i]));
        }
    }
}

template <class Packet>
void WorkerThreadManager<Packet>::join() {
    for (uint32_t i = 0; i < num_threads_; ++i) {
        pthread_join(workers_[i], nullptr);
    }
}

}

#endif
