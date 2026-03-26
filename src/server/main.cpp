#include "server/HttpServer.hpp"
#include "server/RequestHandler.hpp"
#include "services/CanvasService.hpp"
#include "services/CooldownManager.hpp"
#include "services/EventBus.hpp"
#include "services/Palette.hpp"
#include "services/SessionManager.hpp"
#include "services/UserStore.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

void printUsage(const char* argv0) {
    std::cerr << "Usage: " << argv0 << " [port] [width] [height] [cooldown_sec]\n"
              << "  port         TCP port to listen on   (default: 8080)\n"
              << "  width        Canvas width  in pixels (default: 1000)\n"
              << "  height       Canvas height in pixels (default: 1000)\n"
              << "  cooldown_sec Per-user placement cooldown in seconds (default: 300)\n";
}

template <typename T>
T parseArg(const char* s, const char* name) {
    try {
        long v = std::stol(s);
        if (v <= 0) throw std::invalid_argument("must be positive");
        return static_cast<T>(v);
    } catch (...) {
        throw std::runtime_error(std::string("Invalid value for ") + name + ": " + s);
    }
}

} // namespace

int main(int argc, char* argv[]) {
    unsigned short port         = 8080;
    size_t         canvas_w     = 1000;
    size_t         canvas_h     = 1000;
    double         cooldown_sec = 300.0;

    if (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        printUsage(argv[0]);
        return 0;
    }

    try {
        if (argc > 1) port         = parseArg<unsigned short>(argv[1], "port");
        if (argc > 2) canvas_w     = parseArg<size_t>(argv[2], "width");
        if (argc > 3) canvas_h     = parseArg<size_t>(argv[3], "height");
        if (argc > 4) cooldown_sec = parseArg<double>(argv[4], "cooldown_sec");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n\n";
        printUsage(argv[0]);
        return 1;
    }

    // ── Build services ────────────────────────────────────────────────────────

    auto user_store      = std::make_shared<cppplace::UserStore>();
    auto session_manager = std::make_shared<cppplace::SessionManager>();
    auto cooldown_mgr    = std::make_shared<cppplace::CooldownManager>(
        std::chrono::duration<double>(cooldown_sec));
    auto event_bus       = std::make_shared<cppplace::EventBus>();
    auto palette         = std::make_shared<cppplace::Palette>(
        cppplace::Palette::createDefault());
    auto canvas_service  = std::make_shared<cppplace::CanvasService>(
        canvas_w, canvas_h, palette, session_manager, cooldown_mgr, event_bus);

    auto handler = std::make_shared<cppplace::RequestHandler>(
        user_store, session_manager, canvas_service);

    // ── IO context + signal handling ──────────────────────────────────────────

    boost::asio::io_context ioc;

    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&](boost::system::error_code /*ec*/, int signo) {
        std::cout << "\nCaught signal " << signo << ", shutting down...\n";
        ioc.stop();
    });

    // ── Start server ──────────────────────────────────────────────────────────

    cppplace::HttpServer server(
        ioc,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port),
        handler);

    std::cout << "CppPlace server started\n"
              << "  Port    : " << port         << "\n"
              << "  Canvas  : " << canvas_w << " x " << canvas_h << "\n"
              << "  Cooldown: " << cooldown_sec << "s\n"
              << "  Palette : " << palette->size() << " colors\n"
              << "Press Ctrl+C to stop.\n\n";

    server.run();   // blocks until ioc.stop()

    std::cout << "Server stopped.\n";
    return 0;
}
