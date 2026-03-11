#pragma once

#include <boost/thread/shared_mutex.hpp>
#include <unordered_map>
#include <string>
#include <optional>

namespace cppplace {

class SessionManager {
public:
    std::string createSession(const std::string& username);
    std::optional<std::string> validateSession(const std::string& token) const;
    bool removeSession(const std::string& token);
    size_t activeSessionCount() const;

private:
    mutable boost::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> token_to_user_; // token -> username
};

} // namespace cppplace
