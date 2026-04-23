#pragma once

#include <QObject>
#include <QImage>
#include <QColor>
#include <vector>
#include <cstdint>

namespace cppplace::client {

/// Lives in its own QThread. Receives raw pixel data + palette, builds a
/// QImage off the GUI thread, and emits it. The GUI thread only ever blits a
/// finished image, so even very large canvases never freeze the UI.
class CanvasRenderer : public QObject {
    Q_OBJECT
public:
    explicit CanvasRenderer(QObject* parent = nullptr);

    /// Set the palette colours (index → QColor). Safe to call from the GUI
    /// thread because the slot is invoked via queued connection.
    void setPalette(std::vector<QColor> palette);

public slots:
    /// Render a full snapshot. `scale` is an integer pixel-size multiplier
    /// (1 means one logical pixel == one screen pixel).
    void renderCanvas(int width, int height,
                      const std::vector<uint8_t>& pixels, int scale);

    /// Patch an already-rendered image (avoids a full redraw on every
    /// single-pixel update).
    void renderPatch(int x, int y, int colorIndex, int scale);

signals:
    void imageReady(const QImage& image);

private:
    QColor colorOf(uint8_t index) const;

    std::vector<QColor> palette_;
    QImage              cached_;     // last rendered image, used by renderPatch
    int                 scale_ = 1;
    int                 width_ = 0;
    int                 height_ = 0;
};

} // namespace cppplace::client
