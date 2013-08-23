#include <iostream>
#include <string>

#include <QtCore/QString>
#include <QtGui/QImage>

#include <imageviewer/floyd_steinberg_dithering.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <image file>" << std::endl;
        return 1;
    }

    QImage image(argv[1]);
    image::FloydSteinbergDithering dithering_strategy(256);
    dithering_strategy.dither(&image);
    image.save(QString("gray16_") + argv[1]);
}
