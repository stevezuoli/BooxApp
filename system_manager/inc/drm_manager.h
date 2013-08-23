
#ifndef DRM_MANAGER_H_
#define DRM_MANAGER_H_

#include <QtCore/QtCore>
#include "onyx/base/base.h"
#include "onyx/base/dbus.h"
#include "onyx/sys/sys.h"
#include "service_watcher.h"

/// Communicate with drm service.
class DRMManager : public QObject
{
    Q_OBJECT

public:
    DRMManager();
    ~DRMManager();

public Q_SLOTS:
    bool start(const QStringList &all);
    bool stop();

private Q_SLOTS:
    void onProcessStopped(int exitCode, QProcess::ExitStatus exitStatus);

Q_SIGNALS:
    void requestDRMUserInfo(const QString &string, const QString & param);
    void fulfillmentFinished(const QString & string);
    void loanReturnFinished(const QString &path);
    void reportWorkflowError(const QString & workflow, const QString & error_code);

private Q_SLOTS:
    void onRequestDRMUserInfo(const QString &string, const QString & param);
    void onFulfillmentFinished(const QString & string);
    void onLoanReturnFinished(const QString &path);
    void onReportWorkflowError(const QString & workflow, const QString & error_code);
    void onServiceOwnerChanged(const QString &, const QString &, const QString&);

private:
    bool installDRMSlots(bool);

private:
    scoped_ptr<QProcess> drm_service_;
    QDBusConnection connection_;    ///< Connection to drm service.
    ServiceWatcher watcher_;
    Service entry_;
};


#endif // DRM_MANAGER_H_

