#pragma once

#include "Pixel.hpp"
#include <vector>
#include <cstddef>
#include <string_view>

namespace cppplace {

class Canvas {
public:
    Canvas(size_t width, size_t height);

    void setPixel(size_t x, size_t y, uint8_t color_index, std::string_view user_id);
    const Pixel& getPixel(size_t x, size_t y) const;

    bool isValidCoord(size_t x, size_t y) const noexcept;

    size_t getWidth() const noexcept { return width_; }
    size_t getHeight() const noexcept { return height_; }

    std::vector<Pixel> getSnapshot() const;

private:
    size_t index(size_t x, size_t y) const noexcept;

    size_t width_;
    size_t height_;
    std::vector<Pixel> pixels_;
};

} // namespace cppplace
