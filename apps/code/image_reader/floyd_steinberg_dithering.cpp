#include "image_reader/floyd_steinberg_dithering.h"

#include <QtGui/QColor>
#include <QtGui/QImage>

namespace image {

namespace {

// An image needs dithering if it has more than numColors colors.
bool imageNeedsDithering(int numColors, QImage* image);

}  // namespace

FloydSteinbergDithering::FloydSteinbergDithering(unsigned int palette_size)
    : DitheringStrategy()
    , palette_(palette_size)
{
    for (int i = 0; i < palette_.size(); i++)
    {
        palette_[i] = qRgb(i, i, i);
    }
}

FloydSteinbergDithering::~FloydSteinbergDithering()
{
}

/**
 * dither 8 bit grayscale bitmap so that only upper 2 bits are meaningful
 */
void dither8_2_4x4_bayer( unsigned char * base, int width, int height, int stride )
{
    // since we already have 2 bits per pixels (4 levels of gray), and 4x4 dither matrix
    // gives us another 17 the total is 4*17=68 levels which is probably satisfactory

    // we use Bayer dithering, *NOT* cluster dithering, since it is more appropriate
    // for eBook screen (no dot gain, unlike printers and relatively low resolution, so
    // that 4x4 matrix is clearly visible. Research the tradeoffs before tweaking the matrix!

    // here is our logical dither matrix 4x4:
    // 11/16  7/16 10/16  6/16
    //  3/16 15/16  2/16 14/16
    //  9/16  5/16 12/16  8/16
    //  1/16 13/16  4/16 16/16

    const size_t DITHER_CELL_WIDTH = 4;
    const size_t DITHER_CELL_HEIGHT = 4;

    const unsigned char dither_matrix[DITHER_CELL_WIDTH*DITHER_CELL_HEIGHT] =
    {
        (10*0x40+8)/16,  (6*0x40+8)/16,  (9*0x40+8)/16, ( 5*0x40+8)/16,
         (2*0x40+8)/16, (14*0x40+8)/16,  (1*0x40+8)/16, (13*0x40+8)/16,
         (8*0x40+8)/16,  (4*0x40+8)/16, (11*0x40+8)/16,  (7*0x40+8)/16,
         (0*0x40+8)/16, (12*0x40+8)/16,  (3*0x40+8)/16, (15*0x40+8)/16
    };

    unsigned char * vEnd = base + height*stride;
    unsigned char * hEnd = base + width;

    const unsigned char * dither = dither_matrix;
    const unsigned char * ditherEnd = dither_matrix + DITHER_CELL_WIDTH*DITHER_CELL_HEIGHT;

    while( base != vEnd )
    {
        unsigned char * p = base;
        const unsigned char * d = dither;
        const unsigned char * dEnd = dither + DITHER_CELL_WIDTH;
        while( p < hEnd )
        {
            unsigned int r = *p + *(d++);
            if( r > 0xFF )
            {
                r = 0xFF;
            }
            *(p++) = (unsigned char)r;
            if( d == dEnd )
            {
                d = dither;
            }
        }

        // next row
        base += stride;
        hEnd += stride;

        if( dEnd == ditherEnd )
        {
            dither = dither_matrix;
        }
        else
        {
            dither = dEnd;
        }
    }
}


void FloydSteinbergDithering::dither(QImage* image)
{
    if (!imageNeedsDithering(palette_.size(), image))
    {
        qDebug("Dithering not needed, skipping ...");
        return;
    }

    QImage grayscale_image;
    if (image->format() != QImage::Format_Indexed8)
    {
        grayscale_image.setColorTable(palette_);
        grayscale_image = image->convertToFormat(QImage::Format_Indexed8);
    }
    /*int width  = grayscale_image.width();
    int height = grayscale_image.height();
    int stride = grayscale_image.bytesPerLine();
    const uchar * buf = grayscale_image.bits();

    dither8_2_4x4_bayer((uchar*)buf, width, height, stride);*/
    *image = grayscale_image;
}

namespace
{

bool imageNeedsDithering(int numColors, QImage* image)
{
    if (image->numColors() == 0)
    {
        // This image doesn't have a palette
        int depth = image->depth(); // Only 1, 8, 32 are possible
        if (depth == 1)
        {
            return numColors < 2;
        }
        else if (depth == 8)
        {
            return numColors < 256;
        }
        else
        {
            return true;
        }
    }
    else
    {
        // This image has a palette
        return numColors < image->numColors();
    }
}

}  // namespace

}  // namespace image
