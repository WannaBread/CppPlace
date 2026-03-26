#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <memory>
#include <thread>
#include <vector>

namespace cppplace {

class RequestHandler;

class HttpServer {
public:
    HttpServer(boost::asio::ip::tcp::endpoint  endpoint,
               std::shared_ptr<RequestHandler> handler,
               unsigned int worker_threads = std::max(1u,
                   std::thread::hardware_concurrency()));

    void run();   // blocking
    void stop();  // thread-safe

    unsigned short port() const;

private:
    void doAccept();

    boost::asio::io_context         ioc_;
    boost::asio::ip::tcp::acceptor  acceptor_;
    boost::asio::signal_set         signals_;
    std::shared_ptr<RequestHandler> handler_;
    unsigned int                    worker_threads_;
    std::vector<std::thread>        workers_;
};

} // namespace cppplace

