#pragma once

#include <boost/thread/shared_mutex.hpp>
#include <chrono>
#include <string>
#include <unordered_map>

namespace cppplace {

class CooldownManager {
public:
    explicit CooldownManager(std::chrono::seconds cooldown_duration);

    bool canPlace(const std::string& username) const;
    std::chrono::seconds getRemainingTime(const std::string& username) const;
    void recordPlacement(const std::string& username);

    std::chrono::seconds getCooldownDuration() const { return cooldown_duration_; }

private:
    mutable boost::shared_mutex mutex_;
    std::chrono::seconds cooldown_duration_;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> last_placement_;
};

} // namespace cppplace
