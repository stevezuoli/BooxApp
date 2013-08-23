// -*- mode: c++; c-basic-offset: 4; -*-

#include <QtGui/QApplication>
#include <QString>

#include "system_manager.h"


int main(int argc, char* argv[])
{
    // We have to call this before Qt use the tslib plugin.
    // Otherwise, tslib can not initialize the touch screen.
    SystemManagerAdaptor::resetTouchScreen();
    qDebug("touch screen reset.");

    QApplication app(argc, argv);
    SystemManagerAdaptor adaptor(app);
    return app.exec();
}
