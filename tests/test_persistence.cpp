#include <gtest/gtest.h>
#include "services/PersistenceService.hpp"
#include <filesystem>
#include <fstream>

using namespace cppplace;

class PersistenceTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_path = std::filesystem::temp_directory_path() / "cppplace_test_canvas.bin";
    }

    void TearDown() override {
        std::filesystem::remove(test_path);
        std::filesystem::remove(std::string(test_path.string()) + ".tmp");
    }

    std::filesystem::path test_path;
};

TEST_F(PersistenceTest, SaveAndLoadRoundTrip) {
    Canvas canvas(5, 5);
    canvas.setPixel(0, 0, 1, "alice");
    canvas.setPixel(4, 4, 15, "bob");
    canvas.setPixel(2, 3, 8, "charlie");

    ASSERT_TRUE(PersistenceService::saveCanvas(canvas, test_path.string()));

    auto loaded = PersistenceService::loadCanvas(test_path.string());
    ASSERT_TRUE(loaded.has_value());

    EXPECT_EQ(loaded->getWidth(), 5u);
    EXPECT_EQ(loaded->getHeight(), 5u);

    EXPECT_EQ(loaded->getPixel(0, 0).color_index, 1);
    EXPECT_EQ(loaded->getPixel(0, 0).user_id, "alice");

    EXPECT_EQ(loaded->getPixel(4, 4).color_index, 15);
    EXPECT_EQ(loaded->getPixel(4, 4).user_id, "bob");

    EXPECT_EQ(loaded->getPixel(2, 3).color_index, 8);
    EXPECT_EQ(loaded->getPixel(2, 3).user_id, "charlie");
}

TEST_F(PersistenceTest, LoadNonExistentFile) {
    auto loaded = PersistenceService::loadCanvas("/nonexistent/path/canvas.bin");
    EXPECT_FALSE(loaded.has_value());
}

TEST_F(PersistenceTest, LoadCorruptedFile) {
    // Write garbage data
    {
        std::ofstream ofs(test_path, std::ios::binary);
        ofs << "this is not a valid canvas file";
    }

    auto loaded = PersistenceService::loadCanvas(test_path.string());
    EXPECT_FALSE(loaded.has_value());
}

TEST_F(PersistenceTest, EmptyCanvas) {
    Canvas canvas(1, 1);

    ASSERT_TRUE(PersistenceService::saveCanvas(canvas, test_path.string()));

    auto loaded = PersistenceService::loadCanvas(test_path.string());
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->getWidth(), 1u);
    EXPECT_EQ(loaded->getHeight(), 1u);
    EXPECT_EQ(loaded->getPixel(0, 0).color_index, 0);
}

TEST_F(PersistenceTest, LargeCanvas) {
    Canvas canvas(100, 100);
    for (size_t y = 0; y < 100; ++y) {
        for (size_t x = 0; x < 100; ++x) {
            canvas.setPixel(x, y, static_cast<uint8_t>((x + y) % 16), "user" + std::to_string(x));
        }
    }

    ASSERT_TRUE(PersistenceService::saveCanvas(canvas, test_path.string()));

    auto loaded = PersistenceService::loadCanvas(test_path.string());
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->getWidth(), 100u);
    EXPECT_EQ(loaded->getHeight(), 100u);

    // Spot check
    EXPECT_EQ(loaded->getPixel(50, 50).color_index, (50 + 50) % 16);
    EXPECT_EQ(loaded->getPixel(50, 50).user_id, "user50");
}

TEST_F(PersistenceTest, OverwriteExistingFile) {
    Canvas canvas1(3, 3);
    canvas1.setPixel(0, 0, 1, "first");
    PersistenceService::saveCanvas(canvas1, test_path.string());

    Canvas canvas2(5, 5);
    canvas2.setPixel(0, 0, 2, "second");
    PersistenceService::saveCanvas(canvas2, test_path.string());

    auto loaded = PersistenceService::loadCanvas(test_path.string());
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->getWidth(), 5u);
    EXPECT_EQ(loaded->getPixel(0, 0).color_index, 2);
    EXPECT_EQ(loaded->getPixel(0, 0).user_id, "second");
}
