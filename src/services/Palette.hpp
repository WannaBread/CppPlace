#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cppplace {

struct Color {
    uint8_t r, g, b;
    std::string name;

    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
};

class Palette {
public:
    explicit Palette(std::vector<Color> colors);

    bool isValidColor(uint8_t index) const noexcept;
    const Color& getColor(uint8_t index) const;
    size_t size() const noexcept;
    const std::vector<Color>& getColors() const noexcept;

    static Palette createDefault();

private:
    std::vector<Color> colors_;
};

} // namespace cppplace
