#include <QApplication>

int runCanvasRendererTests(int argc, char** argv);
int runNetworkWorkerTests (int argc, char** argv);

int main(int argc, char** argv) {

    QApplication app(argc, argv);

    int rc = 0;
    rc |= runCanvasRendererTests(argc, argv);
    rc |= runNetworkWorkerTests (argc, argv);
    return rc;
}
