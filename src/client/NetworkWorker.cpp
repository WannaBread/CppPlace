#include "client/NetworkWorker.hpp"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QThread>
#include <nlohmann/json.hpp>

namespace cppplace::client {

using json = nlohmann::json;

NetworkWorker::NetworkWorker(QObject* parent) : QObject(parent) {}
NetworkWorker::~NetworkWorker() = default;

void NetworkWorker::setBaseUrl(const QUrl& url) { base_url_ = url; }

void NetworkWorker::init() {
    nam_ = new QNetworkAccessManager(this);
}

QNetworkReply* NetworkWorker::postJson(const QString& path,
                                       const QByteArray& body) const {
    QUrl url = base_url_;
    url.setPath(path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!token_.isEmpty())
        req.setRawHeader("Authorization", ("Bearer " + token_).toUtf8());
    return nam_->post(req, body);
}

QNetworkReply* NetworkWorker::getJson(const QString& path) const {
    QUrl url = base_url_;
    url.setPath(path);
    QNetworkRequest req(url);
    if (!token_.isEmpty())
        req.setRawHeader("Authorization", ("Bearer " + token_).toUtf8());
    return nam_->get(req);
}

// ── Auth ──────────────────────────────────────────────────────────────────────

void NetworkWorker::registerUser(const QString& username, const QString& password) {
    json body = {{"username", username.toStdString()},
                 {"password", password.toStdString()}};
    auto* reply = postJson("/api/register",
                           QByteArray::fromStdString(body.dump()));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const int status = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError && status == 0) {
            emit networkError(reply->errorString());
            return;
        }
        try {
            auto j = json::parse(reply->readAll().toStdString());
            if (status == 201 && j.value("success", false)) {
                token_ = QString::fromStdString(j.value("token", std::string{}));
                emit registered(token_);
            } else {
                emit authFailed(status,
                    QString::fromStdString(j.value("error", std::string{"Registration failed"})));
            }
        } catch (...) {
            emit authFailed(status, "Invalid server response");
        }
    });
}

void NetworkWorker::login(const QString& username, const QString& password) {
    json body = {{"username", username.toStdString()},
                 {"password", password.toStdString()}};
    auto* reply = postJson("/api/login",
                           QByteArray::fromStdString(body.dump()));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const int status = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError && status == 0) {
            emit networkError(reply->errorString());
            return;
        }
        try {
            auto j = json::parse(reply->readAll().toStdString());
            if (status == 200 && j.value("success", false)) {
                token_ = QString::fromStdString(j.value("token", std::string{}));
                emit loggedIn(token_);
            } else {
                emit authFailed(status,
                    QString::fromStdString(j.value("error", std::string{"Login failed"})));
            }
        } catch (...) {
            emit authFailed(status, "Invalid server response");
        }
    });
}

void NetworkWorker::logout() {
    if (token_.isEmpty()) { emit loggedOut(); return; }
    auto* reply = postJson("/api/logout", QByteArray{});
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        token_.clear();
        emit loggedOut();
    });
}

// ── Canvas ────────────────────────────────────────────────────────────────────

void NetworkWorker::fetchCanvas() {
    auto* reply = getJson("/api/canvas");
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit networkError(reply->errorString());
            return;
        }
        try {
            auto j = json::parse(reply->readAll().toStdString());
            const int w      = j.value("width", 0);
            const int h      = j.value("height", 0);
            const int online = j.value("online", 0);
            std::vector<uint8_t> pixels;
            if (j.contains("pixels") && j["pixels"].is_array()) {
                pixels.reserve(j["pixels"].size());
                for (const auto& v : j["pixels"])
                    pixels.push_back(static_cast<uint8_t>(v.get<int>()));
            }
            emit canvasReceived(w, h, online, pixels);
        } catch (const std::exception& ex) {
            emit networkError(QString::fromStdString(ex.what()));
        }
    });
}

void NetworkWorker::placePixel(int x, int y, int colorIndex) {
    json body = {{"x", x}, {"y", y}, {"color_index", colorIndex}};
    auto* reply = postJson("/api/pixel",
                           QByteArray::fromStdString(body.dump()));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, x, y, colorIndex]() {
        reply->deleteLater();
        const int status = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError && status == 0) {
            emit networkError(reply->errorString());
            return;
        }
        if (status == 200) {
            emit pixelPlaced(x, y, colorIndex);
            return;
        }
        QString msg = "Place failed";
        try {
            auto j = json::parse(reply->readAll().toStdString());
            msg = QString::fromStdString(j.value("error", msg.toStdString()));
        } catch (...) {}
        emit placeFailed(status, msg);
    });
}

} // namespace cppplace::client
