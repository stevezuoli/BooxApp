
#include "messenger_manager.h"

/// Communicate with messenger service.
MessengerManager::MessengerManager()
#ifndef _WINDOWS
: connection_(QDBusConnection::systemBus())
#else
: connection_(QDBusConnection::sessionBus())
#endif
{
    connect(&watcher_, SIGNAL(serviceOwnerChanged(const QString &,const QString &,const QString &)),
            this, SLOT(onServiceOwnerChanged(const QString &,const QString &,const QString &)));
    watcher_.watch(connection_);

    SystemConfig conf;
    conf.messengerService(entry_);
}

MessengerManager::~MessengerManager()
{
}

bool MessengerManager::start(const QStringList &strings)
{
    if (messenger_service_)
    {
        return true;
    }

    messenger_service_.reset(new QProcess);
    connect(messenger_service_.get(), SIGNAL(finished(int,QProcess::ExitStatus)),
             this, SLOT(onProcessStopped(int, QProcess::ExitStatus)));
    messenger_service_->start(entry_.app_name(), strings);
    if (!messenger_service_->waitForStarted())
    {
        qDebug("Could not start %s", qPrintable(entry_.app_name()));
        return false;
    }
    return true;
}

bool MessengerManager::stop()
{
    if (messenger_service_)
    {
        messenger_service_->kill();
        return true;
    }
    return false;
}

void MessengerManager::onProcessStopped(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (messenger_service_)
    {
        disconnect(messenger_service_.get(), SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onProcessStopped(int, QProcess::ExitStatus)));
        messenger_service_.reset(0);
    }
}

void MessengerManager::onServiceOwnerChanged(const QString & name,
                                       const QString & oldOwner,
                                       const QString & newOwner)
{
    if (oldOwner.size() <= 0 && newOwner.size() > 0)
    {
        if (name == entry_.service_name())
        {
            installSlots(true);
        }
    }
    else if (oldOwner.size() > 0 && newOwner.size() <= 0)
    {
        if (name == entry_.service_name())
        {
            installSlots(false);
        }
    }
}

bool MessengerManager::installSlots(bool connect)
{
    if (connect)
    {
    }
    else
    {
    }
    return true;
}

