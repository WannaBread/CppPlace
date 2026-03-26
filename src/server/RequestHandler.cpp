#include "server/RequestHandler.hpp"
#include "services/UserStore.hpp"
#include "services/SessionManager.hpp"
#include "services/CanvasService.hpp"
#include "common/Result.hpp"

#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

namespace cppplace {

namespace http = boost::beast::http;
using json = nlohmann::json;

RequestHandler::RequestHandler(std::shared_ptr<UserStore>      user_store,
                                std::shared_ptr<SessionManager> session_manager,
                                std::shared_ptr<CanvasService>  canvas_service)
    : user_store_(std::move(user_store))
    , session_manager_(std::move(session_manager))
    , canvas_service_(std::move(canvas_service))
{}

RequestHandler::Response RequestHandler::handle(const Request& req) {
    const auto target = std::string(req.target());
    const auto method = req.method();

    if (method == http::verb::post && target == "/api/register") return handleRegister(req);
    if (method == http::verb::post && target == "/api/login")    return handleLogin(req);
    if (method == http::verb::post && target == "/api/logout")   return handleLogout(req);
    if (method == http::verb::get  && target == "/api/canvas")   return handleGetCanvas(req);
    if (method == http::verb::post && target == "/api/pixel")    return handlePlacePixel(req);

    return errorResponse(404, "Not found");
}

RequestHandler::Response RequestHandler::handleRegister(const Request& req) {
    json body;
    try { body = json::parse(req.body()); }
    catch (...) { return errorResponse(400, "Invalid JSON"); }

    if (!body.contains("username") || !body.contains("password"))
        return errorResponse(400, "Missing username or password");

    auto result = user_store_->registerUser(
        body["username"].get<std::string>(),
        body["password"].get<std::string>());

    if (!result.ok())
        return errorResponse(409, result.message());

    auto token = session_manager_->createSession(body["username"].get<std::string>());
    return jsonResponse(201, {{"success", true}, {"token", token}});
}

RequestHandler::Response RequestHandler::handleLogin(const Request& req) {
    json body;
    try { body = json::parse(req.body()); }
    catch (...) { return errorResponse(400, "Invalid JSON"); }

    if (!body.contains("username") || !body.contains("password"))
        return errorResponse(400, "Missing username or password");

    auto result = user_store_->authenticate(
        body["username"].get<std::string>(),
        body["password"].get<std::string>());

    if (!result.ok())
        return errorResponse(401, result.message());

    auto token = session_manager_->createSession(result.value());
    return jsonResponse(200, {{"success", true}, {"token", token}});
}

RequestHandler::Response RequestHandler::handleLogout(const Request& req) {
    auto token = extractToken(req);
    if (token.empty())
        return errorResponse(401, "Unauthorized");

    session_manager_->removeSession(token);
    return jsonResponse(200, {{"success", true}});
}

RequestHandler::Response RequestHandler::handleGetCanvas(const Request& /*req*/) {
    const auto pixels  = canvas_service_->getCanvasState();
    const size_t width  = canvas_service_->getWidth();
    const size_t height = canvas_service_->getHeight();
    const size_t online = canvas_service_->getOnlineCount();

    std::vector<uint8_t> colors;
    colors.reserve(pixels.size());
    for (const auto& p : pixels)
        colors.push_back(p.color_index);

    return jsonResponse(200, {
        {"width",  width},
        {"height", height},
        {"online", online},
        {"pixels", colors}
    });
}

RequestHandler::Response RequestHandler::handlePlacePixel(const Request& req) {
    auto token = extractToken(req);
    if (token.empty())
        return errorResponse(401, "Unauthorized");

    json body;
    try { body = json::parse(req.body()); }
    catch (...) { return errorResponse(400, "Invalid JSON"); }

    if (!body.contains("x") || !body.contains("y") || !body.contains("color_index"))
        return errorResponse(400, "Missing x, y, or color_index");

    auto result = canvas_service_->placePixel(
        token,
        body["x"].get<size_t>(),
        body["y"].get<size_t>(),
        body["color_index"].get<uint8_t>());

    if (!result.ok()) {
        unsigned status;
        switch (result.code()) {
            case ErrorCode::Unauthorized:        status = 401; break;
            case ErrorCode::CooldownActive:      status = 429; break;
            case ErrorCode::InvalidCoordinates:
            case ErrorCode::InvalidColor:        status = 422; break;
            default:                             status = 500; break;
        }
        return errorResponse(status, result.message());
    }

    return jsonResponse(200, {{"success", true}});
}

RequestHandler::Response RequestHandler::jsonResponse(unsigned status, json body) {
    Response res{static_cast<http::status>(status), 11};
    res.set(http::field::content_type, "application/json");
    res.body() = body.dump();
    res.prepare_payload();
    return res;
}

RequestHandler::Response RequestHandler::errorResponse(unsigned status,
                                                        std::string_view error) {
    return jsonResponse(status, {{"success", false}, {"error", std::string(error)}});
}

std::string RequestHandler::extractToken(const Request& req) const {
    auto auth = req[http::field::authorization];
    if (auth.empty()) return {};

    constexpr std::string_view prefix = "Bearer ";
    if (auth.substr(0, prefix.size()) == prefix)
        return std::string(auth.substr(prefix.size()));
    return {};
}

} // namespace cppplace
