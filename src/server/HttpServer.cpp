#include "server/HttpServer.hpp"
#include "server/HttpSession.hpp"

#include <boost/asio/strand.hpp>
#include <stdexcept>

namespace cppplace {

namespace net = boost::asio;
using     tcp = net::ip::tcp;

// ── Construction ──────────────────────────────────────────────────────────────

HttpServer::HttpServer(tcp::endpoint                  endpoint,
                       std::shared_ptr<RequestHandler> handler,
                       unsigned int                    worker_threads)
    : acceptor_(ioc_)
    , signals_(ioc_, SIGINT, SIGTERM)
    , handler_(std::move(handler))
    , worker_threads_(worker_threads)
{
    boost::system::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) throw std::runtime_error("open: "   + ec.message());

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) throw std::runtime_error("reuse: "  + ec.message());

    acceptor_.bind(endpoint, ec);
    if (ec) throw std::runtime_error("bind: "   + ec.message());

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) throw std::runtime_error("listen: " + ec.message());

    signals_.async_wait([this](boost::system::error_code, int) { stop(); });
}

// ── Public interface ──────────────────────────────────────────────────────────

unsigned short HttpServer::port() const {
    return acceptor_.local_endpoint().port();
}

void HttpServer::run() {
    doAccept();

    workers_.reserve(worker_threads_ - 1);
    for (unsigned int i = 1; i < worker_threads_; ++i)
        workers_.emplace_back([this] { ioc_.run(); });

    ioc_.run();

    for (auto& t : workers_)
        if (t.joinable()) t.join();
}

void HttpServer::stop() {
    boost::system::error_code ec;
    acceptor_.close(ec);
    ioc_.stop();
}

void HttpServer::doAccept() {
    acceptor_.async_accept(
        net::make_strand(ioc_),
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec)
                std::make_shared<HttpSession>(std::move(socket), handler_)->start();

            if (acceptor_.is_open())
                doAccept();
        });
}

} // namespace cppplace

