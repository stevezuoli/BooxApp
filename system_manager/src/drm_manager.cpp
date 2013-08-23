
#include "drm_manager.h"

/// Communicate with drm service.
DRMManager::DRMManager()
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
    conf.DRMService(entry_);
}

DRMManager::~DRMManager()
{
}

bool DRMManager::start(const QStringList &strings)
{
    if (drm_service_)
    {
        return true;
    }

    drm_service_.reset(new QProcess);
    connect(drm_service_.get(), SIGNAL(finished(int,QProcess::ExitStatus)),
             this, SLOT(onProcessStopped(int, QProcess::ExitStatus)));
    drm_service_->start(entry_.app_name(), strings);
    if (!drm_service_->waitForStarted())
    {
        qDebug("Could not start %s", qPrintable(entry_.app_name()));
        return false;
    }
    return true;
}

bool DRMManager::stop()
{
    if (drm_service_)
    {
        drm_service_->kill();
        return true;
    }
    return false;
}

void DRMManager::onProcessStopped(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (drm_service_)
    {
        disconnect(drm_service_.get(), SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onProcessStopped(int, QProcess::ExitStatus)));
        drm_service_.reset(0);
    }
}

void DRMManager::onRequestDRMUserInfo(const QString &string, const QString & param)
{
    qDebug("Received requestDRMUserInfo signal.");
    emit requestDRMUserInfo(string, param);
}

void DRMManager::onFulfillmentFinished(const QString & string)
{
    qDebug("Received fulfillmentFinished signal.");
    emit fulfillmentFinished(string);
}

void DRMManager::onReportWorkflowError(const QString & workflow, const QString & error_code)
{
    qDebug("Received reportWorkflowError signal.");
    emit reportWorkflowError(workflow, error_code);
}

void DRMManager::onLoanReturnFinished(const QString &path)
{
    qDebug("Received loanReturnFinished signal.");
    emit loanReturnFinished(path);
}

void DRMManager::onServiceOwnerChanged(const QString & name,
                                       const QString & oldOwner,
                                       const QString & newOwner)
{
    if (oldOwner.size() <= 0 && newOwner.size() > 0)
    {
        if (name == entry_.service_name())
        {
            installDRMSlots(true);
        }
    }
    else if (oldOwner.size() > 0 && newOwner.size() <= 0)
    {
        if (name == entry_.service_name())
        {
            installDRMSlots(false);
        }
    }
}

bool DRMManager::installDRMSlots(bool connect)
{
    if (connect)
    {
        if (!connection_.connect(entry_.service_name(), QString(), entry_.interface_name(),
            "requestDRMUserInfo",
            this,
            SLOT(onRequestDRMUserInfo(const QString &, const QString &))))
        {
            qWarning("Could not connect to requestDRMUserInfo signal.");
            return false;
        }

        if (!connection_.connect(entry_.service_name(), QString(), entry_.interface_name(),
            "fulfillmentFinished",
            this,
            SLOT(onFulfillmentFinished(const QString &))))
        {
            qWarning("Could not connect to fulfillmentFinished signal.");
            return false;
        }

        if (!connection_.connect(entry_.service_name(), QString(), entry_.interface_name(),
            "loanReturnFinished",
            this,
            SLOT(onLoanReturnFinished(const QString &))))
        {
            qWarning("Could not connect to loanReturnFinished signal.");
            return false;
        }

        if (!connection_.connect(entry_.service_name(), QString(), entry_.interface_name(),
            "reportWorkflowError",
            this,
            SLOT(onReportWorkflowError(const QString &, const QString &))))
        {
            qWarning("Could not connect to reportWorkflowError signal.");
            return false;
        }
    }
    else
    {
        if (!connection_.disconnect(entry_.service_name(), QString(), entry_.interface_name(),
            "requestDRMUserInfo",
            this,
            SLOT(onRequestDRMUserInfo(const QString &, const QString &))))
        {
            qWarning("Could not disconnect to requestDRMUserInfo signal.");
            return false;
        }

        if (!connection_.disconnect(entry_.service_name(), QString(), entry_.interface_name(),
            "fulfillmentFinished",
            this,
            SLOT(onFulfillmentFinished(const QString &))))
        {
            qWarning("Could not disconnect to fulfillmentFinished signal.");
            return false;
        }

        if (!connection_.disconnect(entry_.service_name(), QString(), entry_.interface_name(),
            "loanReturnFinished",
            this,
            SLOT(onLoanReturnFinished(const QString &))))
        {
            qWarning("Could not disconnect to loanReturnFinished signal.");
            return false;
        }

        if (!connection_.disconnect(entry_.service_name(), QString(), entry_.interface_name(),
            "reportWorkflowError",
            this,
            SLOT(onReportWorkflowError(const QString &, const QString &))))
        {
            qWarning("Could not disconnect to reportWorkflowError signal.");
            return false;
        }
    }
    return true;
}

