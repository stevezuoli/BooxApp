#ifndef ADOBE_DRM_APPLICATION_H_
#define ADOBE_DRM_APPLICATION_H_

#include "adobe_drm_utils.h"
#include "adobe_drm_handler.h"

using namespace ui;
using namespace vbf;

namespace adobe_drm
{

#ifdef WIN32
#define STANDALONE
#endif

/// Adobe DRM Application
class AdobeDRMApplication : public QApplication
{
    Q_OBJECT
public:
    AdobeDRMApplication(int &argc, char **argv);
    ~AdobeDRMApplication(void);

    bool execute();

Q_SIGNALS:
    void requestDRMUserInfo(const QString & url_str, const QString & param);
    void fulfillmentFinished(const QString & path);
    void loanReturnFinished(const QString & loan_doc_path);
    void loanTokenTransferFinished(const QString & transfer_loan_token);
    void reportWorkflowError(const QString & workflow, const QString & error_code);

public Q_SLOTS:
    bool fulfillByACSM(const QString & acsm);
    bool fulfillByUrl(const QString & url);
    bool loanReturn(const QString & loan_doc_path);
    bool loanTokenTransfer(const QString & transfer_loan_token);
    bool getFingerprint(QString & fingerprint);

private Q_SLOTS:
    void onWorkflowError(unsigned int workflow, const QString & error_code);
    void onWorkflowProgress(unsigned int workflows, const QString & title, double progress);
    void onWorkflowDone(unsigned int workflows, const QByteArray & follow_up);
    void onACSMDownloaded(const QString & acsm_path, const QUrl & url);
    void onACSMDownloadError();
    void onRequestDRMUserInfo(const QString & url_str);

    void onActivationFailed();
    void onFulfillmentFinished(const QString & path);
    void onLoanReturnFinished(const QString & loan_doc_path);
    void onLoanTokenTransferFinished(const QString & transfer_loan_token);

    void tryQuit();

private:
    void nextTask();
    bool downloadACSM(QUrl & url);
    bool fulfill(const QString & acsm_path, const QUrl & url);

private:
    AdobeDRMHandler     drm_handler_;
    TasksHandler        task_handler_;
    AdobeDRMOperation   operation_;
#ifdef STANDALONE
    QMainWindow         main_window_;
#endif

    QStringList         fulfillment_urls_;
    QString             fulfillment_url_;
    QStringList         acsm_paths_;
    QString             acsm_path_;
    QString             returning_doc_path_;

    NO_COPY_AND_ASSIGN(AdobeDRMApplication);
};


class AdobeDRMApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;

    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.adobe_drm_handler");

public:
    AdobeDRMApplicationAdaptor(AdobeDRMApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().registerService("com.onyx.service.adobe_drm_handler");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/adobe_drm_handler", app_);

        connect(app_, SIGNAL(requestDRMUserInfo(const QString &, const QString &)),
                this, SLOT(onRequestDRMUserInfo(const QString &, const QString &)));
        connect(app_, SIGNAL(fulfillmentFinished(const QString &)),
                this, SLOT(onFulfillmentFinished(const QString &)));
        connect(app_, SIGNAL(loanReturnFinished(const QString &)),
                this, SLOT(onLoanReturnFinished(const QString &)));
        connect(app_, SIGNAL(reportWorkflowError(const QString &, const QString &)),
                      SIGNAL(reportWorkflowError(const QString &, const QString &)));
    }

Q_SIGNALS:
    void requestDRMUserInfo(const QString & url_str, const QString & param);
    void fulfillmentFinished(const QString & path);
    void loanReturnFinished(const QString & path);
    void reportWorkflowError(const QString & workflow, const QString & error_code);

public Q_SLOTS:
    bool fulfillByACSM(const QString & acsm_path) { return app_->fulfillByACSM(acsm_path); }
    bool fulfillByUrl(const QString & url) { return app_->fulfillByUrl(url); }
    bool loanReturn(const QString & loan_doc_path) { return app_->loanReturn(loan_doc_path); }
    bool loanTokenTransfer(const QString & transfer_loan_token) { return app_->loanTokenTransfer(transfer_loan_token); }

    void onRequestDRMUserInfo(const QString & url_str, const QString & param);
    void onFulfillmentFinished(const QString & path);
    void onLoanReturnFinished(const QString & path);

private:
    AdobeDRMApplication *app_;
    NO_COPY_AND_ASSIGN(AdobeDRMApplicationAdaptor);

};  // AdobeDRMApplicationAdaptor

};

#endif
