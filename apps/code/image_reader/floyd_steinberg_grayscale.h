#ifndef IMAGEVIEWER_FLOYD_STENBERG_GRAYSCALE_H_
#define IMAGEVIEWER_FLOYD_STENBERG_GRAYSCALE_H_

#include <QtGui/QImage>

#include "onyx/base/base.h"
#include "gtest/gtest_prod.h"
#include "dithering_strategy.h"

namespace image {

// Implement Floyd-Steinburg dithering using a fixed grayscale
// palette_. This targets screens only capable of displaying certain
// levels of grayscale, so it doesn't make sense to use an optimized
// palette. But it is possible to improve the contrast of images by
// normalizing the original colors before dithering.
class FloydSteinbergGrayscale : public DitheringStrategy {
  public:
    explicit FloydSteinbergGrayscale(unsigned int level);
    virtual ~FloydSteinbergGrayscale();
    virtual void dither(QImage* image);
  private:
    unsigned int closestPaletteColor(long long rgb);
    QVector<QRgb> palette_;

    FRIEND_TEST(FloydSteinbergGrayscaleTest, ClosestPaletteColor);
};

}  // namespace image

#endif  // IMAGEVIEWER_FLOYD_STENBERG_GRAYSCALE_H_
