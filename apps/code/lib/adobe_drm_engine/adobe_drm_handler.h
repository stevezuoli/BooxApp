#ifndef ADOBE_DRM_HANDLER_H_
#define ADOBE_DRM_HANDLER_H_

#include "adobe_drm_utils.h"
#include "adobe_timer.h"
#include "adobe_drm_device_info.h"
#include "private_conf/conf.h"

using namespace network_service;

namespace dpdrm
{
    class DRMProcessor;
};

namespace adobe_drm
{

class BusyHold;
class AdobeDRMHandlerPrivate;
class AdobeDRMHandler : public QObject
{
    Q_OBJECT
public:
    AdobeDRMHandler();
    ~AdobeDRMHandler();

    void init();
    inline bool isBusy() { return is_busy_; }

    dpdrm::DRMProcessor* processor();
    bool activate(const QUrl & url);
    bool activate(const DRMInfo & info);
    bool downloadACSM(const QUrl & url);
    bool fulfill(const QString & acsm, QString & fulfillment_path);
    bool loanReturn(const QString & loan_doc_path);
    bool loanTokenTransfer(const QString & transfer_loan_token);
    bool runPassHash(const QString & url, const QString & id, const QString & password);
    QString getFingerprint();

    void reportFulfillmentFinished(const QString & fulfillment_path);
    void reportLoanReturnFinished(const QString & loan_doc_path);
    void reportLoanTokenTransferFinished(const QString & transfer_loan_token);
    void reportActivationFailed();

Q_SIGNALS:
    void reportWorkflowProgress(unsigned int workflows, const QString & title, double progress);
    void reportWorkflowDone(unsigned int workflows, const QByteArray & follow_up);
    void reportWorkflowError(unsigned int workflow, const QString & error_code);
    void reportACSMDownloadFinished(const QString & acsm_path, const QUrl & url);
    void reportACSMDownloadError();
    void requestDRMUserInfo(const QString & url_str);

    void activationFailed();
    void fulfillmentFinished(const QString & fulfillment_path);
    void loanReturnFinished(const QString & loan_doc_path);
    void loanTokenTransferFinished(const QString & transfer_loan_token);

public Q_SLOTS:
    void onFulfillmentFailed(const QString & error);
    void onActivationFailed(const QString & error);
    void onLoanReturnFailed(const QString & error);

private Q_SLOTS:
    void onDownloadItemAdded(DownloadItem *item);
    void onACSMLoadFinished();
    void onACSMLoadError();

private:
    void notifyWorkflowProgress(unsigned int workflow, const QString & title, double progress);
    void notifyWorkflowDone(unsigned int workflows, const QByteArray & follow_up);
    void notifyWorkflowError(unsigned int workflow, const QString & error_code);

private:
    shared_ptr<AdobeDRMHandlerPrivate> data_;

    // device info
    AdobeDeviceInfo device_info_;

    // status
    bool is_busy_;
    bool activation_succeeds_;
    bool fulfillment_succeeds_;
    bool loan_return_succeeds_;

    // download manager
    DownloadManager download_manager_;
    DownloadItem *current_download_item_;

    friend class AdobeDRMProcessorClientPrivate;
    friend class BusyHold;
};

class BusyHold
{
public:
    BusyHold(AdobeDRMHandler *h) : drm_handler_(h) { drm_handler_->is_busy_ = true; }
    ~BusyHold() { drm_handler_->is_busy_ = false; }

private:
    AdobeDRMHandler *drm_handler_;
};

};
#endif
