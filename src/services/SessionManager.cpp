#include "services/SessionManager.hpp"
#include "utils/TokenGenerator.hpp"

namespace cppplace {

std::string SessionManager::createSession(std::string_view username) {
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    std::string token = TokenGenerator::generate();
    token_to_user_[token] = std::string(username);
    return token;
}

std::optional<std::string> SessionManager::validateSession(std::string_view token) const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    auto it = token_to_user_.find(std::string(token));
    if (it == token_to_user_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool SessionManager::removeSession(std::string_view token) {
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    return token_to_user_.erase(std::string(token)) > 0;
}

size_t SessionManager::activeSessionCount() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    return token_to_user_.size();
}

} // namespace cppplace
