#include <QtGui/QImage>

#include "gtest/gtest.h"
#include "image_reader/floyd_steinberg_grayscale.h"

namespace image {

TEST(FloydSteinbergGrayscaleTest, HasCorrentNumColors) {
    QImage image(5, 5, QImage::Format_RGB32);
    ASSERT_EQ(0, image.numColors());
    FloydSteinbergGrayscale dithering_strategy(2);
    dithering_strategy.dither(&image);
    EXPECT_EQ(2, image.numColors());
}

TEST(FloydSteinbergGrayscaleTest, ClosestPaletteColor) {
    FloydSteinbergGrayscale dithering_strategy(2);
    EXPECT_EQ(1, dithering_strategy.closestPaletteColor(255));
    EXPECT_EQ(1, dithering_strategy.closestPaletteColor(128));
    EXPECT_EQ(0, dithering_strategy.closestPaletteColor(127));
    EXPECT_EQ(0, dithering_strategy.closestPaletteColor(0));
}

}  // namespace image
