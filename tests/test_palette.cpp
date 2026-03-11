#include <gtest/gtest.h>
#include "services/Palette.hpp"

using namespace cppplace;

TEST(PaletteTest, CreateDefaultHas16Colors) {
    auto palette = Palette::createDefault();
    EXPECT_EQ(palette.size(), 16u);
}

TEST(PaletteTest, ValidColorIndex) {
    auto palette = Palette::createDefault();
    EXPECT_TRUE(palette.isValidColor(0));
    EXPECT_TRUE(palette.isValidColor(15));
    EXPECT_FALSE(palette.isValidColor(16));
    EXPECT_FALSE(palette.isValidColor(255));
}

TEST(PaletteTest, GetColorReturnsCorrectColor) {
    auto palette = Palette::createDefault();
    const auto& white = palette.getColor(0);
    EXPECT_EQ(white.r, 255);
    EXPECT_EQ(white.g, 255);
    EXPECT_EQ(white.b, 255);
    EXPECT_EQ(white.name, "White");
}

TEST(PaletteTest, GetColorOutOfRange) {
    auto palette = Palette::createDefault();
    EXPECT_THROW(palette.getColor(16), std::out_of_range);
}

TEST(PaletteTest, CustomPalette) {
    Palette palette({{255, 0, 0, "Red"}, {0, 255, 0, "Green"}});
    EXPECT_EQ(palette.size(), 2u);
    EXPECT_TRUE(palette.isValidColor(0));
    EXPECT_TRUE(palette.isValidColor(1));
    EXPECT_FALSE(palette.isValidColor(2));
}

TEST(PaletteTest, EmptyPaletteThrows) {
    EXPECT_THROW(Palette({}), std::invalid_argument);
}

TEST(PaletteTest, GetColorsReturnsAll) {
    auto palette = Palette::createDefault();
    const auto& colors = palette.getColors();
    EXPECT_EQ(colors.size(), 16u);
    EXPECT_EQ(colors[0].name, "White");
    EXPECT_EQ(colors[3].name, "Black");
}
