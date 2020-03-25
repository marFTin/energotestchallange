#pragma once

#include <chrono>
#include <cinttypes>
#include <string>

namespace Challenge {

    struct EventData {
        std::chrono::time_point<std::chrono::system_clock> timeStamp;
        std::string text;
        uint32_t priority;
    };

} // namespace Challenge
