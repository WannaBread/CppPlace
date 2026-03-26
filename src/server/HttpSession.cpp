#include "server/HttpSession.hpp"
#include "server/RequestHandler.hpp"

namespace cppplace {

namespace beast = boost::beast;
namespace http  = beast::http;
using     tcp   = boost::asio::ip::tcp;

HttpSession::HttpSession(tcp::socket socket, std::shared_ptr<RequestHandler> handler)
    : stream_(std::move(socket))
    , handler_(std::move(handler))
{}

void HttpSession::start() {
    doRead();
}

// ── Read ──────────────────────────────────────────────────────────────────────

void HttpSession::doRead() {
    req_ = {};

    http::async_read(stream_, buffer_, req_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes) {
            self->onRead(ec, bytes);
        });
}

void HttpSession::onRead(beast::error_code ec, std::size_t) {
    if (ec == http::error::end_of_stream)
        return doClose();

    if (ec) return;

    doWrite(handler_->handle(req_));
}

// ── Write ─────────────────────────────────────────────────────────────────────

void HttpSession::doWrite(http::response<http::string_body> res) {
    res.keep_alive(req_.keep_alive());

    res_ = std::make_shared<http::response<http::string_body>>(std::move(res));
    const bool keep_alive = res_->keep_alive();

    http::async_write(stream_, *res_,
        [self = shared_from_this(), keep_alive](beast::error_code ec, std::size_t bytes) {
            self->onWrite(keep_alive, ec, bytes);
        });
}

void HttpSession::onWrite(bool keep_alive, beast::error_code ec, std::size_t) {
    res_.reset();

    if (ec) return;

    if (!keep_alive)
        return doClose();

    doRead();
}

void HttpSession::doClose() {
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
}

} // namespace cppplace
