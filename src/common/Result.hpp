#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <stdexcept>

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

inline std::string_view errorCodeToString(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::Success:            return "Success";
        case ErrorCode::InvalidCoordinates: return "Invalid coordinates";
        case ErrorCode::InvalidColor:       return "Invalid color";
        case ErrorCode::CooldownActive:     return "Cooldown active";
        case ErrorCode::Unauthorized:       return "Unauthorized";
        case ErrorCode::UsernameTaken:      return "Username already taken";
        case ErrorCode::InvalidCredentials: return "Invalid credentials";
        case ErrorCode::InternalError:      return "Internal error";
        default:                            return "Unknown error";
    }
}

class CppPlaceError : public std::runtime_error {
public:
    CppPlaceError(ErrorCode code, const std::string& message)
        : std::runtime_error(message), code_(code) {}

    ErrorCode code() const noexcept { return code_; }

    struct RpcError {
        ErrorCode code;
        std::string_view code_string;
        std::string message;
    };

    RpcError toRpcError() const {
        return { code_, errorCodeToString(code_), what() };
    }

private:
    ErrorCode code_;
};

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

    bool ok() const noexcept { return code_ == ErrorCode::Success; }
    ErrorCode code() const noexcept { return code_; }
    std::string_view message() const noexcept { return message_; }

    const T& value() const { return value_.value(); }
    T& value() { return value_.value(); }

    CppPlaceError toError() const {
        return CppPlaceError(code_, std::string(message_));
    }

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

    bool ok() const noexcept { return code_ == ErrorCode::Success; }
    ErrorCode code() const noexcept { return code_; }
    std::string_view message() const noexcept { return message_; }

    CppPlaceError toError() const {
        return CppPlaceError(code_, std::string(message_));
    }

private:
    ErrorCode code_ = ErrorCode::Success;
    std::string message_;
};

} // namespace cppplace
