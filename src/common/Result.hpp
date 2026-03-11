#pragma once

#include <string>
#include <optional>

namespace cppplace {

enum class ErrorCode {
    Success,
    InvalidCoordinates,
    InvalidColor,
    CooldownActive,
    Unauthorized,
    UsernameTaken,
    InvalidCredentials,
    InternalError
};

inline std::string errorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::Success:            return "Success";
        case ErrorCode::InvalidCoordinates: return "Invalid coordinates";
        case ErrorCode::InvalidColor:       return "Invalid color";
        case ErrorCode::CooldownActive:     return "Cooldown active";
        case ErrorCode::Unauthorized:       return "Unauthorized";
        case ErrorCode::UsernameTaken:      return "Username already taken";
        case ErrorCode::InvalidCredentials: return "Invalid credentials";
        case ErrorCode::InternalError:      return "Internal error";
    }
    return "Unknown error";
}

template <typename T>
class Result {
public:
    static Result success(T value) {
        Result r;
        r.value_ = std::move(value);
        r.code_ = ErrorCode::Success;
        return r;
    }

    static Result failure(ErrorCode code, std::string message = "") {
        Result r;
        r.code_ = code;
        r.message_ = std::move(message);
        return r;
    }

    bool ok() const { return code_ == ErrorCode::Success; }
    ErrorCode code() const { return code_; }
    const std::string& message() const { return message_; }

    const T& value() const { return value_.value(); }
    T& value() { return value_.value(); }

private:
    std::optional<T> value_;
    ErrorCode code_ = ErrorCode::Success;
    std::string message_;
};

// Specialization for void
template <>
class Result<void> {
public:
    static Result success() {
        Result r;
        r.code_ = ErrorCode::Success;
        return r;
    }

    static Result failure(ErrorCode code, std::string message = "") {
        Result r;
        r.code_ = code;
        r.message_ = std::move(message);
        return r;
    }

    bool ok() const { return code_ == ErrorCode::Success; }
    ErrorCode code() const { return code_; }
    const std::string& message() const { return message_; }

private:
    ErrorCode code_ = ErrorCode::Success;
    std::string message_;
};

} // namespace cppplace
