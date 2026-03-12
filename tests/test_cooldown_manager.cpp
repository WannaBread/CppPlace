#include <gtest/gtest.h>
#include "services/CooldownManager.hpp"
#include <boost/thread/thread.hpp>

using namespace cppplace;

TEST(CooldownManagerTest, CanPlaceInitially) {
    CooldownManager manager(std::chrono::duration<double>(60.0));
    EXPECT_TRUE(manager.canPlace("alice"));
}

TEST(CooldownManagerTest, CannotPlaceAfterPlacement) {
    CooldownManager manager(std::chrono::duration<double>(60.0));
    manager.recordPlacement("alice");
    EXPECT_FALSE(manager.canPlace("alice"));
}

TEST(CooldownManagerTest, RemainingTimeInitiallyZero) {
    CooldownManager manager(std::chrono::duration<double>(60.0));
    EXPECT_DOUBLE_EQ(manager.getRemainingTime("alice").count(), 0.0);
}

TEST(CooldownManagerTest, RemainingTimeAfterPlacement) {
    CooldownManager manager(std::chrono::duration<double>(60.0));
    manager.recordPlacement("alice");
    auto remaining = manager.getRemainingTime("alice");
    EXPECT_GT(remaining.count(), 0.0);
    EXPECT_LE(remaining.count(), 60.0);
}

TEST(CooldownManagerTest, ShortCooldownExpires) {
    CooldownManager manager(std::chrono::duration<double>(1.0));
    manager.recordPlacement("alice");
    EXPECT_FALSE(manager.canPlace("alice"));

    boost::this_thread::sleep_for(boost::chrono::milliseconds(1100));
    EXPECT_TRUE(manager.canPlace("alice"));
}

TEST(CooldownManagerTest, DifferentUsersIndependentCooldowns) {
    CooldownManager manager(std::chrono::duration<double>(60.0));
    manager.recordPlacement("alice");
    EXPECT_FALSE(manager.canPlace("alice"));
    EXPECT_TRUE(manager.canPlace("bob"));
}

TEST(CooldownManagerTest, GetCooldownDuration) {
    CooldownManager manager(std::chrono::duration<double>(120.0));
    EXPECT_DOUBLE_EQ(manager.getCooldownDuration().count(), 120.0);
}

TEST(CooldownManagerTest, ZeroCooldown) {
    CooldownManager manager(std::chrono::duration<double>(0.0));
    manager.recordPlacement("alice");
    EXPECT_TRUE(manager.canPlace("alice"));
}
