#include <QtTest/QtTest>
#include <QThread>
#include <QSignalSpy>
#include <QImage>
#include <QColor>

#include "client/CanvasRenderer.hpp"
#include "client/ClientPalette.hpp"

#include <vector>
#include <cstdint>

using cppplace::client::CanvasRenderer;
using cppplace::client::defaultPaletteColors;

class TestCanvasRenderer : public QObject {
    Q_OBJECT

private slots:
    /// CanvasRenderer must actually do its painting on the worker thread, not
    /// the GUI thread — that's the whole point. We send it a render request
    /// via a queued connection and check thread() inside renderCanvas via the
    /// signal callback.
    void rendersOnWorkerThread() {
        QThread thread;
        CanvasRenderer renderer;
        renderer.setPalette(defaultPaletteColors());
        renderer.moveToThread(&thread);
        thread.start();

        QThread* observed = nullptr;
        connect(&renderer, &CanvasRenderer::imageReady, this,
            [&](const QImage&) {
                observed = QThread::currentThread();
            }, Qt::DirectConnection);

        QSignalSpy spy(&renderer, &CanvasRenderer::imageReady);

        std::vector<uint8_t> px(4, 5); // 2x2, all "Red"
        QMetaObject::invokeMethod(&renderer, "renderCanvas", Qt::QueuedConnection,
            Q_ARG(int, 2), Q_ARG(int, 2),
            Q_ARG(std::vector<uint8_t>, px), Q_ARG(int, 1));

        QVERIFY(spy.wait(2000));

        QCOMPARE(observed, &thread);
        QVERIFY(observed != QThread::currentThread());

        thread.quit();
        thread.wait();
    }

    /// Pixel array → QImage mapping. Position (x, y) → palette index → QColor.
    void mapsPaletteCorrectly() {
        CanvasRenderer renderer;
        const auto pal = defaultPaletteColors();
        renderer.setPalette(pal);

        std::vector<uint8_t> px = {
            5, 0,    // Red,        White
            3, 10,   // Black,      Green
        };

        QImage out;
        connect(&renderer, &CanvasRenderer::imageReady, this,
            [&](const QImage& img) { out = img; }, Qt::DirectConnection);

        renderer.renderCanvas(2, 2, px, 1);

        QCOMPARE(out.size(), QSize(2, 2));
        QCOMPARE(QColor(out.pixel(0, 0)), pal[5]);
        QCOMPARE(QColor(out.pixel(1, 0)), pal[0]);
        QCOMPARE(QColor(out.pixel(0, 1)), pal[3]);
        QCOMPARE(QColor(out.pixel(1, 1)), pal[10]);
    }

    /// Scale factor: each logical pixel becomes a `scale x scale` block.
    void scalesPixels() {
        CanvasRenderer renderer;
        renderer.setPalette(defaultPaletteColors());

        QImage out;
        connect(&renderer, &CanvasRenderer::imageReady, this,
            [&](const QImage& img) { out = img; }, Qt::DirectConnection);

        std::vector<uint8_t> px = {5, 0, 3, 10};
        renderer.renderCanvas(2, 2, px, 4);

        QCOMPARE(out.size(), QSize(8, 8));
        // Top-left 4x4 block should all be "Red"
        const QColor red = defaultPaletteColors()[5];
        for (int y = 0; y < 4; ++y)
            for (int x = 0; x < 4; ++x)
                QCOMPARE(QColor(out.pixel(x, y)), red);
    }

    /// renderPatch updates a single pixel in the cached image without
    /// re-rendering the whole canvas.
    void patchesSinglePixel() {
        CanvasRenderer renderer;
        renderer.setPalette(defaultPaletteColors());

        QImage out;
        connect(&renderer, &CanvasRenderer::imageReady, this,
            [&](const QImage& img) { out = img; }, Qt::DirectConnection);

        std::vector<uint8_t> px(4, 0); // all white
        renderer.renderCanvas(2, 2, px, 1);
        QCOMPARE(QColor(out.pixel(1, 1)), defaultPaletteColors()[0]);

        renderer.renderPatch(1, 1, 5, 1); // turn (1,1) red
        QCOMPARE(QColor(out.pixel(1, 1)), defaultPaletteColors()[5]);
        QCOMPARE(QColor(out.pixel(0, 0)), defaultPaletteColors()[0]); // untouched
    }

    /// Bad input should not crash or emit an image.
    void rejectsBadInput() {
        CanvasRenderer renderer;
        renderer.setPalette(defaultPaletteColors());
        QSignalSpy spy(&renderer, &CanvasRenderer::imageReady);

        renderer.renderCanvas(0, 0, {}, 1);             // zero size
        renderer.renderCanvas(2, 2, {1, 2, 3}, 1);      // size mismatch
        renderer.renderCanvas(2, 2, {0, 0, 0, 0}, 0);   // zero scale

        QCOMPARE(spy.count(), 0);
    }
};

#include "test_canvas_renderer.moc"


int runCanvasRendererTests(int argc, char** argv) {
    TestCanvasRenderer t;
    return QTest::qExec(&t, argc, argv);
}
