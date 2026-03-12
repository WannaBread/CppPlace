#include <gtest/gtest.h>
#include "services/SessionManager.hpp"

using namespace cppplace;

class SessionManagerTest : public ::testing::Test {
protected:
    SessionManager manager;
};

TEST_F(SessionManagerTest, CreateSessionReturnsToken) {
    auto token = manager.createSession("alice");
    EXPECT_FALSE(token.empty());
}

TEST_F(SessionManagerTest, ValidateExistingSession) {
    auto token = manager.createSession("alice");
    auto username = manager.validateSession(token);
    ASSERT_TRUE(username.has_value());
    EXPECT_EQ(username.value(), "alice");
}

TEST_F(SessionManagerTest, ValidateInvalidToken) {
    auto username = manager.validateSession("nonexistent-token");
    EXPECT_FALSE(username.has_value());
}

TEST_F(SessionManagerTest, RemoveSession) {
    auto token = manager.createSession("alice");
    EXPECT_TRUE(manager.removeSession(token));
    EXPECT_FALSE(manager.validateSession(token).has_value());
}

TEST_F(SessionManagerTest, RemoveNonExistentSession) {
    EXPECT_FALSE(manager.removeSession("fake-token"));
}

TEST_F(SessionManagerTest, MultipleSessions) {
    auto token1 = manager.createSession("alice");
    auto token2 = manager.createSession("bob");

    EXPECT_NE(token1, token2);
    EXPECT_EQ(manager.activeSessionCount(), 2u);

    auto user1 = manager.validateSession(token1);
    auto user2 = manager.validateSession(token2);
    EXPECT_EQ(user1.value(), "alice");
    EXPECT_EQ(user2.value(), "bob");
}

TEST_F(SessionManagerTest, MultipleSessionsSameUser) {
    auto token1 = manager.createSession("alice");
    auto token2 = manager.createSession("alice");

    EXPECT_NE(token1, token2);
    EXPECT_EQ(manager.activeSessionCount(), 2u);

    EXPECT_TRUE(manager.validateSession(token1).has_value());
    EXPECT_TRUE(manager.validateSession(token2).has_value());
}

TEST_F(SessionManagerTest, ActiveSessionCount) {
    EXPECT_EQ(manager.activeSessionCount(), 0u);
    auto token = manager.createSession("alice");
    EXPECT_EQ(manager.activeSessionCount(), 1u);
    manager.removeSession(token);
    EXPECT_EQ(manager.activeSessionCount(), 0u);
}
