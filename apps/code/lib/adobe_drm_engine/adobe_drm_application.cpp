#include "dp_all.h"
#include "adobe_drm_application.h"
#include "adobe_drm_tasks.h"

namespace adobe_drm
{

AdobeDRMApplication::AdobeDRMApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , operation_(UNKNOWN_OPT)
#ifdef STANDALONE
    , main_window_(0, Qt::FramelessWindowHint)
#endif
{
#ifdef STANDALONE
    main_window_.showMinimized();
#endif
    int args_num = QCoreApplication::arguments().size();
    QString operation;
    if (args_num > 2)
    {
        operation = QCoreApplication::arguments().last();
        if (operation.toLower() == "fingerprint")
        {
            operation_ = GET_FINGERPRINT;
        }
        else if (operation.toLower() == "fulfill")
        {
            for (int i = 1; i < (QCoreApplication::arguments().size() - 1); i++)
            {
                fulfillment_urls_.push_back(QCoreApplication::arguments().at(i));
            }
            operation_ = FULFILL;
        }
        else if (operation.toLower() == "acsm")
        {
            for (int i = 1; i < (QCoreApplication::arguments().size() - 1); i++)
            {
                acsm_paths_.push_back(QCoreApplication::arguments().at(i));
            }
            operation_ = FULFILL;
        }
        else if (operation.toLower() == "return")
        {
            returning_doc_path_ = QString::fromLocal8Bit(argv[1]);
            operation_ = RETURN;
        }
    }

    connect(&drm_handler_,
            SIGNAL(reportWorkflowError(unsigned int, const QString &)),
            this,
            SLOT(onWorkflowError(unsigned int, const QString &)));
    connect(&drm_handler_,
            SIGNAL(reportWorkflowProgress(unsigned int, const QString &, double)),
            this,
            SLOT(onWorkflowProgress(unsigned int, const QString &, double)));
    connect(&drm_handler_,
            SIGNAL(reportWorkflowDone(unsigned int, const QByteArray &)),
            this,
            SLOT(onWorkflowDone(unsigned int, const QByteArray &)));
    connect(&drm_handler_,
            SIGNAL(reportACSMDownloadFinished(const QString &, const QUrl &)),
            this,
            SLOT(onACSMDownloaded(const QString &, const QUrl &)));
    connect(&drm_handler_,
            SIGNAL(reportACSMDownloadError()),
            this,
            SLOT(onACSMDownloadError()));

    connect(&drm_handler_,
            SIGNAL(activationFailed()),
            this,
            SLOT(onActivationFailed()));
    connect(&drm_handler_,
            SIGNAL(fulfillmentFinished(const QString &)),
            this,
            SLOT(onFulfillmentFinished(const QString &)));
    connect(&drm_handler_,
            SIGNAL(loanReturnFinished(const QString &)),
            this,
            SLOT(onLoanReturnFinished(const QString &)));
    connect(&drm_handler_,
            SIGNAL(loanTokenTransferFinished(const QString &)),
            this,
            SLOT(onLoanTokenTransferFinished(const QString &)));
    connect(&drm_handler_,
            SIGNAL(requestDRMUserInfo(const QString &)),
            this,
            SLOT(onRequestDRMUserInfo(const QString &)));
}

AdobeDRMApplication::~AdobeDRMApplication(void)
{
}

bool AdobeDRMApplication::execute()
{
    bool ret = false;
    switch (operation_)
    {
    case FULFILL:
        if (!fulfillment_urls_.isEmpty())
        {
            fulfillment_url_ = fulfillment_urls_.front();
            fulfillment_urls_.pop_front();
            if (!fulfillment_url_.isEmpty())
            {
                ret = fulfillByUrl(fulfillment_url_);
            }
        }
        else if (!acsm_paths_.isEmpty())
        {
            acsm_path_ = acsm_paths_.front();
            acsm_paths_.pop_front();
            if (!acsm_path_.isEmpty())
            {
                ret = fulfillByACSM(acsm_path_);
            }
        }
        break;
    case RETURN:
        if (!returning_doc_path_.isEmpty())
        {
            ret = loanReturn(returning_doc_path_);
        }
        break;
    case GET_FINGERPRINT:
        {
            QString fingerprint;
            getFingerprint(fingerprint);
            ret = false; // exit the application directly
        }
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

bool AdobeDRMApplication::fulfillByACSM(const QString & acsm)
{
    operation_ = FULFILL;
    if (acsm_path_.isEmpty())
    {
        acsm_path_ = acsm;
    }
    else if (acsm_path_ != acsm)
    {
        acsm_paths_.push_back(acsm);
        return true;
    }
    return fulfill(acsm_path_, QUrl());
}

bool AdobeDRMApplication::fulfillByUrl(const QString & url)
{
    operation_ = FULFILL;
    if (fulfillment_url_.isEmpty())
    {
        fulfillment_url_ = url;
    }
    else if (fulfillment_url_ != url)
    {
        fulfillment_urls_.push_back(url);
        return true;
    }

    QUrl real_url = network_service::guessUrlFromString(url);
    if (!real_url.isValid())
    {
        return false;
    }
    return downloadACSM(real_url);
}

bool AdobeDRMApplication::getFingerprint(QString & fingerprint)
{
    fingerprint = drm_handler_.getFingerprint();
    bool ret = fingerprint.isEmpty();
    return ret;
}

bool AdobeDRMApplication::downloadACSM(QUrl & url)
{
    if (url.isValid())
    {
        AdobeDownloadACSMTask *task = new AdobeDownloadACSMTask(url, &drm_handler_);
        task_handler_.addTask(task);
        return true;
    }
    return false;
}

bool AdobeDRMApplication::fulfill(const QString & acsm_path, const QUrl & url)
{
    if (!acsm_path.isEmpty())
    {
        AdobeFulfillTask *task = new AdobeFulfillTask(acsm_path, url, &drm_handler_);
        task_handler_.addTask(task);
        return true;
    }
    return false;
}

bool AdobeDRMApplication::loanReturn(const QString & loan_doc_path)
{
    if (!loan_doc_path.isEmpty())
    {
        operation_ = RETURN;
        if (returning_doc_path_.isEmpty())
        {
            returning_doc_path_ = loan_doc_path;
        }
        AdobeLoanReturnTask *task = new AdobeLoanReturnTask(loan_doc_path, &drm_handler_);
        task_handler_.addTask(task);
        return true;
    }
    return false;
}

bool AdobeDRMApplication::loanTokenTransfer(const QString & transfer_loan_token)
{
    if (!transfer_loan_token.isEmpty())
    {
        AdobeLoanTokenTranserTask *task = new AdobeLoanTokenTranserTask(transfer_loan_token, &drm_handler_);
        task_handler_.addTask(task);
        return true;
    }
    return false;
}

void AdobeDRMApplication::onRequestDRMUserInfo(const QString & url_str)
{
    QString param;
    switch (operation_)
    {
    case FULFILL:
        if (!fulfillment_url_.isEmpty())
        {
            param = fulfillment_url_;
        }
        else if (!acsm_path_.isEmpty())
        {
            param = acsm_path_;
        }
        break;
    case RETURN:
        if (!returning_doc_path_.isEmpty())
        {
            param = returning_doc_path_;
        }
        break;
    case GET_FINGERPRINT:
    default:
        break;
    }

#ifdef STANDALONE
    QUrl current_url(url_str);
    if (!current_url.isValid())
    {
        return;
    }
    QString host = current_url.host();

    PrivateConfig sys_conf;
    DRMInfo info;
    sys_conf.getDRMInfo(host, info);

    QString title = info.id().isEmpty() ? QApplication::tr("Activate Device") : QApplication::tr("Sign In");
    SignInDialog sign_in_dialog(0, title);
    if (sign_in_dialog.popup(info.id(), QString()) != QDialog::Accepted)
    {
        return;
    }
    info.set_id(sign_in_dialog.id());
    info.set_password(sign_in_dialog.password());
    info.set_url(host);
    sys_conf.updateDRMInfo(info);
    execute();
#else
    emit requestDRMUserInfo(url_str, param);
#endif
}

void AdobeDRMApplication::onWorkflowError(unsigned int workflow, const QString & error_code)
{
    QString kind;
    switch (workflow)
    {
    case dpdrm::DW_ACTIVATE:
        qWarning("Activation Fails:%s", error_code.toUtf8().constData());
        drm_handler_.onActivationFailed(error_code);
        kind = tr("Activation");
        break;
    case dpdrm::DW_AUTH_SIGN_IN:
        qWarning("Signing Fails:%s", error_code.toUtf8().constData());
        drm_handler_.onActivationFailed(error_code);
        kind = tr("Signing");
        break;
    case dpdrm::DW_DOWNLOAD:
        qWarning("Downloading Fails:%s", error_code.toUtf8().constData());
        kind = tr("Downloading");
        break;
    case dpdrm::DW_LOAN_RETURN:
        qWarning("Loan Return Fails:%s", error_code.toUtf8().constData());
        drm_handler_.onLoanReturnFailed(error_code);
        kind = tr("Loan Return");
        break;
    case dpdrm::DW_FULFILL:
        qWarning("Fulfillment Fails:%s", error_code.toUtf8().constData());
        drm_handler_.onFulfillmentFailed(error_code);
        kind = tr("Fulfillment");
        break;
    }
    emit reportWorkflowError(kind, error_code);
}

void AdobeDRMApplication::onWorkflowProgress(unsigned int workflows, const QString & title, double progress)
{
}

void AdobeDRMApplication::onWorkflowDone(unsigned int workflows, const QByteArray & follow_up)
{
    switch (workflows)
    {
    case dpdrm::DW_ACTIVATE:
        break;
    case dpdrm::DW_AUTH_SIGN_IN:
        break;
    case dpdrm::DW_DOWNLOAD:
        break;
    case dpdrm::DW_FULFILL:
        break;
    }
}

void AdobeDRMApplication::onACSMDownloaded(const QString & acsm_path, const QUrl & url)
{
    // append a fulfillment task
    fulfill(acsm_path, url);
}

void AdobeDRMApplication::onACSMDownloadError()
{
    nextTask();
}

void AdobeDRMApplication::nextTask()
{
    if (!execute())
    {
        fulfillment_url_.clear();
        acsm_path_.clear();
        QTimer::singleShot(5000, this, SLOT(tryQuit()));
    }
}

void AdobeDRMApplication::onActivationFailed()
{
    nextTask();
}

void AdobeDRMApplication::onFulfillmentFinished(const QString & path)
{
    emit fulfillmentFinished(path);
    nextTask();
}

void AdobeDRMApplication::onLoanReturnFinished(const QString & loan_id)
{
    emit loanReturnFinished(loan_id);
    nextTask();
}

void AdobeDRMApplication::onLoanTokenTransferFinished(const QString & transfer_loan_token)
{
    emit loanTokenTransferFinished(transfer_loan_token);
    nextTask();
}

void AdobeDRMApplication::tryQuit()
{
    if (task_handler_.isEmpty() && !drm_handler_.isBusy())
    {
        quit();
    }
}

void AdobeDRMApplicationAdaptor::onRequestDRMUserInfo(const QString & url_str, const QString & param)
{
    emit requestDRMUserInfo(url_str, param);
}

void AdobeDRMApplicationAdaptor::onFulfillmentFinished(const QString & path)
{
    qDebug("Fulfillment Finished, Doc Path:%s", qPrintable(path));
    emit fulfillmentFinished(path);
}

void AdobeDRMApplicationAdaptor::onLoanReturnFinished(const QString & path)
{
    qDebug("Loan Return Finished, Doc Path:%s", qPrintable(path));
    emit loanReturnFinished(path);
}

}
