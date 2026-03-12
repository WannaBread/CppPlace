#include "services/PersistenceService.hpp"
#include <fstream>
#include <cstdio>

namespace cppplace {

namespace {
    constexpr uint32_t MAGIC = 0x43505043; // "CPPC"
    constexpr uint32_t VERSION = 1;
}

bool saveCanvas(const Canvas& canvas, std::string_view filepath) {
    // Write to temp file first, then rename for atomicity
    std::string filepath_str(filepath);
    std::string tmp_path = filepath_str + ".tmp";

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
    std::remove(filepath_str.c_str());
    if (std::rename(tmp_path.c_str(), filepath_str.c_str()) != 0) {
        std::remove(tmp_path.c_str());
        return false;
    }

    return true;
}

std::optional<Canvas> loadCanvas(std::string_view filepath) {
    std::ifstream ifs(std::string(filepath), std::ios::binary);
    if (!ifs.is_open()) return std::nullopt;

    // Read header
    struct Header {
        uint32_t magic;
        uint32_t version;
        uint32_t width;
        uint32_t height;
    };

    Header header;
    ifs.read(reinterpret_cast<char*>(&header), sizeof(Header));

    if (!ifs.good() || header.magic != MAGIC || header.version != VERSION) {
        return std::nullopt;
    }

    Canvas canvas(header.width, header.height);

    // Read pixel data
    for (uint32_t y = 0; y < header.height; ++y) {
        for (uint32_t x = 0; x < header.width; ++x) {
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
