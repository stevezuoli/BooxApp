#include "dp_all.h"
#include "adobe_drm_processor.h"
#include "adobe_drm_handler.h"

namespace adobe_drm
{

class PasshashInputData : public dp::RefCounted
{
public: 
    PasshashInputData(const dp::String& user, const dp::String& password, int timeout) :
        ref_count_(0),
        user_(user),
        password_(password),
        timeout_(timeout),
        next_(dp::ref<PasshashInputData>())
    {
    }

    void addRef()
    {
        ref_count_++;
    }

    void release()
    {
        if ( --ref_count_ == 0 ) 
            delete this;
    }

    int ref_count_;
    dp::String user_;
    dp::String password_;
    int timeout_;
    dp::ref<PasshashInputData> next_;
};

class AdobeDRMProcessorClientPrivate : public dpdrm::DRMProcessorClient, public dptimer::TimerClient
{
public:
    AdobeDRMProcessorClientPrivate(AdobeDRMHandler *host);
    virtual ~AdobeDRMProcessorClientPrivate();

    virtual void workflowsDone( unsigned int workflows, const dp::Data& follow_up );
    virtual void requestInput( const dp::Data& input_XHTML );
    virtual void requestConfirmation( const dp::String& code );
    virtual void reportWorkflowProgress( unsigned int workflow, const dp::String& title, double progress );
    virtual void reportWorkflowError( unsigned int workflow, const dp::String& error_code );
    virtual void reportFollowUpURL( unsigned int workflow, const dp::String& url );
    virtual void requestPasshash( const dp::ref<dpdrm::FulfillmentItem>& fulfillmentItem );
    virtual void timerFired( dptimer::Timer * timer );

private:
    void deliverPasshash();
    dp::String workflowToString(int workflow);

private:
    AdobeDRMHandler *host_;
	dp::ref<PasshashInputData> passhash_head;
	dp::ref<PasshashInputData> passhash_tail;
};

AdobeDRMProcessorClientPrivate::AdobeDRMProcessorClientPrivate(AdobeDRMHandler *host)
    : host_(host)
    , passhash_head(dp::ref<PasshashInputData>())
    , passhash_tail(dp::ref<PasshashInputData>())
{
}

AdobeDRMProcessorClientPrivate::~AdobeDRMProcessorClientPrivate()
{
}

/**
 *  Indicates that the workflows have finished. If followUp parameter is
 *  not null, it indicates a follow-up workflows that must be performed
 *  next (server notification).
 */
void AdobeDRMProcessorClientPrivate::workflowsDone( unsigned int workflows, const dp::Data& follow_up )
{
    std::cout << "Workflow " << workflowToString(workflows).utf8() << " is done." << std::endl;
    if (follow_up.length() > 0)
    {
        dp::String str(reinterpret_cast<const char *>(follow_up.data()));
        std::cout << "Data:" << std::endl << str.utf8() << std::endl;
    }
    host_->notifyWorkflowDone(workflows, QByteArray(reinterpret_cast<const char *>(follow_up.data())));
}

/**
 *  Request user input based on simple XHTML UI. Simple XHTML
 *  form syntax is supported. Client is only required to support
 *  plain text and input elements of types "text", "password" and "hidden".
 *  UI based on the given XHTML should be presented to the user.
 *  Once user enters values, input should be given to
 *  dpdrm::DRMProcessor::provideInput method.
 *
 *  Currently unused (for future TransparentID support).
 */
void AdobeDRMProcessorClientPrivate::requestInput( const dp::Data& input_XHTML )
{
}

/**
 *  Request user's confirmation
 *  "Q_ADEPT_NEW_ACCOUNT accountid accountName" - new DRM (Eden2) account is about to be created for accountName
 *  "Q_ADEPT_ADD_ACCOUNT accountid accountName Eden2GUID" - accountName is about to be mapped to the given Eden2 GUID
 * 
 *  Currently unused (for future TransparentID support).
 */
void AdobeDRMProcessorClientPrivate::requestConfirmation( const dp::String& code )
{
}

/**
 *  Reports which DRM workflow is currently active and its progress.
 */
void AdobeDRMProcessorClientPrivate::reportWorkflowProgress( unsigned int workflow, const dp::String& title, double progress )
{
    std::cout << "Workflow " << workflow << ": " << title.utf8() << progress << std::endl;
    host_->notifyWorkflowProgress(workflow, QString(title.utf8()), progress);
}

void AdobeDRMProcessorClientPrivate::reportFollowUpURL( unsigned int workflow, const dp::String& url )
{
    std::cerr << "Workflow " << workflowToString(workflow).utf8() << " BROWSE: " << url.utf8() << std::endl;
}

void AdobeDRMProcessorClientPrivate::deliverPasshash()
{
    dp::Data passhash = dp::Data();
    if (passhash_head != NULL)
    {
        dp::ref<PasshashInputData> temp = passhash_head;
        passhash_head = temp->next_;
        if (passhash_head == NULL)
        {
            passhash_tail = passhash_head;
        }

        if (temp->user_.length() > 0)
        {
            passhash = host_->processor()->calculatePasshash(temp->user_, temp->password_);
        }
    }
    host_->processor()->providePasshash(passhash);
}

void AdobeDRMProcessorClientPrivate::requestPasshash( const dp::ref<dpdrm::FulfillmentItem>& fulfillmentItem )
{
    if (!passhash_head || !passhash_head->timeout_)
    {
        deliverPasshash();
    }
    else
    {
        dptimer::TimerProvider * provider = dptimer::TimerProvider::getProvider();
        dptimer::Timer * timer = provider->createTimer(this);
        timer->setTimeout(passhash_head->timeout_);
    }
}

void AdobeDRMProcessorClientPrivate::timerFired( dptimer::Timer * timer )
{
    timer->release();
    deliverPasshash();
}

/**
 *  Report an error for a DRM workflow which is currently in progress.
 */
void AdobeDRMProcessorClientPrivate::reportWorkflowError( unsigned int workflow, const dp::String& error_code )
{
    std::cerr << "Workflow " << workflow << " ERROR: " << error_code.utf8() << std::endl;
    host_->notifyWorkflowError(workflow, QString(error_code.utf8()));
}

dp::String AdobeDRMProcessorClientPrivate::workflowToString(int workflow)
{
    switch (workflow) 
    {
    case dpdrm::DW_SIGN_IN: return "DW_SIGN_IN";
    case dpdrm::DW_AUTH_SIGN_IN: return "DW_AUTH_SIGN_IN"; 
    case dpdrm::DW_ADD_SIGN_IN: return "DW_ADD_SIGN_IN";
    case dpdrm::DW_ACTIVATE: return "DW_ACTIVATE";
    case dpdrm::DW_FULFILL: return "DW_FULFILL";
    case dpdrm::DW_ENABLE_CONTENT: return "DW_ENABLE_CONTENT";
    case dpdrm::DW_LOAN_RETURN: return "DW_LOAN_RETURN";
    case dpdrm::DW_UPDATE_LOANS: return "DW_UPDATE_LOANS";
    case dpdrm::DW_DOWNLOAD: return "DW_DOWNLOAD";
    case dpdrm::DW_NOTIFY: return "DW_NOTIFY";
    }
    return "";
}


AdobeDRMProcessorClient::AdobeDRMProcessorClient(AdobeDRMHandler *host)
    : data_(new AdobeDRMProcessorClientPrivate(host))
{
}

AdobeDRMProcessorClient::~AdobeDRMProcessorClient()
{
}

AdobeDRMProcessorClientPrivate * AdobeDRMProcessorClient::data()
{
    return data_.get();
}

}
