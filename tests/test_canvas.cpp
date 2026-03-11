#include <gtest/gtest.h>
#include "models/Canvas.hpp"

using namespace cppplace;

TEST(CanvasTest, ConstructorCreatesCorrectSize) {
    Canvas canvas(10, 20);
    EXPECT_EQ(canvas.getWidth(), 10u);
    EXPECT_EQ(canvas.getHeight(), 20u);
}

TEST(CanvasTest, DefaultPixelIsZero) {
    Canvas canvas(5, 5);
    const auto& pixel = canvas.getPixel(0, 0);
    EXPECT_EQ(pixel.color_index, 0);
    EXPECT_TRUE(pixel.user_id.empty());
}

TEST(CanvasTest, SetAndGetPixel) {
    Canvas canvas(10, 10);
    canvas.setPixel(3, 4, 5, "user1");

    const auto& pixel = canvas.getPixel(3, 4);
    EXPECT_EQ(pixel.color_index, 5);
    EXPECT_EQ(pixel.user_id, "user1");
}

TEST(CanvasTest, OverwritePixel) {
    Canvas canvas(10, 10);
    canvas.setPixel(0, 0, 1, "user1");
    canvas.setPixel(0, 0, 2, "user2");

    const auto& pixel = canvas.getPixel(0, 0);
    EXPECT_EQ(pixel.color_index, 2);
    EXPECT_EQ(pixel.user_id, "user2");
}

TEST(CanvasTest, IsValidCoordInBounds) {
    Canvas canvas(10, 10);
    EXPECT_TRUE(canvas.isValidCoord(0, 0));
    EXPECT_TRUE(canvas.isValidCoord(9, 9));
    EXPECT_TRUE(canvas.isValidCoord(5, 5));
}

TEST(CanvasTest, IsValidCoordOutOfBounds) {
    Canvas canvas(10, 10);
    EXPECT_FALSE(canvas.isValidCoord(10, 0));
    EXPECT_FALSE(canvas.isValidCoord(0, 10));
    EXPECT_FALSE(canvas.isValidCoord(10, 10));
    EXPECT_FALSE(canvas.isValidCoord(100, 100));
}

TEST(CanvasTest, GetSnapshotReturnsAllPixels) {
    Canvas canvas(3, 3);
    canvas.setPixel(0, 0, 1, "a");
    canvas.setPixel(2, 2, 2, "b");

    auto snapshot = canvas.getSnapshot();
    EXPECT_EQ(snapshot.size(), 9u);
    EXPECT_EQ(snapshot[0].color_index, 1);  // (0,0)
    EXPECT_EQ(snapshot[8].color_index, 2);  // (2,2) = index 2*3+2=8
}

TEST(CanvasTest, PixelTimestampIsSet) {
    Canvas canvas(5, 5);
    auto before = std::chrono::system_clock::now();
    canvas.setPixel(1, 1, 3, "user");
    auto after = std::chrono::system_clock::now();

    const auto& pixel = canvas.getPixel(1, 1);
    EXPECT_GE(pixel.timestamp, before);
    EXPECT_LE(pixel.timestamp, after);
}

TEST(CanvasTest, CornerPixels) {
    Canvas canvas(100, 100);
    canvas.setPixel(0, 0, 1, "tl");
    canvas.setPixel(99, 0, 2, "tr");
    canvas.setPixel(0, 99, 3, "bl");
    canvas.setPixel(99, 99, 4, "br");

    EXPECT_EQ(canvas.getPixel(0, 0).user_id, "tl");
    EXPECT_EQ(canvas.getPixel(99, 0).user_id, "tr");
    EXPECT_EQ(canvas.getPixel(0, 99).user_id, "bl");
    EXPECT_EQ(canvas.getPixel(99, 99).user_id, "br");
}
