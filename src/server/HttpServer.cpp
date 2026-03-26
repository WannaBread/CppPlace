#include "server/HttpServer.hpp"
#include "server/RequestHandler.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <stdexcept>
#include <thread>

namespace cppplace {

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using     tcp   = net::ip::tcp;

// ── Construction ─────────────────────────────────────────────────────────────

HttpServer::HttpServer(net::io_context&              ioc,
                       tcp::endpoint                 endpoint,
                       std::shared_ptr<RequestHandler> handler)
    : ioc_(ioc)
    , acceptor_(ioc)
    , handler_(std::move(handler))
{
    boost::system::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) throw std::runtime_error("acceptor open: " + ec.message());

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) throw std::runtime_error("set_option reuse_address: " + ec.message());

    acceptor_.bind(endpoint, ec);
    if (ec) throw std::runtime_error("bind: " + ec.message());

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) throw std::runtime_error("listen: " + ec.message());
}

// ── Public interface ──────────────────────────────────────────────────────────

unsigned short HttpServer::port() const {
    return acceptor_.local_endpoint().port();
}

void HttpServer::run() {
    doAccept();
    ioc_.run();
}

void HttpServer::stop() {
    boost::system::error_code ec;
    acceptor_.close(ec);
    ioc_.stop();
}

// ── Private ───────────────────────────────────────────────────────────────────

void HttpServer::doAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                // Capture handler_ by value (shared_ptr) so the connection
                // thread keeps services alive independently of HttpServer's
                // own lifetime — prevents use-after-free when stop() races
                // with in-flight connection threads.
                std::thread([h = handler_, sock = std::move(socket)]() mutable {
                    try {
                        beast::flat_buffer               buffer;
                        http::request<http::string_body> req;
                        boost::system::error_code        err;

                        http::read(sock, buffer, req, err);
                        if (err) return;

                        auto res = h->handle(req);
                        http::write(sock, res, err);
                        sock.shutdown(tcp::socket::shutdown_send, err);
                    } catch (...) {}
                }).detach();
            }
            if (acceptor_.is_open())
                doAccept();
        });
}

// handleConnection is no longer used — logic moved into the lambda above.

} // namespace cppplace
