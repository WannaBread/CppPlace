#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>

namespace cppplace {

class RequestHandler;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    explicit HttpSession(boost::asio::ip::tcp::socket       socket,
                         std::shared_ptr<RequestHandler>    handler);

    void start();

private:
    void doRead();
    void onRead (boost::beast::error_code ec, std::size_t);
    void doWrite(boost::beast::http::response<boost::beast::http::string_body> res);
    void onWrite(bool keep_alive, boost::beast::error_code ec, std::size_t);
    void doClose();

    boost::beast::tcp_stream                                          stream_;
    boost::beast::flat_buffer                                         buffer_;
    boost::beast::http::request<boost::beast::http::string_body>      req_;
    std::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> res_;
    std::shared_ptr<RequestHandler>                                   handler_;
};

}
