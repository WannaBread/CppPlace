#include <gtest/gtest.h>
#include "utils/PasswordHasher.hpp"

using namespace cppplace;

TEST(PasswordHasherTest, HashReturnsNonEmpty) {
    auto hash = PasswordHasher::hash("password123");
    EXPECT_FALSE(hash.empty());
}

TEST(PasswordHasherTest, HashIsDeterministic) {
    auto hash1 = PasswordHasher::hash("password123");
    auto hash2 = PasswordHasher::hash("password123");
    EXPECT_EQ(hash1, hash2);
}

TEST(PasswordHasherTest, DifferentPasswordsDifferentHashes) {
    auto hash1 = PasswordHasher::hash("password123");
    auto hash2 = PasswordHasher::hash("password456");
    EXPECT_NE(hash1, hash2);
}

TEST(PasswordHasherTest, VerifyCorrectPassword) {
    auto hash = PasswordHasher::hash("mypassword");
    EXPECT_TRUE(PasswordHasher::verify("mypassword", hash));
}

TEST(PasswordHasherTest, VerifyWrongPassword) {
    auto hash = PasswordHasher::hash("mypassword");
    EXPECT_FALSE(PasswordHasher::verify("wrong", hash));
}

TEST(PasswordHasherTest, EmptyPassword) {
    auto hash = PasswordHasher::hash("");
    EXPECT_FALSE(hash.empty());
    EXPECT_EQ(hash.size(), 40u);
}
