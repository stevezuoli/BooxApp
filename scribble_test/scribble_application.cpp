#include "scribble_application.h"

ScribbleApplication::ScribbleApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    view_.reset(new ScribbleWidget(0));
    if (argc > 1)
    {
        current_path_ = QString::fromLocal8Bit(argv[1]);
    }
}

ScribbleApplication::~ScribbleApplication(void)
{
    close( current_path_ );
}

bool ScribbleApplication::open(const QString &path_name)
{
    view_->show();
    return true;
}

bool ScribbleApplication::close(const QString &path_name)
{
    return false;
}

void ScribbleApplication::onWakeUp()
{
}

void ScribbleApplication::onUSBSignal(bool inserted)
{
}

void ScribbleApplication::onSDChangedSignal(bool inserted)
{
}

void ScribbleApplication::onMountTreeSignal(bool inserted, const QString &mount_point)
{
}

void ScribbleApplication::onConnectToPCSignal(bool connected)
{
}

void ScribbleApplication::onBatterySignal(const int, const int, bool)
{
}

void ScribbleApplication::onSystemIdleSignal()
{
}

void ScribbleApplication::onAboutToShutDown()
{
}

void ScribbleApplication::onRotateScreen()
{
}

void ScribbleApplication::onScreenSizeChanged(int)
{
}
