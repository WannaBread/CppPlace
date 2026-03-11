#pragma once

#include "models/Canvas.hpp"
#include <string>
#include <optional>

namespace cppplace {

class PersistenceService {
public:
    static bool saveCanvas(const Canvas& canvas, const std::string& filepath);
    static std::optional<Canvas> loadCanvas(const std::string& filepath);
};

} // namespace cppplace
