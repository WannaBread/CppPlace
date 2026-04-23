#pragma once

#include "models/User.hpp"
#include "common/Result.hpp"
#include "utils/PasswordHasher.hpp"

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_types.hpp>
#include <unordered_map>
#include <string>
#include <string_view>

namespace cppplace {

class UserStore {
public:
    Result<void> registerUser(std::string_view username, std::string_view password);
    Result<std::string> authenticate(std::string_view username, std::string_view password);

    bool userExists(std::string_view username) const;
    size_t userCount() const;

private:
    mutable boost::shared_mutex mutex_;
    std::unordered_map<std::string, User> users_;
};

}
