#pragma once

#include <QWidget>
#include <QImage>

namespace cppplace::client {

class CanvasView : public QWidget {
    Q_OBJECT
public:
    explicit CanvasView(QWidget* parent = nullptr);

    void setScale(int scale);
    int  scale() const { return scale_; }

public slots:
    void setImage(const QImage& image);

signals:
    void pixelClicked(int x, int y);
    void scaleChanged(int scale);
    void panRequested(int dx, int dy);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QImage image_;
    int    scale_       = 4;
    int    hover_x_     = -1;
    int    hover_y_     = -1;
    bool   dragging_    = false;
    QPoint drag_last_;
};

} // namespace cppplace::client
