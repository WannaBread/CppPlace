#include "client/MainWindow.hpp"

#include <QApplication>
#include <QFile>
#include <QUrl>
#include <QCommandLineParser>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("CppPlace");
    QApplication::setOrganizationName("CppPlace");


    QFile qss(":/styles/styles.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QString::fromUtf8(qss.readAll()));
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption serverOpt({"s", "server"},
        "Server URL (default http://127.0.0.1:8080)",
        "url", "http://127.0.0.1:8080");
    parser.addOption(serverOpt);
    parser.process(app);

    cppplace::client::MainWindow w(QUrl(parser.value(serverOpt)));
    w.show();
    return app.exec();
}
