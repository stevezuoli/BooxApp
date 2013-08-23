#include "dp_all.h"
#include "adobe_curl_net_provider.h"
#include "adobe_drm_processor.h"
#include "adobe_drm_handler.h"

using namespace network_service;

namespace adobe_drm
{

static const QString DEFAULT_ACTIVATION_URL = "http://contentserver.adobe.com/";

struct FulfillmentItemInfo
{
    bool is_returnable;
    bool has_returned;
    dp::time_t expiration;

    FulfillmentItemInfo() : is_returnable(false), has_returned(false), expiration(0) {}
};

enum ActivationStatus
{
    HAS_VALID_ACTIVATION_REC = 0,
    ACTIVATION_OUT_OF_DATE,
    NO_OTA_FLAG,
    NO_ACTIVATION_REC
};

static bool isDRMInfoValid(const DRMInfo & info)
{
    return !info.id().isEmpty() && !info.password().isEmpty();
}

static const QString DEV_DIR = ".adobe-digital-editions";
static bool removeOriginActivationRecord()
{
    QString path;
#ifdef Q_WS_QWS
    path = ("/media/flash");
#else
    path = QDir::home().path();
#endif
    QDir dir(path);
    if (dir.cd(DEV_DIR))
    {
        path = dir.filePath("activation.xml");
        scoped_ptr<QFile> file(new QFile(path));
        return file->remove();
    }
    return false;
}

class AdobeDRMHandlerPrivate
{
public:
    AdobeDRMHandlerPrivate();
    ~AdobeDRMHandlerPrivate();

    void init();
    inline dpdrm::DRMProcessor * processor() { return drm_processor_; }
    inline void setProcessor(dpdrm::DRMProcessor * p) { drm_processor_ = p; }

    QString getFulfillmentItemPath(const int index,
                                   dpdrm::DRMProcessor * drm_processor,
                                   dpdev::Device * device);
    QString saveFulfillmentItems(dpdrm::DRMProcessor * drm_processor,
                                 dpdev::Device * device);
    ActivationStatus checkActivationRecords(const DRMInfo & usr_info,
                                            dpdrm::DRMProcessor * drm_processor,
                                            dpdev::Device * device);
    bool saveFulfillmentNode(const QString & path,
                             const QString & id,
                             const FulfillmentItemInfo & info);
    bool getFulfillmentItemInfo(const QString & doc_path,
                                QString & fulfillment_id,
                                FulfillmentItemInfo & info);

private:
    dp::time_t getExpirationDate(dp::ref<dpdrm::FulfillmentItem> item);

private:
    // network provider
    scoped_ptr<dpnet::NetProvider> net_provider_;

    // drm processor
    dpdrm::DRMProcessor *drm_processor_;

    // content manager
    cms::ContentManager cms_;

