
#ifndef MESSENGER_MANAGER_H_
#define MESSENGER_MANAGER_H_

#include <QtCore/QtCore>
#include "onyx/base/base.h"
#include "onyx/base/dbus.h"
#include "onyx/sys/sys.h"
#include "service_watcher.h"

/// Messenger manager to control messenger.
class MessengerManager : public QObject
{
    Q_OBJECT

public:
    MessengerManager();
    ~MessengerManager();

public Q_SLOTS:
    bool start(const QStringList &all = QStringList());
    bool stop();

private Q_SLOTS:
    void onProcessStopped(int exitCode, QProcess::ExitStatus exitStatus);

private Q_SLOTS:
    void onServiceOwnerChanged(const QString &, const QString &, const QString&);

private:
    bool installSlots(bool);

private:
    scoped_ptr<QProcess> messenger_service_;
    QDBusConnection connection_;    ///< Connection to messenger service.
    ServiceWatcher watcher_;
    Service entry_;
};


#endif // MESSENGER_MANAGER_H_

