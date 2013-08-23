#ifndef IMAGEVIEWER_DITHERING_STRATEGY_H_
#define IMAGEVIEWER_DITHERING_STRATEGY_H_

class QImage;

namespace image {

// An abstract interface for image dithering strategies.
class DitheringStrategy {
  public:
    DitheringStrategy() {}
    virtual ~DitheringStrategy() {}
    virtual void dither(QImage*) = 0;
};

}  // namespace image

#endif  // IMAGEVIEWER_DITHERING_STRATEGY_H_
