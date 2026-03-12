#pragma once

#include <boost/thread/shared_mutex.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <atomic>

namespace cppplace {

struct PixelPlacedEvent {
    size_t x;
    size_t y;
    uint8_t color_index;
    std::string username;
};

struct UserCountChangedEvent {
    size_t count;
};

using Event = std::variant<PixelPlacedEvent, UserCountChangedEvent>;
using EventCallback = std::function<void(const Event&)>;
using SubscriptionId = uint64_t;

class EventBus {
public:
    SubscriptionId subscribe(EventCallback callback);
    void unsubscribe(SubscriptionId id);
    void publish(const Event& event);

    size_t subscriberCount() const;

private:
    mutable boost::shared_mutex mutex_;
    std::unordered_map<SubscriptionId, EventCallback> subscribers_;
    std::atomic<SubscriptionId> next_id_{1};
};

}
