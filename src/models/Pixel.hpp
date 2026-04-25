#pragma once

#include <cstdint>
#include <string>
#include <chrono>

namespace cppplace {

struct Pixel {
    uint8_t color_index = 0;
    std::string user_id;
    std::chrono::system_clock::time_point timestamp{};

    bool operator==(const Pixel& other) const {
        return color_index == other.color_index && user_id == other.user_id;
    }
};

} // namespace cppplace