    // timer
    AdobeTimer master_timer_;
};

AdobeDRMHandlerPrivate::AdobeDRMHandlerPrivate()
    : net_provider_(0)
    , drm_processor_(0)
{
}

AdobeDRMHandlerPrivate::~AdobeDRMHandlerPrivate()
{
    cms_.close();
    if (drm_processor_ != 0)
    {
        drm_processor_->release();
    }
}

dp::time_t AdobeDRMHandlerPrivate::getExpirationDate(dp::ref<dpdrm::FulfillmentItem> item)
{
    dp::time_t expire_time = 0;
    dp::ref<dpdrm::Rights> rights = item->getRights();

    // TODO. Rollback this change when getValidLicenses() works again.
    //dp::list<dpdrm::License> licenses = rights->getValidLicenses();
    dp::list<dpdrm::License> licenses = rights->getLicenses();
    for (size_t i = 0; i < licenses.length(); ++i)
    {
        dp::list<dpdrm::Permission> permissions = licenses[i]->getPermissions("display");
        for (size_t m = 0; m < permissions.length(); ++m)
        {
            expire_time = permissions[m]->getExpiration();
        }
    }
    return expire_time;
}

void AdobeDRMHandlerPrivate::init()
{
    net_provider_.reset(new CurlNetProvider);
    dpnet::NetProvider::setProvider(net_provider_.get());
    dp::timerRegisterMasterTimer((dptimer::Timer*)master_timer_.data());

    if (!cms_.isOpen())
    {
        vbf::openDatabase(QString(), cms_);
    }
}

QString AdobeDRMHandlerPrivate::getFulfillmentItemPath(const int index,
                                                       dpdrm::DRMProcessor * drm_processor,
                                                       dpdev::Device * device)
{
    QString path;
    if (index < 0 || index >= static_cast<int>(drm_processor->getFulfillmentItems().length()))
    {
        return path;
    }

    // get the url of download source
    dp::ref<dpdrm::FulfillmentItem> item = drm_processor->getFulfillmentItems()[index];
    dpio::Partition * partition = device->getPartition(0);

    void * url = partition->getOptionalInterface("URL");
    if (url != 0)
    {
        path = QString::fromUtf8((char*)url);
    }

    QUrl doc_url(path);
    path = QString::fromLocal8Bit(QByteArray::fromPercentEncoding(doc_url.toLocalFile().toLocal8Bit()));
    return path;
}

QString AdobeDRMHandlerPrivate::saveFulfillmentItems(dpdrm::DRMProcessor * drm_processor,
                                                     dpdev::Device * device)
{
    int count = drm_processor->getFulfillmentItems().length();
    QString fulfillment_path;
    for (int i = 0; i < count; ++i)
    {
        // get the url of download source
        dp::ref<dpdrm::FulfillmentItem> item = drm_processor->getFulfillmentItems()[i];
        fulfillment_path = getFulfillmentItemPath(i, drm_processor, device);
        if (fulfillment_path.isEmpty())
        {
            continue;
        }

        FulfillmentItemInfo info;
        info.expiration = getExpirationDate(item);
        info.has_returned = false;
        info.is_returnable = drm_processor->isReturnable();

        saveFulfillmentNode(fulfillment_path,
                            drm_processor->getFulfillmentID().utf8(),
                            info);
    }
    return fulfillment_path;
}

ActivationStatus AdobeDRMHandlerPrivate::checkActivationRecords(const DRMInfo & usr_info,
                                                                dpdrm::DRMProcessor * drm_processor,
                                                                dpdev::Device * device)
{
    dp::list<dpdrm::Activation> activations = drm_processor->getActivations();
    for (size_t i = 0; i < activations.length(); ++i)
    {
        dp::ref<dpdrm::Activation> activation = activations[i];

        dp::String usr_name = activation->getUsername();
        QString usr_name_str(usr_name.utf8());
        qDebug("Input User ID:%s", qPrintable(usr_info.id()));
        qDebug("Activation User ID:%s", qPrintable(usr_name_str));
        qDebug("Act Flag:%s", qPrintable(usr_info.activation()));

        bool expired = false;
        dp::time_t expiration = activation->getExpiration() / 1000;
        if (expiration > 0)
        {
            QDateTime expire_date = QDateTime::fromTime_t(expiration);
            QDateTime current = QDateTime::currentDateTime();
            if (expire_date <= current)
            {
                qDebug("Activation has expired. Date:%s", qPrintable(expire_date.toString()));
                expired = true;
            }
        }

        if (!expired)
        {
            if (activation->hasCredentials() &&
                usr_info.id() == usr_name_str &&
                usr_info.activation() == DRM_ACTIVATION_OTA)
            {
                // the activation is only useful when OTA is executed at least once
                qDebug("Find valid activation record. Skip Activation");
                return HAS_VALID_ACTIVATION_REC;
            }
            else if (!usr_info.id().isEmpty() && usr_info.id() != usr_name_str)
            {
                qDebug("The previous activation is out-of-date. Need reactivate device");
                return ACTIVATION_OUT_OF_DATE;
            }
            else
            {
                if (usr_info.id().isEmpty())
                {
                    qDebug("Boox has been activated before, need password to sign in");

                    // save activation record
                    DRMInfo updated_info(usr_info);
                    updated_info.set_id(usr_name_str);
                    updated_info.set_password(QString());
                    updated_info.set_activation(DRM_ACTIVATION_NONE);

                    PrivateConfig conf;
                    conf.updateDRMInfo(updated_info);
                }
                else
                {
                    qDebug("Activation should be executed to fill the OTA flag");
                }
                return NO_OTA_FLAG;
            }
        }
    }
    return NO_ACTIVATION_REC;
}

bool AdobeDRMHandlerPrivate::getFulfillmentItemInfo(const QString & doc_path,
                                                    QString & fulfillment_id,
                                                    FulfillmentItemInfo & info)
{
    info.is_returnable = false;
    info.has_returned = false;
    info.expiration = 0;
    if (doc_path.isEmpty())
    {
        qWarning("Document Path of fulfillment item is empty");
        return false;
    }

    cms::ContentNode node;
    if (cms_.getContentNode(node, doc_path))
    {
        QVariantMap attributes;
        node.attributes(attributes);
        fulfillment_id = attributes[CMS_FULFILLMENT_ID].toString();
        if (attributes.contains(CMS_IS_RETURNABLE))
        {
            info.is_returnable = attributes[CMS_IS_RETURNABLE].toBool();
        }
        if (attributes.contains(CMS_HAS_RETURNED))
        {
            info.has_returned = attributes[CMS_HAS_RETURNED].toBool();
        }
        if (attributes.contains(CMS_EXPIRED_DATE))
        {
            info.expiration = attributes[CMS_EXPIRED_DATE].toULongLong();
        }

        qDebug("Loading... Fulfillment ID:%s", qPrintable(fulfillment_id));
        uint expiration_date_in_uint = info.expiration / 1000;
        if (expiration_date_in_uint != 0)
        {
            QDateTime date_time = QDateTime::fromTime_t(expiration_date_in_uint);
            qDebug("Is Returnable:%d, Has Returned:%d, Expiration:%s",
                    info.is_returnable, info.has_returned, qPrintable(date_time.toString()));
        }
        return true;
    }
    qWarning("Cannot get the content node, Path:%s", qPrintable(doc_path));
    return false;
}

bool AdobeDRMHandlerPrivate::saveFulfillmentNode(const QString & path,
                                                 const QString & id,
                                                 const FulfillmentItemInfo & info)
{
    cms::ContentNode node;
    cms_.getContentNode(node, path, true);

    QVariantMap attributes;
    attributes[CMS_FULFILLMENT_ID] = id;
    attributes[CMS_IS_RETURNABLE]  = info.is_returnable;
    attributes[CMS_HAS_RETURNED]   = info.has_returned;
    if (info.expiration != 0)
    {
        attributes[CMS_EXPIRED_DATE]   = info.expiration;
    }
    qDebug("Saving... Fulfillment ID:%s", qPrintable(id));

    QDateTime date_time = QDateTime::fromTime_t(info.expiration / 1000);
    qDebug("Is Returnable:%d, Has Returned:%d, Expiration:%s",
            info.is_returnable, info.has_returned, qPrintable(date_time.toString()));
    node.setAttributes(attributes);
    return cms_.updateContentNode(node);
}


AdobeDRMHandler::AdobeDRMHandler()
    : data_(new AdobeDRMHandlerPrivate())
    , current_download_item_(0)
    , is_busy_(false)
    , activation_succeeds_(false)
    , fulfillment_succeeds_(false)
    , loan_return_succeeds_(false)
{
    init();
}

AdobeDRMHandler::~AdobeDRMHandler()
{
}

void AdobeDRMHandler::init()
{
    data_->init();
    download_manager_.setRemovePolicy(network_service::DownloadManager::SuccessFullDownload);

    // connect the download manager
    connect(&download_manager_, SIGNAL(itemAdded(DownloadItem *)),
            this, SLOT(onDownloadItemAdded(DownloadItem *)));
}

dpdrm::DRMProcessor* AdobeDRMHandler::processor()
{
    return data_->processor();
}

bool AdobeDRMHandler::activate(const QUrl & url)
{
    //QUrl current_url(url);
    //if (!current_url.isValid())
    //{
    //    current_url = QUrl(DEFAULT_ACTIVATION_URL);
    //}

    QUrl current_url(DEFAULT_ACTIVATION_URL);
    QString host = current_url.host();
    bool succeed = false;
    if (!host.isEmpty())
    {
        DRMInfo info;
        info.set_url(host);
        {
            PrivateConfig conf;
            conf.getDRMInfo(host, info);
        }
        if (activate(info))
        {
            succeed = true;
        }
        else
        {
            succeed = false;
        }
    }

    if (!succeed)
    {
        // TODO. popup error dialog to notify user the activation fails
        emit requestDRMUserInfo(current_url.toString());
    }
    return succeed;
}

bool AdobeDRMHandler::activate(const DRMInfo & info)
{
    BusyHold busy_hold(this);
    dpdev::DeviceProvider *device_provider = dpdev::DeviceProvider::getProvider(0);
    if (device_provider == 0)
    {
        qWarning("No Device Provider Implementation");
        return false;
    }

    dpdev::Device *device = device_provider->getDevice(0);
    if( device == 0 )
    {
        qWarning("No Device Implementation");
        return false;
    }

    AdobeDRMProcessorClient drm_client(this);
    if (data_->processor() != 0)
    {
        data_->processor()->release();
    }
    data_->setProcessor(dpdrm::DRMProvider::getProvider()->createDRMProcessor(
        (dpdrm::DRMProcessorClient*)drm_client.data(), device));

    ActivationStatus status = data_->checkActivationRecords(info, data_->processor(), device);
    unsigned int workflow = 0;
    switch (status)
    {
    case HAS_VALID_ACTIVATION_REC:
        return true;
    case NO_OTA_FLAG:
        if (!isDRMInfoValid(info))
        {
            return false;
        }
        workflow = dpdrm::DW_AUTH_SIGN_IN;
        break;
    case ACTIVATION_OUT_OF_DATE:
        {
            removeOriginActivationRecord();
        }
    case NO_ACTIVATION_REC:
        if (!isDRMInfoValid(info))
        {
            return false;
        }
        workflow = dpdrm::DW_AUTH_SIGN_IN | dpdrm::DW_ACTIVATE;
        break;
    default:
        return false;
    }

    dp::String adobeid(info.id().toUtf8().constData());
    dp::String password(info.password().toUtf8().constData());
    dp::String auth_provider("AdobeID"); // TODO. support different authentication methods
    activation_succeeds_ = true;
    data_->processor()->initSignInWorkflow(workflow, auth_provider, adobeid, password);
    data_->processor()->startWorkflows(workflow);

    if (activation_succeeds_)
    {
        // save activation record
        DRMInfo updated_info(info);
        updated_info.set_activation(DRM_ACTIVATION_OTA);
        PrivateConfig conf;
        conf.updateDRMInfo(updated_info);
    }
    return activation_succeeds_;
}

void AdobeDRMHandler::onFulfillmentFailed(const QString & error)
{
    fulfillment_succeeds_ = false;
    if (error.startsWith(QString("E_AUTH_BAD_DEVICE_KEY_OR_PKCS12")) ||
        error.startsWith(QString("E_AUTH_BAD_DEVICE_KEY")))
    {
        qDebug("Bad device key, clear all DRM info");
        PrivateConfig conf;
        conf.clearDRMInfo();
    }
}

void AdobeDRMHandler::onActivationFailed(const QString & error)
{
    activation_succeeds_ = false;
    qDebug("Activation fail, clear all DRM info");
    PrivateConfig conf;
    conf.clearDRMInfo();
}


void AdobeDRMHandler::onLoanReturnFailed(const QString & error)
{
    loan_return_succeeds_ = false;
}

bool AdobeDRMHandler::fulfill(const QString & acsm, QString & fulfillment_path)
{
    BusyHold busy_hold(this);
    fulfillment_path.clear();
    if (acsm.isEmpty())
    {
        return false;
    }

    dpdev::DeviceProvider *device_provider = dpdev::DeviceProvider::getProvider(0);
    if (device_provider == 0)
    {
        qWarning("No Device Provider Implementation");
        return false;
    }

    dpdev::Device *device = device_provider->getDevice(0);
    if( device == 0 )
    {
        qWarning("No Device Implementation");
        return false;
    }

    AdobeDRMProcessorClient drm_client(this);
    if (data_->processor() != 0)
    {
        data_->processor()->release();
    }
    data_->setProcessor(dpdrm::DRMProvider::getProvider()->createDRMProcessor(
        (dpdrm::DRMProcessorClient*)drm_client.data(), device));
    if( data_->processor()->getActivations().length() == 0 )
    {
        qWarning("Warning : Device is not activated");
        return false;
    }

    QByteArray acsm_data;
    QFile file(acsm);
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        acsm_data = file.readAll();
        acsm_data.append('\0');
    }

