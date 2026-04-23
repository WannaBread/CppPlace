#pragma once

#include <QWidget>
#include <QString>
#include <vector>
#include "client/ClientPalette.hpp"

class QButtonGroup;

namespace cppplace::client {

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
    int           current_ = 5;
};

} // namespace cppplace::client
