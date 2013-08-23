#include "image_reader/floyd_steinberg_grayscale.h"

#include <cassert>
#include <iostream>

#include <QtGui/QImage>

namespace image {

namespace {
inline unsigned int stepOfLevel(unsigned int level) {
    return 255 / (level - 1);
}
}

FloydSteinbergGrayscale::FloydSteinbergGrayscale(unsigned int level)
        : DitheringStrategy(),
          palette_(level) {
    assert(level > 1 && level <= 256);
    unsigned int gray = 0;
    unsigned int step = stepOfLevel(level);
    for (int i = 0; i < level - 1; ++i) {
        palette_[i] = qRgb(gray, gray, gray);
        gray += step;
    }
    palette_[level - 1] = qRgb(255, 255, 255);
}

FloydSteinbergGrayscale::~FloydSteinbergGrayscale() {
}

namespace {

inline void fillWithZero(vector<float>* v) {
    v->assign(v->size(), 0.0);
}

// An image needs dithering if it has more than numColors colors.
bool imageNeedsDithering(int numColors, QImage* image);

}  // namespace

void FloydSteinbergGrayscale::dither(QImage* image) {
    if (!imageNeedsDithering(palette_.size(), image)) {
        std::cout << "Dithering not needed, skipping ..." << std::endl;
        return;
    }
    QImage new_image(image->size(), QImage::Format_Indexed8);
    new_image.setColorTable(palette_);
    int width = image->width();
    int height = image->height();
    vector<float> corrections_this_row(width+1);
    vector<float> corrections_next_row(width+1);
    fillWithZero(&corrections_this_row);
    fillWithZero(&corrections_next_row);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            long long old_pixel = qGray(image->pixel(x, y)) +
                    static_cast<int>(corrections_this_row[x]);
            unsigned int new_pixel = closestPaletteColor(old_pixel);
            // If the following assertion fails, it probably means Qt
            // does not support the given palette size. In other
            // words, the earlier new_image.setColorTable(palette_)
            // call failed.
            assert(new_pixel < new_image.numColors());
            new_image.setPixel(x, y, new_pixel);
            float quant_error = old_pixel - qGray(palette_[new_pixel]);
            corrections_this_row[x+1] += ((7.0/16.0) * quant_error);
            if (x > 0) {
                corrections_next_row[x-1] += ((3.0/16.0) * quant_error);
            }
            corrections_next_row[x] += ((5.0/16.0) * quant_error);
            corrections_next_row[x+1] += ((1.0/16.0) * quant_error);
        }
        corrections_next_row.swap(corrections_this_row);
        fillWithZero(&corrections_next_row);
    }
    *image = new_image;
}

unsigned int FloydSteinbergGrayscale::closestPaletteColor(long long rgb) {
    if (rgb > 255) {
        rgb = 255;
    }
    if (rgb < 0) {
        rgb = 0;
    }
    unsigned int step = stepOfLevel(palette_.size());
    return (rgb + step/2) / step;
}

namespace {

bool imageNeedsDithering(int numColors, QImage* image) {
    if (image->numColors() == 0) {
        // This image doesn't have a palette
        int depth = image->depth(); // Only 1, 8, 32 are possible
        if (depth == 1) {
            return numColors < 2;
        } else if (depth == 8) {
            return numColors < 256;
        } else {
            return true;
        }
    } else { // This image has a palette
        return numColors < image->numColors();
    }
}

}  // namespace


}  // namespace image