    if (acsm_data.isEmpty())
    {
        qWarning("Empty ACSM file");
        return false;
    }

    fulfillment_succeeds_ = true;
    dp::String content_data(acsm_data.constData());

    data_->processor()->initWorkflows(dpdrm::DW_FULFILL | dpdrm::DW_DOWNLOAD | dpdrm::DW_NOTIFY, content_data);
    data_->processor()->startWorkflows(dpdrm::DW_FULFILL | dpdrm::DW_DOWNLOAD | dpdrm::DW_NOTIFY);

    if (fulfillment_succeeds_)
    {
        size_t count = data_->processor()->getFulfillmentItems().length();
        qDebug("Number of items fulfilled: %d\n", count);
        if (count > 0)
        {
            fulfillment_path = data_->saveFulfillmentItems(data_->processor(), device);
        }
        return true;
    }
    return false;
}

bool AdobeDRMHandler::loanReturn(const QString & loan_doc_path)
{
    BusyHold busy_hold(this);
    if (loan_doc_path.isEmpty())
    {
        return false;
    }

    FulfillmentItemInfo info;
    QString fulfillment_id;
    if (!data_->getFulfillmentItemInfo(loan_doc_path, fulfillment_id, info) ||
        !info.is_returnable || info.has_returned)
    {
        return false;
    }

    dpdev::DeviceProvider *device_provider = dpdev::DeviceProvider::getProvider(0);
    if (device_provider == 0)
    {
        qWarning("No Device Provider Implementation");
        return false;
    }

    dpdev::Device *device = device_provider->getDevice(0);
    if( device == 0 )
    {
        qWarning("No Device Implementation");
        return false;
    }

    AdobeDRMProcessorClient drm_client(this);
    if (data_->processor() != 0)
    {
        data_->processor()->release();
    }
    data_->setProcessor(dpdrm::DRMProvider::getProvider()->createDRMProcessor(
        (dpdrm::DRMProcessorClient*)drm_client.data(), device));
    if( data_->processor()->getActivations().length() == 0 )
    {
        qWarning("Device is not activated");
        return false;
    }

    loan_return_succeeds_ = true;
    dp::String loan_id(fulfillment_id.toUtf8().constData());
    data_->processor()->initLoanReturnWorkflow(loan_id);
    data_->processor()->startWorkflows(dpdrm::DW_LOAN_RETURN | dpdrm::DW_NOTIFY);
    if (loan_return_succeeds_)
    {
        // save the node
        info.has_returned = true;
        return data_->saveFulfillmentNode(loan_doc_path, fulfillment_id, info);
    }
    return false;
}

