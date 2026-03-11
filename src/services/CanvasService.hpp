#pragma once

#include "models/Canvas.hpp"
#include "services/Palette.hpp"
#include "services/SessionManager.hpp"
#include "services/CooldownManager.hpp"
#include "services/EventBus.hpp"
#include "common/Result.hpp"

#include <boost/thread/shared_mutex.hpp>
#include <memory>
#include <set>

namespace cppplace {

class CanvasService {
public:
    CanvasService(size_t width, size_t height,
                  std::shared_ptr<Palette> palette,
                  std::shared_ptr<SessionManager> session_manager,
                  std::shared_ptr<CooldownManager> cooldown_manager,
                  std::shared_ptr<EventBus> event_bus);

    Result<void> placePixel(const std::string& token, size_t x, size_t y, uint8_t color_index);
    std::vector<Pixel> getCanvasState() const;
    size_t getOnlineCount() const;

    void connectUser(const std::string& token);
    void disconnectUser(const std::string& token);

    const Canvas& getCanvas() const { return canvas_; }
    size_t getWidth() const { return canvas_.getWidth(); }
    size_t getHeight() const { return canvas_.getHeight(); }

private:
    mutable boost::shared_mutex canvas_mutex_;
    mutable boost::shared_mutex online_mutex_;
    Canvas canvas_;
    std::shared_ptr<Palette> palette_;
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<CooldownManager> cooldown_manager_;
    std::shared_ptr<EventBus> event_bus_;
    std::set<std::string> online_tokens_;
};

} // namespace cppplace
