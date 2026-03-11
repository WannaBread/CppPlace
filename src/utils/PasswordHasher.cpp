#include "utils/PasswordHasher.hpp"
#include <boost/uuid/detail/sha1.hpp>
#include <sstream>
#include <iomanip>
#include <cstdint>

namespace cppplace {

std::string PasswordHasher::hash(const std::string& password) {
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

bool PasswordHasher::verify(const std::string& password, const std::string& stored_hash) {
    return hash(password) == stored_hash;
}

} // namespace cppplace
