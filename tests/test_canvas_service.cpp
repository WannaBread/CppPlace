#include <gtest/gtest.h>
#include "services/CanvasService.hpp"
#include <memory>

using namespace cppplace;

class CanvasServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        palette = std::make_shared<Palette>(Palette::createDefault());
        session_manager = std::make_shared<SessionManager>();
        // Use 0 second cooldown for most tests to avoid waits
        cooldown_manager = std::make_shared<CooldownManager>(std::chrono::seconds(0));
        event_bus = std::make_shared<EventBus>();

        service = std::make_unique<CanvasService>(
            10, 10, palette, session_manager, cooldown_manager, event_bus
        );

        // Register a test user
        token = session_manager->createSession("testuser");
    }

    std::shared_ptr<Palette> palette;
    std::shared_ptr<SessionManager> session_manager;
    std::shared_ptr<CooldownManager> cooldown_manager;
    std::shared_ptr<EventBus> event_bus;
    std::unique_ptr<CanvasService> service;
    std::string token;
};

TEST_F(CanvasServiceTest, PlacePixelSuccess) {
    auto result = service->placePixel(token, 5, 5, 1);
    EXPECT_TRUE(result.ok());

    auto state = service->getCanvasState();
    // 5 + 5 * 10 = 55
    EXPECT_EQ(state[55].color_index, 1);
    EXPECT_EQ(state[55].user_id, "testuser");
}

TEST_F(CanvasServiceTest, PlacePixelUnauthorized) {
    auto result = service->placePixel("invalid-token", 5, 5, 1);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), ErrorCode::Unauthorized);
}

TEST_F(CanvasServiceTest, PlacePixelInvalidColor) {
    auto result = service->placePixel(token, 5, 5, 20); // palette has 16 colors
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), ErrorCode::InvalidColor);
}

TEST_F(CanvasServiceTest, PlacePixelInvalidCoordinates) {
    auto result = service->placePixel(token, 100, 100, 1);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), ErrorCode::InvalidCoordinates);
}

TEST_F(CanvasServiceTest, PlacePixelCooldownActive) {
    // Create a service with a cooldown
    auto cd_manager = std::make_shared<CooldownManager>(std::chrono::seconds(60));
    CanvasService cd_service(10, 10, palette, session_manager, cd_manager, event_bus);

    auto result1 = cd_service.placePixel(token, 0, 0, 1);
    EXPECT_TRUE(result1.ok());

    auto result2 = cd_service.placePixel(token, 1, 1, 2);
    EXPECT_FALSE(result2.ok());
    EXPECT_EQ(result2.code(), ErrorCode::CooldownActive);
}

TEST_F(CanvasServiceTest, PlacePixelNotifiesSubscribers) {
    bool notified = false;
    event_bus->subscribe([&](const Event& event) {
        if (auto* e = std::get_if<PixelPlacedEvent>(&event)) {
            EXPECT_EQ(e->x, 3u);
            EXPECT_EQ(e->y, 4u);
            EXPECT_EQ(e->color_index, 5);
            EXPECT_EQ(e->username, "testuser");
            notified = true;
        }
    });

    service->placePixel(token, 3, 4, 5);
    EXPECT_TRUE(notified);
}

TEST_F(CanvasServiceTest, GetCanvasState) {
    auto state = service->getCanvasState();
    EXPECT_EQ(state.size(), 100u); // 10x10
}

TEST_F(CanvasServiceTest, ConnectDisconnectUser) {
    EXPECT_EQ(service->getOnlineCount(), 0u);

    service->connectUser(token);
    EXPECT_EQ(service->getOnlineCount(), 1u);

    service->disconnectUser(token);
    EXPECT_EQ(service->getOnlineCount(), 0u);
}

TEST_F(CanvasServiceTest, ConnectNotifiesUserCount) {
    size_t received_count = 0;
    event_bus->subscribe([&](const Event& event) {
        if (auto* e = std::get_if<UserCountChangedEvent>(&event)) {
            received_count = e->count;
        }
    });

    service->connectUser(token);
    EXPECT_EQ(received_count, 1u);

    service->disconnectUser(token);
    EXPECT_EQ(received_count, 0u);
}

TEST_F(CanvasServiceTest, ConnectInvalidTokenIgnored) {
    service->connectUser("invalid-token");
    EXPECT_EQ(service->getOnlineCount(), 0u);
}

TEST_F(CanvasServiceTest, GetWidthAndHeight) {
    EXPECT_EQ(service->getWidth(), 10u);
    EXPECT_EQ(service->getHeight(), 10u);
}

TEST_F(CanvasServiceTest, MultiplePlacementsNoCooldown) {
    // With 0 cooldown, multiple placements should all succeed
    for (int i = 0; i < 5; ++i) {
        auto result = service->placePixel(token, i, 0, static_cast<uint8_t>(i));
        EXPECT_TRUE(result.ok()) << "Placement " << i << " failed";
    }
}
