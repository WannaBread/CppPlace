#include <QApplication>

// Each TU defines its own runner — we compose them here so the whole client
// test surface lives in one binary.
int runCanvasRendererTests(int argc, char** argv);
int runNetworkWorkerTests (int argc, char** argv);

int main(int argc, char** argv) {
    // QApplication (not QCoreApplication) — CanvasRenderer creates QImage,
    // and some Qt image paths require QGuiApplication; QApplication covers
    // that and lets the network tests use the default event loop.
    QApplication app(argc, argv);

    int rc = 0;
    rc |= runCanvasRendererTests(argc, argv);
    rc |= runNetworkWorkerTests (argc, argv);
    return rc;
}
