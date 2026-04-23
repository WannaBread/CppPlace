#include "client/CanvasView.hpp"

#include <QPainter>
#include <QMouseEvent>

namespace cppplace::client {

CanvasView::CanvasView(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(false);
    setCursor(Qt::CrossCursor);
}

void CanvasView::setScale(int scale) {
    if (scale < 1) scale = 1;
    scale_ = scale;
}

void CanvasView::setImage(const QImage& image) {
    image_ = image;
    setMinimumSize(image_.size());
    resize(image_.size());
    update();
}

void CanvasView::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor(24, 24, 28));
    if (!image_.isNull()) p.drawImage(0, 0, image_);
}

void CanvasView::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton || image_.isNull()) return;
    const int x = event->position().x() / scale_;
    const int y = event->position().y() / scale_;
    if (x < 0 || y < 0) return;
    if (x >= image_.width() / scale_ || y >= image_.height() / scale_) return;
    emit pixelClicked(x, y);
}

} // namespace cppplace::client
