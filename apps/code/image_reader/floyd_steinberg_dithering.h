#ifndef IMAGEVIEWER_FLOYD_STEINBERG_DITHERING_H_
#define IMAGEVIEWER_FLOYD_STEINBERG_DITHERING_H_

#include <QtGui/QImage>

#include "onyx/base/base.h"
#include "dithering_strategy.h"

class QImage;

namespace image {

// An implementation of the Floyd-Steinberg dithering algorithm,
// refactored out of ImageItem. We probably should not use a fixed
// 256-color palette, but generate a 16-color palette from the
// original palette instead.
class FloydSteinbergDithering : public DitheringStrategy
{
public:
    explicit FloydSteinbergDithering(unsigned int palette_size);
    virtual ~FloydSteinbergDithering();
    virtual void dither(QImage*);
private:
    QVector<QRgb> palette_;
};

}  // namespace image

#endif  // IMAGEVIEWER_FLOYD_STEINBERG_DITHERING_H_
