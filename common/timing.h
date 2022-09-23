#ifndef __COMMON_TIMING_H__
#define __COMMON_TIMING_H__

#include <cstdint>
#include <chrono>
#include <memory>
#include <ostream>
#include <string>

namespace mm_utils {

class Timer {
  public:
    using Handle = std::shared_ptr<Timer>;

    Timer() :
      elapsed_time_ns_ (0)
    { }
    ~Timer() = default;

    void startTimer() {
        time_point_begin_ = std::chrono::steady_clock::now();
    }
    void endTimer() {
        time_point_end_ = std::chrono::steady_clock::now();
        std::chrono::duration<int64_t, std::nano> diff = time_point_end_ - time_point_begin_;
        elapsed_time_ns_ += diff.count();
    }
    void resumeTimer() {
        time_point_begin_ = time_point_end_;
    }
    void restartTimer() {
        time_point_begin_ = std::chrono::steady_clock::now();
    }

    const int64_t& getElapsedTimeNs() const { return elapsed_time_ns_; }
    double getElapsedTimeUs() const { return elapsed_time_ns_ / 1e3; }
    double getElapsedTimeMs() const { return elapsed_time_ns_ / 1e6; }
    double getElapsedTime() const { return elapsed_time_ns_ / 1e9; }
    std::string getElapsedTimeStr() const;

  private:
    std::chrono::steady_clock::time_point time_point_begin_;
    std::chrono::steady_clock::time_point time_point_end_;
    int64_t elapsed_time_ns_;
};


void start_timer(const std::string& timer_key);
void end_timer(const std::string& timer_key, std::ostream& os);

}

#endif
