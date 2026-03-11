#include "services/SessionManager.hpp"
#include "utils/TokenGenerator.hpp"

namespace cppplace {

std::string SessionManager::createSession(const std::string& username) {
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    std::string token = TokenGenerator::generate();
    token_to_user_[token] = username;
    return token;
}

std::optional<std::string> SessionManager::validateSession(const std::string& token) const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    auto it = token_to_user_.find(token);
    if (it == token_to_user_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool SessionManager::removeSession(const std::string& token) {
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    return token_to_user_.erase(token) > 0;
}

size_t SessionManager::activeSessionCount() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    return token_to_user_.size();
}

} // namespace cppplace
