#pragma once

#include "models/User.hpp"
#include "common/Result.hpp"
#include "utils/PasswordHasher.hpp"

#include <boost/thread/shared_mutex.hpp>
#include <unordered_map>
#include <string>

namespace cppplace {

class UserStore {
public:
    Result<void> registerUser(const std::string& username, const std::string& password);
    Result<std::string> authenticate(const std::string& username, const std::string& password);

    bool userExists(const std::string& username) const;
    size_t userCount() const;

private:
    mutable boost::shared_mutex mutex_;
    std::unordered_map<std::string, User> users_;
};

} // namespace cppplace
