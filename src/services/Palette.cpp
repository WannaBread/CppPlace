#include "services/Palette.hpp"
#include <stdexcept>

namespace cppplace {

Palette::Palette(std::vector<Color> colors) : colors_(std::move(colors)) {
    if (colors_.empty()) {
        throw std::invalid_argument("Palette must contain at least one color");
    }
}

bool Palette::isValidColor(uint8_t index) const noexcept {
    return index < colors_.size();
}

const Color& Palette::getColor(uint8_t index) const {
    if (!isValidColor(index)) {
        throw std::out_of_range("Color index out of range");
    }
    return colors_[index];
}

size_t Palette::size() const noexcept {
    return colors_.size();
}

const std::vector<Color>& Palette::getColors() const noexcept {
    return colors_;
}

Palette Palette::createDefault() {
    return Palette({
        {255, 255, 255, "White"},
        {228, 228, 228, "Light Gray"},
        {136, 136, 136, "Gray"},
        {34,  34,  34,  "Black"},
        {255, 167, 209, "Pink"},
        {229, 0,   0,   "Red"},
        {229, 149, 0,   "Orange"},
        {160, 106, 66,  "Brown"},
        {229, 217, 0,   "Yellow"},
        {148, 224, 68,  "Light Green"},
        {2,   190, 1,   "Green"},
        {0,   211, 221, "Cyan"},
        {0,   131, 199, "Blue"},
        {0,   0,   234, "Dark Blue"},
        {207, 110, 228, "Purple"},
        {130, 0,   128, "Dark Purple"}
    });
}

}