bool AdobeDRMHandler::runPassHash(const QString & url, const QString & id, const QString & password)
{
    BusyHold busy_hold(this);
    if (url.isEmpty() || id.isEmpty() || password.isEmpty())
    {
        return false;
    }

    dpdev::DeviceProvider *device_provider = dpdev::DeviceProvider::getProvider(0);
    if (device_provider == 0)
    {
        qWarning("No Device Provider Implementation");
        return false;
    }

    dpdev::Device *device = device_provider->getDevice(0);
    if( device == 0 )
    {
        qWarning("No Device Implementation");
        return false;
    }

    AdobeDRMProcessorClient drm_client(this);
    if (data_->processor() != 0)
    {
        data_->processor()->release();
    }
    data_->setProcessor(dpdrm::DRMProvider::getProvider()->createDRMProcessor(
        (dpdrm::DRMProcessorClient*)drm_client.data(), device));

    dp::Data pass_hash;
    if( !id.isEmpty() )
    {
        dp::String hash_user(id.toUtf8().constData());
        dp::String hash_password(password.toUtf8().constData());
        pass_hash = data_->processor()->calculatePasshash( hash_user, hash_password );
        dp::String pass_hash_str = dp::String::base64Encode( pass_hash );
        qDebug( "Passhash: %s\n", pass_hash_str.utf8() );
    }

    dp::String operator_url(url.toUtf8().constData());
    int code = data_->processor()->addPasshash( operator_url, pass_hash );
    switch( code )
    {
    case dpdrm::PH_ERROR :
        qDebug( "PH_ERROR\n" );
        return false;
    case dpdrm::PH_NO_CHANGE :
        qDebug( "PH_NO_CHANGE\n" );
        break;
    case dpdrm::PH_ACCEPTED :
        qDebug( "PH_ACCEPTED\n" );
        break;
    }
    return true;
}

