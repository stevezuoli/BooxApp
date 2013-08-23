#include "service_watcher.h"

ServiceWatcher::ServiceWatcher()
{
}

ServiceWatcher::~ServiceWatcher()
{
}

bool ServiceWatcher::watch(QDBusConnection & connection)
{
    if (connection.isConnected())
    {
        QDBusConnectionInterface *iface = connection.interface();
        connect(iface, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                this, SLOT(onServiceOwnerChanged(QString,QString,QString)));
        return true;
    }
    else
    {
        qWarning("Cannot connect to D-Bus: %s" , qPrintable(connection.lastError().message()));
        return false;
    }
}

void ServiceWatcher::onServiceOwnerChanged(const QString &name,
                                           const QString &oldOwner,
                                           const QString &newOwner)
{
    emit serviceOwnerChanged(name, oldOwner, newOwner);
}
