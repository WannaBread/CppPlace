#pragma once

#include <string>
#include <string_view>
#include <boost/uuid/detail/sha1.hpp>
#include <sstream>
#include <iomanip>
#include <cstdint>

namespace cppplace {

class PasswordHasher {
public:
    static inline std::string hash(std::string_view password) {
        boost::uuids::detail::sha1 sha;
        sha.process_bytes(password.data(), password.size());

        boost::uuids::detail::sha1::digest_type digest;
        sha.get_digest(digest);

        std::ostringstream oss;
        for (int i = 0; i < 20; ++i) {
            oss << std::hex << std::setfill('0') << std::setw(2)
                << static_cast<unsigned>(digest[i]);
        }
        return oss.str();
    }

    static inline bool verify(std::string_view password, std::string_view stored_hash) {
        return hash(password) == stored_hash;
    }
};

} // namespace cppplace
