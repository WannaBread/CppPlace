#include "services/PersistenceService.hpp"
#include <fstream>
#include <cstdio>

namespace cppplace {

namespace {
    const uint32_t MAGIC = 0x43505043; // "CPPC"
    const uint32_t VERSION = 1;
}

bool PersistenceService::saveCanvas(const Canvas& canvas, const std::string& filepath) {
    // Write to temp file first, then rename for atomicity
    std::string tmp_path = filepath + ".tmp";

    {
        std::ofstream ofs(tmp_path, std::ios::binary);
        if (!ofs.is_open()) return false;

        // Header
        uint32_t width = static_cast<uint32_t>(canvas.getWidth());
        uint32_t height = static_cast<uint32_t>(canvas.getHeight());
        ofs.write(reinterpret_cast<const char*>(&MAGIC), sizeof(MAGIC));
        ofs.write(reinterpret_cast<const char*>(&VERSION), sizeof(VERSION));
        ofs.write(reinterpret_cast<const char*>(&width), sizeof(width));
        ofs.write(reinterpret_cast<const char*>(&height), sizeof(height));

        // Pixel data
        auto snapshot = canvas.getSnapshot();
        for (const auto& pixel : snapshot) {
            ofs.write(reinterpret_cast<const char*>(&pixel.color_index), sizeof(pixel.color_index));

            uint16_t user_len = static_cast<uint16_t>(pixel.user_id.size());
            ofs.write(reinterpret_cast<const char*>(&user_len), sizeof(user_len));
            ofs.write(pixel.user_id.data(), user_len);
        }

        if (!ofs.good()) {
            std::remove(tmp_path.c_str());
            return false;
        }
    }

    // Atomic rename
    std::remove(filepath.c_str());
    if (std::rename(tmp_path.c_str(), filepath.c_str()) != 0) {
        std::remove(tmp_path.c_str());
        return false;
    }

    return true;
}

std::optional<Canvas> PersistenceService::loadCanvas(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open()) return std::nullopt;

    // Read header
    uint32_t magic, version, width, height;
    ifs.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    ifs.read(reinterpret_cast<char*>(&version), sizeof(version));
    ifs.read(reinterpret_cast<char*>(&width), sizeof(width));
    ifs.read(reinterpret_cast<char*>(&height), sizeof(height));

    if (!ifs.good() || magic != MAGIC || version != VERSION) {
        return std::nullopt;
    }

    Canvas canvas(width, height);

    // Read pixel data
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint8_t color_index;
            ifs.read(reinterpret_cast<char*>(&color_index), sizeof(color_index));

            uint16_t user_len;
            ifs.read(reinterpret_cast<char*>(&user_len), sizeof(user_len));

            std::string user_id(user_len, '\0');
            ifs.read(user_id.data(), user_len);

            if (!ifs.good()) return std::nullopt;

            canvas.setPixel(x, y, color_index, user_id);
        }
    }

    return canvas;
}

} // namespace cppplace
