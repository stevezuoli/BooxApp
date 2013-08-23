#include "mount_tree_watcher.h"
#include <QApplication>

MountTreeWatcher::MountTreeWatcher()
{
    connect(&impl_, SIGNAL(USBChanged(bool, const QString &)),
        this, SLOT(onUSBChanged(bool, const QString &)));
    connect(&impl_, SIGNAL(SDChanged(bool, const QString &)),
        this, SLOT(onSDChanged(bool, const QString &)));
}

MountTreeWatcher::~MountTreeWatcher()
{
}

void MountTreeWatcher::onUSBChanged(bool mounted, const QString &mount_point)
{
    if (mounted)
    {
        qDebug("USB mounted!");
    }
    else
    {
        qDebug("USB gone!");
    }
}

void MountTreeWatcher::onSDChanged(bool mounted, const QString &mount_point)
{
    if (mounted)
    {
        qDebug("SD mounted!");
    }
    else
    {
        qDebug("SD gone!");
    }
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MountTreeWatcher watcher;
    return app.exec();
}


