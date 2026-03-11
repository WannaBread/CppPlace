#include <gtest/gtest.h>
#include "services/EventBus.hpp"
#include <atomic>

using namespace cppplace;

TEST(EventBusTest, SubscribeAndPublish) {
    EventBus bus;
    bool received = false;

    bus.subscribe([&](const Event& event) {
        if (std::holds_alternative<PixelPlacedEvent>(event)) {
            received = true;
        }
    });

    bus.publish(PixelPlacedEvent{1, 2, 3, "alice"});
    EXPECT_TRUE(received);
}

TEST(EventBusTest, MultipleSubscribers) {
    EventBus bus;
    int count = 0;

    bus.subscribe([&](const Event&) { count++; });
    bus.subscribe([&](const Event&) { count++; });
    bus.subscribe([&](const Event&) { count++; });

    bus.publish(PixelPlacedEvent{0, 0, 0, "test"});
    EXPECT_EQ(count, 3);
}

TEST(EventBusTest, Unsubscribe) {
    EventBus bus;
    int count = 0;

    auto id = bus.subscribe([&](const Event&) { count++; });
    bus.publish(PixelPlacedEvent{0, 0, 0, "test"});
    EXPECT_EQ(count, 1);

    bus.unsubscribe(id);
    bus.publish(PixelPlacedEvent{0, 0, 0, "test"});
    EXPECT_EQ(count, 1); // Should not increment
}

TEST(EventBusTest, PixelPlacedEventData) {
    EventBus bus;
    size_t rx = 0, ry = 0;
    uint8_t rc = 0;
    std::string ru;

    bus.subscribe([&](const Event& event) {
        if (auto* e = std::get_if<PixelPlacedEvent>(&event)) {
            rx = e->x;
            ry = e->y;
            rc = e->color_index;
            ru = e->username;
        }
    });

    bus.publish(PixelPlacedEvent{5, 10, 7, "bob"});
    EXPECT_EQ(rx, 5u);
    EXPECT_EQ(ry, 10u);
    EXPECT_EQ(rc, 7);
    EXPECT_EQ(ru, "bob");
}

TEST(EventBusTest, UserCountChangedEvent) {
    EventBus bus;
    size_t received_count = 0;

    bus.subscribe([&](const Event& event) {
        if (auto* e = std::get_if<UserCountChangedEvent>(&event)) {
            received_count = e->count;
        }
    });

    bus.publish(UserCountChangedEvent{42});
    EXPECT_EQ(received_count, 42u);
}

TEST(EventBusTest, SubscriberCount) {
    EventBus bus;
    EXPECT_EQ(bus.subscriberCount(), 0u);

    auto id1 = bus.subscribe([](const Event&) {});
    EXPECT_EQ(bus.subscriberCount(), 1u);

    auto id2 = bus.subscribe([](const Event&) {});
    EXPECT_EQ(bus.subscriberCount(), 2u);

    bus.unsubscribe(id1);
    EXPECT_EQ(bus.subscriberCount(), 1u);

    bus.unsubscribe(id2);
    EXPECT_EQ(bus.subscriberCount(), 0u);
}

TEST(EventBusTest, UnsubscribeNonExistent) {
    EventBus bus;
    // Should not throw
    bus.unsubscribe(99999);
    EXPECT_EQ(bus.subscriberCount(), 0u);
}
