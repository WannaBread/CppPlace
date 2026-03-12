#include "models/Canvas.hpp"
#include <chrono>

namespace cppplace {

Canvas::Canvas(size_t width, size_t height)
    : width_(width), height_(height), pixels_(width * height) {}

void Canvas::setPixel(size_t x, size_t y, uint8_t color_index, std::string_view user_id) {
    auto& pixel = pixels_[index(x, y)];
    pixel.color_index = color_index;
    pixel.user_id = std::string(user_id);
    pixel.timestamp = std::chrono::system_clock::now();
}

const Pixel& Canvas::getPixel(size_t x, size_t y) const {
    return pixels_[index(x, y)];
}

bool Canvas::isValidCoord(size_t x, size_t y) const noexcept {
    return x < width_ && y < height_;
}

std::vector<Pixel> Canvas::getSnapshot() const {
    return pixels_;
}

size_t Canvas::index(size_t x, size_t y) const noexcept {
    return y * width_ + x;
}

}
