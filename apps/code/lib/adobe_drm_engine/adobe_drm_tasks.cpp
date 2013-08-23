#include "adobe_drm_tasks.h"

namespace adobe_drm
{

AdobeFulfillTask::AdobeFulfillTask(const QString & acsm,
                                   const QUrl & url,
                                   AdobeDRMHandler *handler)
    : acsm_(acsm)
    , url_(url)
    , handler_(handler)
{
}

AdobeFulfillTask::~AdobeFulfillTask()
{
}

void AdobeFulfillTask::exec()
{
    start();
    QString fulfillment_path;
    if (handler_->activate(url_))
    {
        handler_->fulfill(acsm_, fulfillment_path);
        handler_->reportFulfillmentFinished(fulfillment_path);
    }
    else
    {
        handler_->reportActivationFailed();
    }
    abort();
}

AdobeLoanReturnTask::AdobeLoanReturnTask(const QString & loan_doc_path,
                                         AdobeDRMHandler *handler)
    : loan_doc_path_(loan_doc_path)
    , handler_(handler)
{
}

AdobeLoanReturnTask::~AdobeLoanReturnTask()
{
}

void AdobeLoanReturnTask::exec()
{
    start();
    if (handler_->activate(QUrl()))
    {
        handler_->loanReturn(loan_doc_path_);
        handler_->reportLoanReturnFinished(loan_doc_path_);
    }
    else
    {
        handler_->reportActivationFailed();
    }
    abort();
}

AdobeLoanTokenTranserTask::AdobeLoanTokenTranserTask(const QString token,
                                                     AdobeDRMHandler *handler)
    : transfer_loan_token_(token)
    , handler_(handler)
{
}

AdobeLoanTokenTranserTask::~AdobeLoanTokenTranserTask()
{
}

void AdobeLoanTokenTranserTask::exec()
{
    start();
    handler_->loanTokenTransfer(transfer_loan_token_);
    handler_->reportLoanTokenTransferFinished(transfer_loan_token_);
    abort();
}

AdobeDownloadACSMTask::AdobeDownloadACSMTask(const QUrl & url, AdobeDRMHandler *handler)
    : url_(url)
    , handler_(handler)
{
}

AdobeDownloadACSMTask::~AdobeDownloadACSMTask()
{
}

void AdobeDownloadACSMTask::exec()
{
    start();
    handler_->downloadACSM(url_);
    abort();
}


}
