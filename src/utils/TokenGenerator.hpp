#pragma once

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

namespace cppplace {

class TokenGenerator {
public:
    static inline std::string generate() {
        static thread_local boost::uuids::random_generator gen;
        boost::uuids::uuid uuid = gen();
        std::ostringstream oss;
        oss << uuid;
        return oss.str();
    }
};

}