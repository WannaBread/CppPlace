#pragma once

#include <QWidget>
#include <QString>
#include <vector>
#include "client/ClientPalette.hpp"

class QButtonGroup;

namespace cppplace::client {

/// Horizontal strip of color swatches. Emits colorSelected(index) when the
/// user picks one. Visually styled via QSS (see resources/styles.qss).
class PaletteBar : public QWidget {
    Q_OBJECT
public:
    explicit PaletteBar(const std::vector<PaletteEntry>& palette,
                        QWidget* parent = nullptr);

    int currentColor() const { return current_; }

signals:
    void colorSelected(int index);

private:
    QButtonGroup* group_   = nullptr;
    int           current_ = 5; // start on Red — visible on white background
};

} // namespace cppplace::client
