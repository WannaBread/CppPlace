#include "services/UserStore.hpp"

namespace cppplace {

Result<void> UserStore::registerUser(std::string_view username, std::string_view password) {
    if (username.empty() || password.empty()) {
        return Result<void>::failure(ErrorCode::InvalidCredentials, "Username and password must not be empty");
    }

    boost::unique_lock<boost::shared_mutex> lock(mutex_);

    if (users_.count(std::string(username))) {
        return Result<void>::failure(ErrorCode::UsernameTaken, "Username already exists");
    }

    User user;
    user.username = std::string(username);
    user.password_hash = PasswordHasher::hash(password);
    users_[user.username] = std::move(user);

    return Result<void>::success();
}

Result<std::string> UserStore::authenticate(std::string_view username, std::string_view password) {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);

    auto it = users_.find(std::string(username));
    if (it == users_.end()) {
        return Result<std::string>::failure(ErrorCode::InvalidCredentials, "User not found");
    }

    if (!PasswordHasher::verify(password, it->second.password_hash)) {
        return Result<std::string>::failure(ErrorCode::InvalidCredentials, "Invalid password");
    }

    return Result<std::string>::success(std::string(username));
}

bool UserStore::userExists(std::string_view username) const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    return users_.count(std::string(username)) > 0;
}

size_t UserStore::userCount() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    return users_.size();
}

} // namespace cppplace
