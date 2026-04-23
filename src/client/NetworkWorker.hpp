#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QByteArray>
#include <vector>
#include <cstdint>

class QNetworkAccessManager;
class QNetworkReply;

namespace cppplace::client {

/// Lives in its own QThread. All HTTP requests to the server happen here so
/// the GUI thread never blocks on network I/O. Communication with the rest
/// of the app is purely via signals/slots (queued connections across threads).
class NetworkWorker : public QObject {
    Q_OBJECT
public:
    explicit NetworkWorker(QObject* parent = nullptr);
    ~NetworkWorker() override;

    void setBaseUrl(const QUrl& url);
    QString token() const { return token_; }

public slots:
    /// Must be invoked once after moveToThread() so QNetworkAccessManager is
    /// created in the worker thread (Qt requirement).
    void init();

    void registerUser(const QString& username, const QString& password);
    void login(const QString& username, const QString& password);
    void logout();
    void fetchCanvas();
    void placePixel(int x, int y, int colorIndex);

signals:
    void registered(const QString& token);
    void loggedIn(const QString& token);
    void loggedOut();
    void authFailed(int httpStatus, const QString& message);

    void canvasReceived(int width, int height, int online,
                        const std::vector<uint8_t>& pixels);

    void pixelPlaced(int x, int y, int colorIndex);
    void placeFailed(int httpStatus, const QString& message);

    void networkError(const QString& message);

private:
    QNetworkReply* postJson(const QString& path, const QByteArray& body) const;
    QNetworkReply* getJson(const QString& path) const;
    void attachAuth(QNetworkReply* reply) const;

    QNetworkAccessManager* nam_ = nullptr;
    QUrl    base_url_;
    QString token_;
};

} // namespace cppplace::client
