#include <QtGui/QtGui>
#include "my_widget.h"
#include "onyx/sys/sys_status.h"
#include "../inc/bs_screen_manager.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    sys::SysStatus::instance().setSystemBusy(false);
    sys::SysStatus::instance().enableIdle(false);

    MyWidget widget;
    widget.showMaximized();
    return app.exec();
}

