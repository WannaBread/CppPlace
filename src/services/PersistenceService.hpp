#pragma once

#include "models/Canvas.hpp"
#include <string>
#include <string_view>
#include <optional>

namespace cppplace {

bool saveCanvas(const Canvas& canvas, std::string_view filepath);
std::optional<Canvas> loadCanvas(std::string_view filepath);

} 