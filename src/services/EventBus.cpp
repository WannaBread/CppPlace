#include "services/EventBus.hpp"

namespace cppplace {

SubscriptionId EventBus::subscribe(EventCallback callback) {
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    SubscriptionId id = next_id_++;
    subscribers_[id] = std::move(callback);
    return id;
}

void EventBus::unsubscribe(SubscriptionId id) {
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    subscribers_.erase(id);
}

void EventBus::publish(const Event& event) {
    std::vector<EventCallback> callbacks;
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        callbacks.reserve(subscribers_.size());
        for (const auto& [id, cb] : subscribers_) {
            callbacks.push_back(cb);
        }
    }
    for (const auto& cb : callbacks) {
        cb(event);
    }
}

size_t EventBus::subscriberCount() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    return subscribers_.size();
}

}
