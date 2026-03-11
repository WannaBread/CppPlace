#include "services/CooldownManager.hpp"

namespace cppplace {

CooldownManager::CooldownManager(std::chrono::seconds cooldown_duration)
    : cooldown_duration_(cooldown_duration) {}

bool CooldownManager::canPlace(const std::string& username) const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    auto it = last_placement_.find(username);
    if (it == last_placement_.end()) {
        return true;
    }
    auto elapsed = std::chrono::system_clock::now() - it->second;
    return elapsed >= cooldown_duration_;
}

std::chrono::seconds CooldownManager::getRemainingTime(const std::string& username) const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    auto it = last_placement_.find(username);
    if (it == last_placement_.end()) {
        return std::chrono::seconds(0);
    }
    auto elapsed = std::chrono::system_clock::now() - it->second;
    auto remaining = cooldown_duration_ - std::chrono::duration_cast<std::chrono::seconds>(elapsed);
    if (remaining.count() <= 0) {
        return std::chrono::seconds(0);
    }
    return std::chrono::duration_cast<std::chrono::seconds>(remaining);
}

void CooldownManager::recordPlacement(const std::string& username) {
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    last_placement_[username] = std::chrono::system_clock::now();
}

} // namespace cppplace
