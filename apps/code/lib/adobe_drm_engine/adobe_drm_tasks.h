#ifndef ADOBE_DRM_TASKS_H_
#define ADOBE_DRM_TASKS_H_

#include "adobe_drm_utils.h"
#include "adobe_drm_handler.h"

using namespace vbf;
namespace adobe_drm
{

class AdobeDownloadACSMTask : public BaseTask
{
public:
    AdobeDownloadACSMTask(const QUrl & url, AdobeDRMHandler *handler);
    virtual ~AdobeDownloadACSMTask();
    virtual void exec();

private:
    QUrl url_;
    AdobeDRMHandler *handler_;
};

class AdobeFulfillTask : public BaseTask
{
public:
    AdobeFulfillTask(const QString & acsm,
                     const QUrl & url,
                     AdobeDRMHandler *handler);
    virtual ~AdobeFulfillTask();
    virtual void exec();

private:
    QString acsm_;
    QUrl url_;
    AdobeDRMHandler *handler_;
};

class AdobeLoanReturnTask : public BaseTask
{
public:
    AdobeLoanReturnTask(const QString & loan_doc_path,
                        AdobeDRMHandler *handler);
    virtual ~AdobeLoanReturnTask();
    virtual void exec();

private:
    QString loan_doc_path_;
    AdobeDRMHandler *handler_;
};

class AdobeLoanTokenTranserTask : public BaseTask
{
public:
    AdobeLoanTokenTranserTask(const QString token,
                              AdobeDRMHandler *handler);
    virtual ~AdobeLoanTokenTranserTask();
    virtual void exec();

private:
    QString transfer_loan_token_;
    AdobeDRMHandler *handler_;
};

};

#endif
