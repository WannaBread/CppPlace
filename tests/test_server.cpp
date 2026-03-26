#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

namespace {

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using     tcp   = net::ip::tcp;
using     json  = nlohmann::json;

// ── HTTP helper ───────────────────────────────────────────────────────────────

struct Resp {
    unsigned    status;
    json        body;
};

Resp doRequest(unsigned short   port,
               http::verb       method,
               std::string_view target,
               json             body  = nullptr,
               std::string_view token = "") {
    net::io_context   ioc;
    tcp::resolver     resolver(ioc);
    beast::tcp_stream stream(ioc);

    stream.connect(resolver.resolve("127.0.0.1", std::to_string(port)));

    http::request<http::string_body> req{method, std::string(target), 11};
    req.set(http::field::host,         "127.0.0.1");
    req.set(http::field::content_type, "application/json");
    if (!token.empty())
        req.set(http::field::authorization, "Bearer " + std::string(token));
    if (!body.is_null())
        req.body() = body.dump();
    req.prepare_payload();

    http::write(stream, req);

    beast::flat_buffer               buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);

    boost::system::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    return {res.result_int(), json::parse(res.body())};
}

// ── Fixture ───────────────────────────────────────────────────────────────────

class ServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        user_store_       = std::make_shared<cppplace::UserStore>();
        session_manager_  = std::make_shared<cppplace::SessionManager>();
        cooldown_manager_ = std::make_shared<cppplace::CooldownManager>(
            std::chrono::duration<double>(60.0));
        event_bus_        = std::make_shared<cppplace::EventBus>();
        palette_          = std::make_shared<cppplace::Palette>(
            cppplace::Palette::createDefault());
        canvas_service_   = std::make_shared<cppplace::CanvasService>(
            10, 10, palette_, session_manager_, cooldown_manager_, event_bus_);

        auto handler = std::make_shared<cppplace::RequestHandler>(
            user_store_, session_manager_, canvas_service_);

        ioc_    = std::make_unique<net::io_context>();
        server_ = std::make_unique<cppplace::HttpServer>(
            *ioc_, tcp::endpoint(tcp::v4(), 0), handler);
        port_   = server_->port();

        server_thread_ = std::thread([this] { server_->run(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    void TearDown() override {
        server_->stop();
        if (server_thread_.joinable()) server_thread_.join();
    }

    Resp request(http::verb method, std::string_view target,
                 json body = nullptr, std::string_view token = "") {
        return doRequest(port_, method, target, std::move(body), token);
    }

    // Register + return token
    std::string reg(const std::string& user, const std::string& pass = "password") {
        auto r = request(http::verb::post, "/api/register",
                         {{"username", user}, {"password", pass}});
        return r.body.value("token", "");
    }

    unsigned short port_{};
    std::unique_ptr<net::io_context>         ioc_;
    std::unique_ptr<cppplace::HttpServer>    server_;
    std::thread                              server_thread_;

    std::shared_ptr<cppplace::UserStore>       user_store_;
    std::shared_ptr<cppplace::SessionManager>  session_manager_;
    std::shared_ptr<cppplace::CooldownManager> cooldown_manager_;
    std::shared_ptr<cppplace::EventBus>        event_bus_;
    std::shared_ptr<cppplace::Palette>         palette_;
    std::shared_ptr<cppplace::CanvasService>   canvas_service_;
};

// ── POST /api/register ────────────────────────────────────────────────────────

TEST_F(ServerTest, Register_Success) {
    auto [status, body] = request(http::verb::post, "/api/register",
                                  {{"username", "alice"}, {"password", "secret"}});
    EXPECT_EQ(status, 201);
    EXPECT_TRUE(body["success"].get<bool>());
    EXPECT_FALSE(body["token"].get<std::string>().empty());
}

TEST_F(ServerTest, Register_DuplicateUsername) {
    reg("alice");
    auto [status, body] = request(http::verb::post, "/api/register",
                                  {{"username", "alice"}, {"password", "other"}});
    EXPECT_EQ(status, 409);
    EXPECT_FALSE(body["success"].get<bool>());
    EXPECT_TRUE(body.contains("error"));
}

TEST_F(ServerTest, Register_MissingPassword) {
    auto [status, body] = request(http::verb::post, "/api/register",
                                  {{"username", "alice"}});
    EXPECT_EQ(status, 400);
    EXPECT_FALSE(body["success"].get<bool>());
}

TEST_F(ServerTest, Register_MissingUsername) {
    auto [status, body] = request(http::verb::post, "/api/register",
                                  {{"password", "secret"}});
    EXPECT_EQ(status, 400);
}

TEST_F(ServerTest, Register_EmptyBody) {
    auto [status, body] = request(http::verb::post, "/api/register", json::object());
    EXPECT_EQ(status, 400);
}

TEST_F(ServerTest, Register_InvalidJson) {
    // raw string, not going through json helper
    net::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    stream.connect(resolver.resolve("127.0.0.1", std::to_string(port_)));

    http::request<http::string_body> req{http::verb::post, "/api/register", 11};
    req.set(http::field::host,         "127.0.0.1");
    req.set(http::field::content_type, "application/json");
    req.body() = "not json at all {{{";
    req.prepare_payload();
    http::write(stream, req);

    beast::flat_buffer buf;
    http::response<http::string_body> res;
    http::read(stream, buf, res);
    EXPECT_EQ(res.result_int(), 400);
}

// ── POST /api/login ───────────────────────────────────────────────────────────

TEST_F(ServerTest, Login_Success) {
    reg("bob", "pass");
    auto [status, body] = request(http::verb::post, "/api/login",
                                  {{"username", "bob"}, {"password", "pass"}});
    EXPECT_EQ(status, 200);
    EXPECT_TRUE(body["success"].get<bool>());
    EXPECT_FALSE(body["token"].get<std::string>().empty());
}

TEST_F(ServerTest, Login_WrongPassword) {
    reg("bob", "pass");
    auto [status, body] = request(http::verb::post, "/api/login",
                                  {{"username", "bob"}, {"password", "wrong"}});
    EXPECT_EQ(status, 401);
    EXPECT_FALSE(body["success"].get<bool>());
}

TEST_F(ServerTest, Login_UnknownUser) {
    auto [status, body] = request(http::verb::post, "/api/login",
                                  {{"username", "ghost"}, {"password", "x"}});
    EXPECT_EQ(status, 401);
}

TEST_F(ServerTest, Login_MissingFields) {
    auto [status, body] = request(http::verb::post, "/api/login",
                                  {{"username", "bob"}});
    EXPECT_EQ(status, 400);
}

TEST_F(ServerTest, Login_TokenIsDifferentEachTime) {
    reg("carol", "p");
    auto r1 = request(http::verb::post, "/api/login",
                      {{"username", "carol"}, {"password", "p"}});
    auto r2 = request(http::verb::post, "/api/login",
                      {{"username", "carol"}, {"password", "p"}});
    EXPECT_NE(r1.body["token"].get<std::string>(),
              r2.body["token"].get<std::string>());
}

// ── POST /api/logout ──────────────────────────────────────────────────────────

TEST_F(ServerTest, Logout_Success) {
    auto token = reg("dave");
    auto [status, body] = request(http::verb::post, "/api/logout", nullptr, token);
    EXPECT_EQ(status, 200);
    EXPECT_TRUE(body["success"].get<bool>());
}

TEST_F(ServerTest, Logout_NoToken) {
    auto [status, body] = request(http::verb::post, "/api/logout");
    EXPECT_EQ(status, 401);
}

TEST_F(ServerTest, Logout_InvalidatesSession) {
    auto token = reg("eve");
    request(http::verb::post, "/api/logout", nullptr, token);
    auto [status, body] = request(http::verb::post, "/api/pixel",
                                  {{"x", 0}, {"y", 0}, {"color_index", 1}}, token);
    EXPECT_EQ(status, 401);
}

// ── GET /api/canvas ───────────────────────────────────────────────────────────

TEST_F(ServerTest, GetCanvas_ReturnsOk) {
    auto [status, body] = request(http::verb::get, "/api/canvas");
    EXPECT_EQ(status, 200);
}

TEST_F(ServerTest, GetCanvas_CorrectDimensions) {
    auto [status, body] = request(http::verb::get, "/api/canvas");
    EXPECT_EQ(body["width"].get<int>(),  10);
    EXPECT_EQ(body["height"].get<int>(), 10);
}

TEST_F(ServerTest, GetCanvas_PixelsIsArray) {
    auto [status, body] = request(http::verb::get, "/api/canvas");
    ASSERT_TRUE(body["pixels"].is_array());
    EXPECT_EQ(body["pixels"].size(), 100u);  // 10×10
}

TEST_F(ServerTest, GetCanvas_OnlineCountPresent) {
    auto [status, body] = request(http::verb::get, "/api/canvas");
    EXPECT_TRUE(body.contains("online"));
}

// ── POST /api/pixel ───────────────────────────────────────────────────────────

TEST_F(ServerTest, PlacePixel_Success) {
    auto token = reg("frank");
    auto [status, body] = request(http::verb::post, "/api/pixel",
                                  {{"x", 3}, {"y", 4}, {"color_index", 5}}, token);
    EXPECT_EQ(status, 200);
    EXPECT_TRUE(body["success"].get<bool>());
}

TEST_F(ServerTest, PlacePixel_ReflectedInCanvas) {
    auto token = reg("grace");
    request(http::verb::post, "/api/pixel",
            {{"x", 2}, {"y", 3}, {"color_index", 7}}, token);

    auto [status, body] = request(http::verb::get, "/api/canvas");
    // pixel at (2,3) = index 3*10 + 2 = 32
    EXPECT_EQ(body["pixels"][32].get<int>(), 7);
}

TEST_F(ServerTest, PlacePixel_NoToken) {
    auto [status, body] = request(http::verb::post, "/api/pixel",
                                  {{"x", 0}, {"y", 0}, {"color_index", 1}});
    EXPECT_EQ(status, 401);
}

TEST_F(ServerTest, PlacePixel_InvalidToken) {
    auto [status, body] = request(http::verb::post, "/api/pixel",
                                  {{"x", 0}, {"y", 0}, {"color_index", 1}},
                                  "totally-fake-token");
    EXPECT_EQ(status, 401);
}

TEST_F(ServerTest, PlacePixel_OutOfBoundsX) {
    auto token = reg("henry");
    auto [status, body] = request(http::verb::post, "/api/pixel",
                                  {{"x", 99}, {"y", 0}, {"color_index", 1}}, token);
    EXPECT_EQ(status, 422);
}

TEST_F(ServerTest, PlacePixel_OutOfBoundsY) {
    auto token = reg("iris");
    auto [status, body] = request(http::verb::post, "/api/pixel",
                                  {{"x", 0}, {"y", 99}, {"color_index", 1}}, token);
    EXPECT_EQ(status, 422);
}

TEST_F(ServerTest, PlacePixel_InvalidColor) {
    auto token = reg("jack");
    auto [status, body] = request(http::verb::post, "/api/pixel",
                                  {{"x", 0}, {"y", 0}, {"color_index", 200}}, token);
    EXPECT_EQ(status, 422);
}

TEST_F(ServerTest, PlacePixel_Cooldown) {
    auto token = reg("kate");
    request(http::verb::post, "/api/pixel",
            {{"x", 1}, {"y", 1}, {"color_index", 1}}, token);
    auto [status, body] = request(http::verb::post, "/api/pixel",
                                  {{"x", 2}, {"y", 2}, {"color_index", 2}}, token);
    EXPECT_EQ(status, 429);
    EXPECT_THAT(body["error"].get<std::string>(),
                ::testing::HasSubstr("ooldown"));
}

TEST_F(ServerTest, PlacePixel_MissingColorIndex) {
    auto token = reg("liam");
    auto [status, body] = request(http::verb::post, "/api/pixel",
                                  {{"x", 1}, {"y", 1}}, token);
    EXPECT_EQ(status, 400);
}

TEST_F(ServerTest, PlacePixel_ZeroCoordinates) {
    auto token = reg("mia");
    auto [status, body] = request(http::verb::post, "/api/pixel",
                                  {{"x", 0}, {"y", 0}, {"color_index", 0}}, token);
    EXPECT_EQ(status, 200);
}

// ── Routing ───────────────────────────────────────────────────────────────────

TEST_F(ServerTest, UnknownRoute_Returns404) {
    auto [status, body] = request(http::verb::get, "/api/unknown");
    EXPECT_EQ(status, 404);
}

TEST_F(ServerTest, WrongMethod_Returns404) {
    auto [status, body] = request(http::verb::get, "/api/register");
    EXPECT_EQ(status, 404);
}

TEST_F(ServerTest, RootPath_Returns404) {
    auto [status, body] = request(http::verb::get, "/");
    EXPECT_EQ(status, 404);
}

} // namespace
