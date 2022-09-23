#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_map>

#include "common/timing.h"

namespace mm_utils {

std::unordered_map<std::string, Timer::Handle> g_timer_map;
std::mutex g_timer_map_mu;


std::string Timer::getElapsedTimeStr() const {
    std::stringstream ss;
    if (getElapsedTime() > 1) {
        ss << std::setprecision(4) << getElapsedTime() << " sec";
    } else if (getElapsedTimeMs() > 1) {
        ss << std::setprecision(4) << getElapsedTimeMs() << " ms";
    } else if (getElapsedTimeUs() > 1) {
        ss << std::setprecision(4) << getElapsedTimeUs() << " us";
    } else {
        ss << getElapsedTimeNs() << " ns";
    }
    return ss.str();
}


void start_timer(const std::string& timer_key) {
    std::lock_guard<std::mutex> lock(g_timer_map_mu);
    // adding to timer pool needs to be thread safe
    g_timer_map[timer_key] = std::make_shared<Timer>();
    // start timer
    g_timer_map[timer_key]->startTimer();
}

void end_timer(const std::string& timer_key, std::ostream& os) {
    std::string elapsed_time_str;
    {
        try {
            std::lock_guard<std::mutex> lock(g_timer_map_mu);
            const Timer::Handle& timer = g_timer_map.at(timer_key);
            timer->endTimer();
            elapsed_time_str = timer->getElapsedTimeStr();
            // remove it
            g_timer_map.erase(timer_key);
        } catch (const std::out_of_range& oor) {
            std::cerr << "Out of range error: " << oor.what()
                << " on " << timer_key << std::endl;
        } catch (...) {
            std::cerr << "Timer error ..." << std::endl;
        }
    }
    std::string out_str = "timer <" + timer_key + "> elapsed: " +
        elapsed_time_str + "\n";
    os << out_str;
}

}
