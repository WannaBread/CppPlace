#pragma once

#include <QWidget>
#include <QImage>

namespace cppplace::client {

/// Pure-display widget. Draws whatever QImage CanvasRenderer hands it and
/// translates mouse clicks into (x, y) canvas coordinates. No threading, no
/// network — those live elsewhere.
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

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QImage image_;
    int    scale_ = 4;
};

} // namespace cppplace::client
