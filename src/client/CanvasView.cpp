#include "client/CanvasView.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <algorithm>

namespace cppplace::client {

CanvasView::CanvasView(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);
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
    if (image_.isNull()) return;
    p.drawImage(0, 0, image_);

    if (hover_x_ >= 0 && hover_y_ >= 0) {
        const int px  = hover_x_ * scale_;
        const int py  = hover_y_ * scale_;
        const int s   = scale_;
        const int arm = std::max(2, std::min(s / 3 + 1, 6));

        // Draw black shadow first (1px outward), then white on top
        for (int pass = 0; pass < 2; ++pass) {
            const int o = (pass == 0) ? 1 : 0;
            p.setPen(QPen(pass == 0 ? QColor(0,0,0) : QColor(255,255,255), 1));

            // top-left
            p.drawLine(px-o,       py-o,       px+arm,     py-o      );
            p.drawLine(px-o,       py-o,       px-o,       py+arm    );
            // top-right
            p.drawLine(px+s-1+o,   py-o,       px+s-1-arm, py-o      );
            p.drawLine(px+s-1+o,   py-o,       px+s-1+o,   py+arm    );
            // bottom-left
            p.drawLine(px-o,       py+s-1+o,   px+arm,     py+s-1+o  );
            p.drawLine(px-o,       py+s-1+o,   px-o,       py+s-1-arm);
            // bottom-right
            p.drawLine(px+s-1+o,   py+s-1+o,   px+s-1-arm, py+s-1+o  );
            p.drawLine(px+s-1+o,   py+s-1+o,   px+s-1+o,   py+s-1-arm);
        }
    }
}

void CanvasView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        dragging_ = true;
        drag_last_ = event->globalPosition().toPoint();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    if (event->button() != Qt::LeftButton || image_.isNull()) return;
    const int x = event->position().x() / scale_;
    const int y = event->position().y() / scale_;
    if (x < 0 || y < 0) return;
    if (x >= image_.width() / scale_ || y >= image_.height() / scale_) return;
    emit pixelClicked(x, y);
}

void CanvasView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton && dragging_) {
        dragging_ = false;
        unsetCursor();
        event->accept();
    }
}

void CanvasView::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_) {
        const QPoint pos   = event->globalPosition().toPoint();
        const QPoint delta = pos - drag_last_;
        drag_last_ = pos;
        emit panRequested(-delta.x(), -delta.y());
        return;
    }
    if (image_.isNull() || scale_ <= 0) return;
    const int hx = static_cast<int>(event->position().x()) / scale_;
    const int hy = static_cast<int>(event->position().y()) / scale_;
    const int cw = image_.width()  / scale_;
    const int ch = image_.height() / scale_;
    const int nx = (hx >= 0 && hx < cw) ? hx : -1;
    const int ny = (hy >= 0 && hy < ch) ? hy : -1;
    if (nx != hover_x_ || ny != hover_y_) {
        hover_x_ = nx;
        hover_y_ = ny;
        update();
    }
}

void CanvasView::leaveEvent(QEvent*) {
    hover_x_ = -1;
    hover_y_ = -1;
    update();
}

void CanvasView::wheelEvent(QWheelEvent* event) {
    if (!(event->modifiers() & Qt::ControlModifier)) {
        event->ignore();
        return;
    }
    const int delta    = event->angleDelta().y();
    const int newScale = std::clamp(scale_ + (delta > 0 ? 1 : -1), 1, 32);
    if (newScale != scale_) {
        scale_ = newScale;
        emit scaleChanged(scale_);
    }
    event->accept();
}

} // namespace cppplace::client
