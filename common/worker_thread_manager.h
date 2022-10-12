#ifndef __WORKER_THREAD_MANAGER_H__
#define __WORKER_THREAD_MANAGER_H__

#include <cstdint>
#include <iostream>
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
        bool do_binding);
    ~WorkerThreadManager() = default;

    void dumpThreadMapping() const;

    Packet& getPacket(const uint32_t& idx) { return packets_[idx]; }
    const Packet& getPacket(const uint32_t& idx) const { return packets_[idx]; }

    template <class UnaryPredicate>
    void setRoutine(void *(*start_routine)(void *), UnaryPredicate pred);
    void setRoutine(void *(*start_routine)(void *));

    void create();
    void join();

  private:
    const uint32_t num_threads_;
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
        bool do_binding) :
    num_threads_ (num_threads),
    cpu_core_id_ (cpu_core_id),
    workers_ (num_threads),
    attrs_ (num_threads),
    start_routines_ (num_threads, nullptr),
    packets_ (num_threads)
{
    if (do_binding && cpu_core_id_.size() != num_threads_) {
        std::cerr << "CPU Core ID list not match thread count; "
                  << " so will NOT do binding" << std::endl;
        do_binding = false;
    }
    for (uint32_t i = 0; i < num_threads_; ++i) {
        // prepare thread attrs
        pthread_attr_init(&attrs_[i]);
        if (do_binding) {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(cpu_core_id_[i], &cpuset);
            // set thread attribute
            pthread_attr_setaffinity_np(&attrs_[i], sizeof(cpu_set_t), &cpuset);
        }
        // thread packet basics
        packets_[i].setThreadId(num_threads_, i);
    }

}

template <class Packet>
void WorkerThreadManager<Packet>::dumpThreadMapping() const {
    if (cpu_core_id_.size() == num_threads_) {
        std::cout << "thread ID: [";
        for (uint32_t i = 0; i < num_threads_; ++i) {
            std::cout << i;
            if (num_threads_ > 100 && i < 100) std::cout << " ";
            if (i < 10) std::cout << " ";
            if (i < num_threads_ - 1) std::cout << " ";
        }
        std::cout << "]\n core  ID: [";
        for (uint32_t i = 0; i < num_threads_; ++i) {
            std::cout << cpu_core_id_[i];
            if (num_threads_ > 100 && i < 100) std::cout << " ";
            if (i < 10) std::cout << " ";
            if (i < num_threads_ - 1) std::cout << " ";
        }
        std::cout << "]" << std::endl;
    } else {
        std::cout << "No valid thread-core mapping" << std::endl;
    }
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
