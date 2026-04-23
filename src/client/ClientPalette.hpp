#pragma once

#include <QColor>
#include <QString>
#include <vector>

namespace cppplace::client {

struct PaletteEntry {
    QColor  color;
    QString name;
};

inline std::vector<PaletteEntry> defaultPalette() {
    return {
        {{255, 255, 255}, "White"},
        {{228, 228, 228}, "Light Gray"},
        {{136, 136, 136}, "Gray"},
        {{34,  34,  34},  "Black"},
        {{255, 167, 209}, "Pink"},
        {{229, 0,   0},   "Red"},
        {{229, 149, 0},   "Orange"},
        {{160, 106, 66},  "Brown"},
        {{229, 217, 0},   "Yellow"},
        {{148, 224, 68},  "Light Green"},
        {{2,   190, 1},   "Green"},
        {{0,   211, 221}, "Cyan"},
        {{0,   131, 199}, "Blue"},
        {{0,   0,   234}, "Dark Blue"},
        {{207, 110, 228}, "Purple"},
        {{130, 0,   128}, "Dark Purple"}
    };
}

inline std::vector<QColor> defaultPaletteColors() {
    std::vector<QColor> out;
    for (const auto& e : defaultPalette()) out.push_back(e.color);
    return out;
}

} // namespace cppplace::client
