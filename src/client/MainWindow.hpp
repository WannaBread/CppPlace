#pragma once

#include <QMainWindow>
#include <QUrl>
#include <vector>
#include <cstdint>

class QThread;
class QLabel;
class QScrollArea;
class QTimer;

namespace cppplace::client {

class NetworkWorker;
class CanvasRenderer;
class CanvasView;
class PaletteBar;
class LoginDialog;

/// Owns the two worker threads and ferries data between them and the GUI.
///
///   GUI thread ───── signal ────▶ NetworkWorker (net thread)
///                                     │
///                  signal             ▼ HTTP
///   GUI thread ◀─────────────── server reply
///        │
///        ├──▶ CanvasRenderer (render thread) ──signal──▶ CanvasView (GUI)
///        └──▶ status updates
///
/// All cross-thread arrows are queued connections (Qt picks this automatically
/// when sender and receiver live in different threads).
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const QUrl& serverUrl, QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onLoggedIn(const QString& token);
    void onAuthFailed(int status, const QString& message);
    void onCanvasReceived(int w, int h, int online,
                          const std::vector<uint8_t>& pixels);
    void onPixelClicked(int x, int y);
    void onPixelPlaced(int x, int y, int colorIndex);
    void onPlaceFailed(int status, const QString& message);
    void onNetworkError(const QString& message);
    void onImageReady(const QImage& image);
    void onScaleChanged(int scale);

private:
    void buildUi();
    void promptLogin();
    void startWorkers();

    QUrl base_url_;

    // Workers (live on their own threads)
    QThread*        net_thread_    = nullptr;
    QThread*        render_thread_ = nullptr;
    NetworkWorker*  network_       = nullptr;
    CanvasRenderer* renderer_      = nullptr;

    // GUI
    CanvasView*     view_     = nullptr;
    PaletteBar*     palette_  = nullptr;
    QScrollArea*    scroll_   = nullptr;
    QLabel*         status_   = nullptr;
    LoginDialog*    login_    = nullptr;
    QTimer*         poll_     = nullptr;

    // State
    int  scale_       = 4;
    int  width_       = 0;
    int  height_      = 0;
    bool authed_      = false;
};

} // namespace cppplace::client
