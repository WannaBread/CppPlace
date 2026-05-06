#include "client/PaletteBar.hpp"

#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

namespace cppplace::client {

PaletteBar::PaletteBar(const std::vector<PaletteEntry>& palette, QWidget* parent)
    : QWidget(parent), group_(new QButtonGroup(this)) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(8, 6, 8, 6);
    lay->setSpacing(4);

    group_->setExclusive(true);

    for (size_t i = 0; i < palette.size(); ++i) {
        const auto& entry = palette[i];
        auto* btn = new QPushButton(this);
        btn->setObjectName("paletteSwatch");
        btn->setCheckable(true);
        btn->setFixedSize(28, 28);
        btn->setToolTip(entry.name);

        btn->setStyleSheet(QString(
            "QPushButton#paletteSwatch {"
            "  background-color: %1;"
            "  border: 2px solid #2a2a32;"
            "  border-radius: 6px;"
            "}"
            "QPushButton#paletteSwatch:hover { border-color: #888; }"
            "QPushButton#paletteSwatch:checked { border-color: #f0c43a; }"
        ).arg(entry.color.name()));

        group_->addButton(btn, static_cast<int>(i));
        lay->addWidget(btn);
    }

    if (auto* btn = group_->button(current_)) btn->setChecked(true);

    connect(group_, &QButtonGroup::idClicked, this, [this](int id) {
        current_ = id;
        emit colorSelected(id);
    });

    lay->addStretch();
}

} // namespace cppplace::client
