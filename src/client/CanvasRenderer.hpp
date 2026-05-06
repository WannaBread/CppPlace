#pragma once

#include <QObject>
#include <QImage>
#include <QColor>
#include <vector>
#include <cstdint>

namespace cppplace::client {

class CanvasRenderer : public QObject {
    Q_OBJECT
public:
    explicit CanvasRenderer(QObject* parent = nullptr);

    void setPalette(std::vector<QColor> palette);

public slots:
    void renderCanvas(int width, int height,
                      const std::vector<uint8_t>& pixels, int scale);
    void renderPatch(int x, int y, int colorIndex, int scale);
    void rescale(int scale);

signals:
    void imageReady(const QImage& image);

private:
    QColor colorOf(uint8_t index) const;

    std::vector<QColor>   palette_;
    QImage                cached_;
    std::vector<uint8_t>  pixels_;
    int                   scale_  = 1;
    int                   width_  = 0;
    int                   height_ = 0;
};

} // namespace cppplace::client
