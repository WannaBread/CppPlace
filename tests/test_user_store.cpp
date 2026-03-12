#include <gtest/gtest.h>
#include "services/UserStore.hpp"

using namespace cppplace;

class UserStoreTest : public ::testing::Test {
protected:
    UserStore store;
};

TEST_F(UserStoreTest, RegisterNewUser) {
    auto result = store.registerUser("alice", "password123");
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(store.userExists("alice"));
    EXPECT_EQ(store.userCount(), 1u);
}

TEST_F(UserStoreTest, RegisterDuplicateUsername) {
    store.registerUser("alice", "password123");
    auto result = store.registerUser("alice", "other_password");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), ErrorCode::UsernameTaken);
}

TEST_F(UserStoreTest, RegisterEmptyUsername) {
    auto result = store.registerUser("", "password");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), ErrorCode::InvalidCredentials);
}

TEST_F(UserStoreTest, RegisterEmptyPassword) {
    auto result = store.registerUser("alice", "");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), ErrorCode::InvalidCredentials);
}

TEST_F(UserStoreTest, AuthenticateValidUser) {
    store.registerUser("alice", "password123");
    auto result = store.authenticate("alice", "password123");
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(result.value(), "alice");
}

TEST_F(UserStoreTest, AuthenticateWrongPassword) {
    store.registerUser("alice", "password123");
    auto result = store.authenticate("alice", "wrong_password");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), ErrorCode::InvalidCredentials);
}

TEST_F(UserStoreTest, AuthenticateNonExistentUser) {
    auto result = store.authenticate("ghost", "password");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), ErrorCode::InvalidCredentials);
}

TEST_F(UserStoreTest, MultipleUsers) {
    store.registerUser("alice", "pass1");
    store.registerUser("bob", "pass2");
    store.registerUser("charlie", "pass3");
    EXPECT_EQ(store.userCount(), 3u);
    EXPECT_TRUE(store.userExists("bob"));
    EXPECT_FALSE(store.userExists("dave"));
}

TEST_F(UserStoreTest, PasswordIsHashed) {
    store.registerUser("alice", "password123");
    auto result = store.authenticate("alice", "password123");
    EXPECT_TRUE(result.ok());
}
