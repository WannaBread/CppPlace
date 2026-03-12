#pragma once

#include <boost/thread/shared_mutex.hpp>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>

namespace cppplace {

class SessionManager {
public:
    std::string createSession(std::string_view username);
    std::optional<std::string> validateSession(std::string_view token) const;
    bool removeSession(std::string_view token);
    size_t activeSessionCount() const;

private:
    mutable boost::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> token_to_user_; // token -> username
};

} // namespace cppplace
