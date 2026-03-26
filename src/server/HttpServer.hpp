#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>

namespace cppplace {

class RequestHandler;

// Synchronous-per-connection HTTP/1.1 server backed by Boost.Beast.
// Each accepted connection is dispatched to a detached thread so the
// io_context remains free to keep accepting.
//
// Typical usage:
//   boost::asio::io_context ioc;
//   auto server = std::make_unique<HttpServer>(
//       ioc, tcp::endpoint(tcp::v4(), 8080), handler);
//   std::thread t([&]{ server->run(); });  // blocking
//   ...
//   server->stop();
//   t.join();

class HttpServer {
public:
    HttpServer(boost::asio::io_context& ioc,
               boost::asio::ip::tcp::endpoint endpoint,
               std::shared_ptr<RequestHandler> handler);

    // Blocking: posts the first async_accept and calls ioc_.run().
    void run();

    // Thread-safe: closes the acceptor and stops the io_context.
    void stop();

    // Actual port bound (useful when endpoint was constructed with port 0).
    unsigned short port() const;

private:
    void doAccept();

    boost::asio::io_context&            ioc_;
    boost::asio::ip::tcp::acceptor      acceptor_;
    std::shared_ptr<RequestHandler>     handler_;
};

} // namespace cppplace
