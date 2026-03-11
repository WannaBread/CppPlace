#include <gtest/gtest.h>
#include "services/CooldownManager.hpp"
#include <thread>

using namespace cppplace;

TEST(CooldownManagerTest, CanPlaceInitially) {
    CooldownManager manager(std::chrono::seconds(60));
    EXPECT_TRUE(manager.canPlace("alice"));
}

TEST(CooldownManagerTest, CannotPlaceAfterPlacement) {
    CooldownManager manager(std::chrono::seconds(60));
    manager.recordPlacement("alice");
    EXPECT_FALSE(manager.canPlace("alice"));
}

TEST(CooldownManagerTest, RemainingTimeInitiallyZero) {
    CooldownManager manager(std::chrono::seconds(60));
    EXPECT_EQ(manager.getRemainingTime("alice").count(), 0);
}

TEST(CooldownManagerTest, RemainingTimeAfterPlacement) {
    CooldownManager manager(std::chrono::seconds(60));
    manager.recordPlacement("alice");
    auto remaining = manager.getRemainingTime("alice");
    EXPECT_GT(remaining.count(), 0);
    EXPECT_LE(remaining.count(), 60);
}

TEST(CooldownManagerTest, ShortCooldownExpires) {
    CooldownManager manager(std::chrono::seconds(1));
    manager.recordPlacement("alice");
    EXPECT_FALSE(manager.canPlace("alice"));

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    EXPECT_TRUE(manager.canPlace("alice"));
}

TEST(CooldownManagerTest, DifferentUsersIndependentCooldowns) {
    CooldownManager manager(std::chrono::seconds(60));
    manager.recordPlacement("alice");
    EXPECT_FALSE(manager.canPlace("alice"));
    EXPECT_TRUE(manager.canPlace("bob"));
}

TEST(CooldownManagerTest, GetCooldownDuration) {
    CooldownManager manager(std::chrono::seconds(120));
    EXPECT_EQ(manager.getCooldownDuration().count(), 120);
}

TEST(CooldownManagerTest, ZeroCooldown) {
    CooldownManager manager(std::chrono::seconds(0));
    manager.recordPlacement("alice");
    EXPECT_TRUE(manager.canPlace("alice"));
}