bool AdobeDRMHandler::loanTokenTransfer(const QString & transfer_loan_token)
{
    BusyHold busy_hold(this);
    if (transfer_loan_token.isEmpty())
    {
        return false;
    }

    // TODO. Implement Me
    return true;
}

QString AdobeDRMHandler::getFingerprint()
{
    return device_info_.fingerprint();
}

void AdobeDRMHandler::onDownloadItemAdded(DownloadItem *item)
{
    current_download_item_ = item;
    connect(current_download_item_, SIGNAL(loadFinished()), this, SLOT(onACSMLoadFinished()));
    connect(current_download_item_, SIGNAL(loadError()), this, SLOT(onACSMLoadError()));
}

bool AdobeDRMHandler::downloadACSM(const QUrl & url)
{
    is_busy_ = true;
    download_manager_.download(url);
    return true;
}

void AdobeDRMHandler::onACSMLoadError()
{
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    disconnect(item, SIGNAL(loadFinished()), this, SLOT(onACSMLoadFinished()));
    disconnect(item, SIGNAL(loadError()), this, SLOT(onACSMLoadError()));
    is_busy_ = false;
    emit reportACSMDownloadError();
}

void AdobeDRMHandler::onACSMLoadFinished()
{
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    disconnect(item, SIGNAL(loadFinished()), this, SLOT(onACSMLoadFinished()));
    disconnect(item, SIGNAL(loadError()), this, SLOT(onACSMLoadError()));
    is_busy_ = false;
    emit reportACSMDownloadFinished(item->fileName(), item->url());
}

void AdobeDRMHandler::notifyWorkflowProgress(unsigned int workflow, const QString & title, double progress)
{
    emit reportWorkflowProgress(workflow, title, progress);
}

void AdobeDRMHandler::notifyWorkflowDone(unsigned int workflows, const QByteArray & follow_up)
{
    emit reportWorkflowDone(workflows, follow_up);
}

void AdobeDRMHandler::notifyWorkflowError(unsigned int workflow, const QString & error_code)
{
    emit reportWorkflowError(workflow, error_code);
}

void AdobeDRMHandler::reportFulfillmentFinished(const QString & fulfillment_path)
{
    emit fulfillmentFinished(fulfillment_path);
}

void AdobeDRMHandler::reportLoanReturnFinished(const QString & loan_doc_path)
{
    emit loanReturnFinished(loan_doc_path);
}

void AdobeDRMHandler::reportLoanTokenTransferFinished(const QString & transfer_loan_token)
{
    emit loanTokenTransferFinished(transfer_loan_token);
}

void AdobeDRMHandler::reportActivationFailed()
{
    emit activationFailed();
}

}
