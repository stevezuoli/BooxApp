#ifndef DBUS_SERVICE_WATCHER_H_
#define DBUS_SERVICE_WATCHER_H_

#include "onyx/base/dbus.h"

/// When using Qt 4.6, we don't need this class.
class ServiceWatcher : public QObject
{
    Q_OBJECT

public:
    ServiceWatcher();
    ~ServiceWatcher();

public:
    bool watch(QDBusConnection & connection);

Q_SIGNALS:
    void serviceOwnerChanged(const QString &, const QString &, const QString&);

private Q_SLOTS:
    void onServiceOwnerChanged(const QString &name,
                               const QString &oldOwner,
                               const QString &newOwner);

};

#endif
