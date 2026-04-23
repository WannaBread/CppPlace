#include "client/MainWindow.hpp"
#include "client/NetworkWorker.hpp"
#include "client/CanvasRenderer.hpp"
#include "client/CanvasView.hpp"
#include "client/PaletteBar.hpp"
#include "client/LoginDialog.hpp"
#include "client/ClientPalette.hpp"

#include <QThread>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>

namespace cppplace::client {


MainWindow::MainWindow(const QUrl& serverUrl, QWidget* parent)
    : QMainWindow(parent), base_url_(serverUrl) {
    buildUi();
    startWorkers();
    promptLogin();
}

MainWindow::~MainWindow() {
    if (net_thread_)    { net_thread_->quit();    net_thread_->wait(); }
    if (render_thread_) { render_thread_->quit(); render_thread_->wait(); }
}

void MainWindow::buildUi() {
    setWindowTitle("CppPlace");
    resize(900, 720);

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    palette_ = new PaletteBar(defaultPalette(), this);
    root->addWidget(palette_);

    view_ = new CanvasView(this);
    view_->setScale(scale_);

    scroll_ = new QScrollArea(this);
    scroll_->setWidget(view_);
    scroll_->setAlignment(Qt::AlignCenter);
    scroll_->setBackgroundRole(QPalette::Dark);
    scroll_->viewport()->setMouseTracking(true);
    root->addWidget(scroll_, 1);

    setCentralWidget(central);

    status_ = new QLabel("Connecting…", this);
    statusBar()->addWidget(status_, 1);

    connect(view_, &CanvasView::pixelClicked,
            this,  &MainWindow::onPixelClicked);
    connect(view_, &CanvasView::scaleChanged,
            this,  &MainWindow::onScaleChanged);
    connect(view_, &CanvasView::panRequested, this, [this](int dx, int dy) {
        scroll_->horizontalScrollBar()->setValue(
            scroll_->horizontalScrollBar()->value() + dx);
        scroll_->verticalScrollBar()->setValue(
            scroll_->verticalScrollBar()->value() + dy);
    });
}

void MainWindow::startWorkers() {
    // ── Network worker thread ─────────────────────────────────────────────
    net_thread_ = new QThread(this);
    network_    = new NetworkWorker;
    network_->setBaseUrl(base_url_);
    network_->moveToThread(net_thread_);

    connect(net_thread_, &QThread::started,  network_, &NetworkWorker::init);
    connect(net_thread_, &QThread::finished, network_, &QObject::deleteLater);

    connect(network_, &NetworkWorker::loggedIn,        this, &MainWindow::onLoggedIn);
    connect(network_, &NetworkWorker::registered,      this, &MainWindow::onLoggedIn);
    connect(network_, &NetworkWorker::authFailed,      this, &MainWindow::onAuthFailed);
    connect(network_, &NetworkWorker::canvasReceived,  this, &MainWindow::onCanvasReceived);
    connect(network_, &NetworkWorker::pixelPlaced,     this, &MainWindow::onPixelPlaced);
    connect(network_, &NetworkWorker::placeFailed,     this, &MainWindow::onPlaceFailed);
    connect(network_, &NetworkWorker::networkError,    this, &MainWindow::onNetworkError);

    net_thread_->start();

    // ── Render worker thread ──────────────────────────────────────────────
    render_thread_ = new QThread(this);
    renderer_      = new CanvasRenderer;
    renderer_->setPalette(defaultPaletteColors());
    renderer_->moveToThread(render_thread_);

    connect(render_thread_, &QThread::finished, renderer_, &QObject::deleteLater);
    connect(renderer_, &CanvasRenderer::imageReady,
            this,      &MainWindow::onImageReady);

    render_thread_->start();

    poll_ = new QTimer(this);
    poll_->setInterval(2000);
    connect(poll_, &QTimer::timeout, network_, &NetworkWorker::fetchCanvas);
}

void MainWindow::promptLogin() {
    login_ = new LoginDialog(this);

    connect(login_, &LoginDialog::loginRequested,    network_, &NetworkWorker::login);
    connect(login_, &LoginDialog::registerRequested, network_, &NetworkWorker::registerUser);

    login_->show();
}

// ── Network reply handlers ────────────────────────────────────────────────────

void MainWindow::onLoggedIn(const QString& /*token*/) {
    authed_ = true;
    if (login_) { login_->accept(); login_->deleteLater(); login_ = nullptr; }
    status_->setText("Connected. Fetching canvas…");
    QMetaObject::invokeMethod(network_, "fetchCanvas", Qt::QueuedConnection);
    poll_->start();
}

void MainWindow::onAuthFailed(int status, const QString& message) {
    if (login_) login_->setError(QString("[%1] %2").arg(status).arg(message));
}

void MainWindow::onCanvasReceived(int w, int h, int online,
                                  const std::vector<uint8_t>& pixels) {
    width_  = w;
    height_ = h;
    status_->setText(QString("%1x%2 — %3 online").arg(w).arg(h).arg(online));

    QMetaObject::invokeMethod(renderer_, "renderCanvas", Qt::QueuedConnection,
        Q_ARG(int, w), Q_ARG(int, h),
        Q_ARG(std::vector<uint8_t>, pixels), Q_ARG(int, scale_));
}

void MainWindow::onPixelClicked(int x, int y) {
    if (!authed_) return;
    QMetaObject::invokeMethod(network_, "placePixel", Qt::QueuedConnection,
        Q_ARG(int, x), Q_ARG(int, y), Q_ARG(int, palette_->currentColor()));
}

void MainWindow::onPixelPlaced(int x, int y, int colorIndex) {
    QMetaObject::invokeMethod(renderer_, "renderPatch", Qt::QueuedConnection,
        Q_ARG(int, x), Q_ARG(int, y),
        Q_ARG(int, colorIndex), Q_ARG(int, scale_));
    status_->setText(QString("Placed (%1, %2)").arg(x).arg(y));
}

void MainWindow::onPlaceFailed(int status, const QString& message) {
    status_->setText(QString("[%1] %2").arg(status).arg(message));
}

void MainWindow::onNetworkError(const QString& message) {
    status_->setText("Network: " + message);
}

void MainWindow::onImageReady(const QImage& image) {
    view_->setImage(image);
}

void MainWindow::onScaleChanged(int scale) {
    scale_ = scale;
    QMetaObject::invokeMethod(renderer_, "rescale", Qt::QueuedConnection,
        Q_ARG(int, scale));
}

} // namespace cppplace::client
