// -*- mode: c++; c-basic-offset: 4; -*-

#include <iostream>
#include <unistd.h>

#include <QtGui/QApplication>

#include "baby_sitter.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    if (argc != 2)
    {
        std::cerr << "This program accepts exactly one argument." << std::endl;
        return 1;
    }
    BabySitter baby_sitter(argv[1]);
    baby_sitter.start();
    return app.exec();
}
