#pragma once

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <string>

namespace cppplace {

class UserStore;
class SessionManager;
class CanvasService;

// Stateless request dispatcher.  One instance is shared by all connections.
// All referenced services are thread-safe.
class RequestHandler {
public:
    using Request  = boost::beast::http::request<boost::beast::http::string_body>;
    using Response = boost::beast::http::response<boost::beast::http::string_body>;

    RequestHandler(std::shared_ptr<UserStore>       user_store,
                   std::shared_ptr<SessionManager>  session_manager,
                   std::shared_ptr<CanvasService>   canvas_service);

    Response handle(const Request& req);

private:
    // ── Endpoint handlers ────────────────────────────────────────────────────
    Response handleRegister   (const Request& req);
    Response handleLogin      (const Request& req);
    Response handleLogout     (const Request& req);
    Response handleGetCanvas  (const Request& req);
    Response handlePlacePixel (const Request& req);

    // ── Helpers ──────────────────────────────────────────────────────────────
    Response jsonResponse (unsigned status, nlohmann::json body);
    Response errorResponse(unsigned status, std::string_view error);

    // Extract the bearer token from the Authorization header (empty if absent).
    std::string extractToken(const Request& req) const;

    // ── Dependencies ─────────────────────────────────────────────────────────
    std::shared_ptr<UserStore>      user_store_;
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<CanvasService>  canvas_service_;
};

} // namespace cppplace
