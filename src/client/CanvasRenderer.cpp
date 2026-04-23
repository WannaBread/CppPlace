#include "client/CanvasRenderer.hpp"

#include <QPainter>
#include <QMetaType>

namespace cppplace::client {

namespace {
    int registerVecMeta() {
        qRegisterMetaType<std::vector<uint8_t>>("std::vector<uint8_t>");
        return 0;
    }
    const int kVecMeta = registerVecMeta();
}

CanvasRenderer::CanvasRenderer(QObject* parent) : QObject(parent) {}

void CanvasRenderer::setPalette(std::vector<QColor> palette) {
    palette_ = std::move(palette);
}

QColor CanvasRenderer::colorOf(uint8_t index) const {
    if (index < palette_.size()) return palette_[index];
    return Qt::magenta;
}

void CanvasRenderer::renderCanvas(int width, int height,
                                  const std::vector<uint8_t>& pixels,
                                  int scale) {
    if (width <= 0 || height <= 0 || scale <= 0) return;
    if (static_cast<int>(pixels.size()) != width * height) return;

    width_  = width;
    height_ = height;
    scale_  = scale;
    pixels_ = pixels;

    QImage img(width * scale, height * scale, QImage::Format_RGB32);

    for (int y = 0; y < height; ++y) {
        for (int sy = 0; sy < scale; ++sy) {
            QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y * scale + sy));
            for (int x = 0; x < width; ++x) {
                const QRgb rgb = colorOf(pixels[y * width + x]).rgb();
                QRgb* dst = line + x * scale;
                for (int sx = 0; sx < scale; ++sx) dst[sx] = rgb;
            }
        }
    }

    cached_ = img;
    emit imageReady(cached_);
}

void CanvasRenderer::renderPatch(int x, int y, int colorIndex, int scale) {
    if (cached_.isNull()) return;
    if (scale != scale_) return;
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

    const QRgb rgb = colorOf(static_cast<uint8_t>(colorIndex)).rgb();
    for (int sy = 0; sy < scale; ++sy) {
        QRgb* line = reinterpret_cast<QRgb*>(cached_.scanLine(y * scale + sy));
        QRgb* dst  = line + x * scale;
        for (int sx = 0; sx < scale; ++sx) dst[sx] = rgb;
    }
    emit imageReady(cached_);
}

void CanvasRenderer::rescale(int scale) {
    if (pixels_.empty() || width_ == 0 || height_ == 0 || scale <= 0) return;
    renderCanvas(width_, height_, pixels_, scale);
}

} // namespace cppplace::client
