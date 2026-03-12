#include "services/CanvasService.hpp"

namespace cppplace {

CanvasService::CanvasService(size_t width, size_t height,
                             std::shared_ptr<Palette> palette,
                             std::shared_ptr<SessionManager> session_manager,
                             std::shared_ptr<CooldownManager> cooldown_manager,
                             std::shared_ptr<EventBus> event_bus)
    : canvas_(width, height),
      palette_(std::move(palette)),
      session_manager_(std::move(session_manager)),
      cooldown_manager_(std::move(cooldown_manager)),
      event_bus_(std::move(event_bus)) {}

Result<void> CanvasService::placePixel(std::string_view token, size_t x, size_t y, uint8_t color_index) {
    auto username_opt = session_manager_->validateSession(token);
    if (!username_opt.has_value()) {
        return Result<void>::failure(ErrorCode::Unauthorized, "Invalid or expired session");
    }
    const std::string& username = username_opt.value();

    if (!palette_->isValidColor(color_index)) {
        return Result<void>::failure(ErrorCode::InvalidColor, "Color index out of palette range");
    }

    
    if (!canvas_.isValidCoord(x, y)) {
        return Result<void>::failure(ErrorCode::InvalidCoordinates, "Coordinates out of canvas bounds");
    }
    

    if (!cooldown_manager_->canPlace(username)) {
        auto remaining = cooldown_manager_->getRemainingTime(username);
        return Result<void>::failure(ErrorCode::CooldownActive,
            "Cooldown active, " + std::to_string(remaining.count()) + " seconds remaining");
    }

    {
        boost::unique_lock<boost::shared_mutex> lock(canvas_mutex_);
        canvas_.setPixel(x, y, color_index, username);
    }

    cooldown_manager_->recordPlacement(username);

    event_bus_->publish(PixelPlacedEvent{x, y, color_index, username});

    return Result<void>::success();
}

std::vector<Pixel> CanvasService::getCanvasState() const {
    boost::shared_lock<boost::shared_mutex> lock(canvas_mutex_);
    return canvas_.getSnapshot();
}

size_t CanvasService::getOnlineCount() const {
    boost::shared_lock<boost::shared_mutex> lock(online_mutex_);
    return online_tokens_.size();
}

void CanvasService::connectUser(std::string_view token) {
    auto username = session_manager_->validateSession(token);
    if (!username.has_value()) return;

    size_t count;
    {
        boost::unique_lock<boost::shared_mutex> lock(online_mutex_);
        online_tokens_.insert(std::string(token));
        count = online_tokens_.size();
    }

    event_bus_->publish(UserCountChangedEvent{count});
}

void CanvasService::disconnectUser(std::string_view token) {
    size_t count;
    {
        boost::unique_lock<boost::shared_mutex> lock(online_mutex_);
        online_tokens_.erase(std::string(token));
        count = online_tokens_.size();
    }

    event_bus_->publish(UserCountChangedEvent{count});
}

}
