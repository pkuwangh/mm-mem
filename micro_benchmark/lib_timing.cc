#include <iostream>
#include <mutex>
#include <unordered_map>

#include "lib_timing.h"

namespace mm_utils {

std::unordered_map<std::string, Timer::Handle> g_timer_map;
std::mutex g_timer_map_mu;

void start_timer(const std::string& timer_key) {
    std::lock_guard<std::mutex> lock(g_timer_map_mu);
    // adding to timer pool needs to be thread safe
    g_timer_map[timer_key] = std::make_shared<Timer>();
    // start timer
    g_timer_map[timer_key]->startTimer();
}

void end_timer(const std::string& timer_key, std::ostream& os) {
    float elapsed_time = 0;
    {
        try {
            std::lock_guard<std::mutex> lock(g_timer_map_mu);
            const Timer::Handle& timer = g_timer_map.at(timer_key);
            timer->endTimer();
            elapsed_time = timer->getElapsedTime();
            // remove it
            g_timer_map.erase(timer_key);
        } catch (const std::out_of_range& oor) {
            std::cerr << "Out of range error: " << oor.what() << " on " << timer_key << std::endl;
        } catch (...) {
            std::cerr << "Timer error ..." << std::endl;
        }
    }
    std::string out_str = "timer <" + timer_key + "> elapsed:" +
        " total(s)= " + std::to_string(elapsed_time) + "\n";
    os << out_str;
}

}
