#pragma once

#include <boost/thread/shared_mutex.hpp>
#include <chrono>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cppplace {

class CooldownManager {
public:
    explicit CooldownManager(std::chrono::duration<double> cooldown_duration);

    bool canPlace(std::string_view username) const;
    std::chrono::duration<double> getRemainingTime(std::string_view username) const;
    void recordPlacement(std::string_view username);

    std::chrono::duration<double> getCooldownDuration() const noexcept { return cooldown_duration_; }

private:
    mutable boost::shared_mutex mutex_;
    std::chrono::duration<double> cooldown_duration_;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> last_placement_;
};

} // namespace cppplace
