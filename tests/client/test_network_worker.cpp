#include <QtTest/QtTest>
#include <QThread>
#include <QSignalSpy>
#include <QCoreApplication>
#include <QUrl>

#include "client/NetworkWorker.hpp"

#include "server/HttpServer.hpp"
#include "server/RequestHandler.hpp"
#include "services/UserStore.hpp"
#include "services/SessionManager.hpp"
#include "services/CanvasService.hpp"
#include "services/CooldownManager.hpp"
#include "services/EventBus.hpp"
#include "services/Palette.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <chrono>
#include <memory>
#include <thread>

using cppplace::client::NetworkWorker;

namespace {


class ServerFixture {
public:
    ServerFixture() {
        user_store_  = std::make_shared<cppplace::UserStore>();
        sessions_    = std::make_shared<cppplace::SessionManager>();
        cooldowns_   = std::make_shared<cppplace::CooldownManager>(
                          std::chrono::duration<double>(0.0)); // no cooldown
        bus_         = std::make_shared<cppplace::EventBus>();
        palette_     = std::make_shared<cppplace::Palette>(
                          cppplace::Palette::createDefault());
        canvas_svc_  = std::make_shared<cppplace::CanvasService>(
                          16, 16, palette_, sessions_, cooldowns_, bus_);
        handler_     = std::make_shared<cppplace::RequestHandler>(
                          user_store_, sessions_, canvas_svc_);

        ioc_      = std::make_unique<boost::asio::io_context>();
        server_   = std::make_unique<cppplace::HttpServer>(
            *ioc_,
            boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0),
            handler_);
        port_     = server_->port();
        thread_   = std::thread([this] { server_->run(); });
    }

    ~ServerFixture() {
        server_->stop();
        if (thread_.joinable()) thread_.join();
    }

    unsigned short port() const { return port_; }

private:
    std::shared_ptr<cppplace::UserStore>       user_store_;
    std::shared_ptr<cppplace::SessionManager>  sessions_;
    std::shared_ptr<cppplace::CooldownManager> cooldowns_;
    std::shared_ptr<cppplace::EventBus>        bus_;
    std::shared_ptr<cppplace::Palette>         palette_;
    std::shared_ptr<cppplace::CanvasService>   canvas_svc_;
    std::shared_ptr<cppplace::RequestHandler>  handler_;
    std::unique_ptr<boost::asio::io_context>   ioc_;
    std::unique_ptr<cppplace::HttpServer>      server_;
    std::thread                                thread_;
    unsigned short                             port_ = 0;
};

} // namespace

class TestNetworkWorker : public QObject {
    Q_OBJECT

private slots:
    /// Sanity check: after moveToThread, the worker reports a different
    /// thread() than the test — the whole "two threads" requirement hinges
    /// on this.
    void livesOnSeparateThread() {
        QThread thread;
        NetworkWorker worker;
        worker.moveToThread(&thread);
        thread.start();

        QVERIFY(worker.thread() == &thread);
        QVERIFY(worker.thread() != QThread::currentThread());

        thread.quit();
        thread.wait();
    }

    /// Full register → fetchCanvas → placePixel → fetchCanvas round-trip
    /// against a real HttpServer. This exercises queued connections from the
    /// test thread to the worker thread and the signal/slot return path.
    void registersFetchesPlaces() {
        ServerFixture srv;

        QThread thread;
        NetworkWorker worker;
        worker.setBaseUrl(QUrl(QString("http://127.0.0.1:%1").arg(srv.port())));
        worker.moveToThread(&thread);
        connect(&thread, &QThread::started, &worker, &NetworkWorker::init);
        thread.start();

        QSignalSpy registered (&worker, &NetworkWorker::registered);
        QSignalSpy canvasSpy  (&worker, &NetworkWorker::canvasReceived);
        QSignalSpy placedSpy  (&worker, &NetworkWorker::pixelPlaced);
        QSignalSpy errSpy     (&worker, &NetworkWorker::networkError);

        QMetaObject::invokeMethod(&worker, "registerUser", Qt::QueuedConnection,
            Q_ARG(QString, "alice"), Q_ARG(QString, "secret"));
        QVERIFY(registered.wait(3000));
        QVERIFY(errSpy.isEmpty());

        QMetaObject::invokeMethod(&worker, "fetchCanvas", Qt::QueuedConnection);
        QVERIFY(canvasSpy.wait(3000));
        {
            const auto args = canvasSpy.takeFirst();
            QCOMPARE(args[0].toInt(), 16); // width
            QCOMPARE(args[1].toInt(), 16); // height
        }

        QMetaObject::invokeMethod(&worker, "placePixel", Qt::QueuedConnection,
            Q_ARG(int, 3), Q_ARG(int, 4), Q_ARG(int, 5));
        QVERIFY(placedSpy.wait(3000));
        {
            const auto args = placedSpy.takeFirst();
            QCOMPARE(args[0].toInt(), 3);
            QCOMPARE(args[1].toInt(), 4);
            QCOMPARE(args[2].toInt(), 5);
        }

        thread.quit();
        thread.wait();
    }

    /// Login with bad credentials must surface as authFailed, not crash, not
    /// silently succeed.
    void rejectsBadLogin() {
        ServerFixture srv;

        QThread thread;
        NetworkWorker worker;
        worker.setBaseUrl(QUrl(QString("http://127.0.0.1:%1").arg(srv.port())));
        worker.moveToThread(&thread);
        connect(&thread, &QThread::started, &worker, &NetworkWorker::init);
        thread.start();

        QSignalSpy fail (&worker, &NetworkWorker::authFailed);
        QSignalSpy ok   (&worker, &NetworkWorker::loggedIn);

        QMetaObject::invokeMethod(&worker, "login", Qt::QueuedConnection,
            Q_ARG(QString, "ghost"), Q_ARG(QString, "nope"));
        QVERIFY(fail.wait(3000));
        QVERIFY(ok.isEmpty());

        thread.quit();
        thread.wait();
    }
};

#include "test_network_worker.moc"

int runNetworkWorkerTests(int argc, char** argv) {
    TestNetworkWorker t;
    return QTest::qExec(&t, argc, argv);
}
