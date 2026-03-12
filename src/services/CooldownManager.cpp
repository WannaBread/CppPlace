#include "services/CooldownManager.hpp"

namespace cppplace {

CooldownManager::CooldownManager(std::chrono::duration<double> cooldown_duration)
    : cooldown_duration_(cooldown_duration) {}

bool CooldownManager::canPlace(std::string_view username) const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    auto it = last_placement_.find(std::string(username));
    if (it == last_placement_.end()) {
        return true;
    }
    auto elapsed = std::chrono::system_clock::now() - it->second;
    return elapsed >= cooldown_duration_;
}

std::chrono::duration<double> CooldownManager::getRemainingTime(std::string_view username) const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    auto it = last_placement_.find(std::string(username));
    if (it == last_placement_.end()) {
        return std::chrono::duration<double>(0.0);
    }
    auto elapsed = std::chrono::system_clock::now() - it->second;
    auto remaining = cooldown_duration_ - elapsed;
    if (remaining.count() <= 0.0) {
        return std::chrono::duration<double>(0.0);
    }
    return remaining;
}

void CooldownManager::recordPlacement(std::string_view username) {
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    last_placement_[std::string(username)] = std::chrono::system_clock::now();
}

} 
