/*
 *  ADOBE CONFIDENTIAL
 *
 *  Copyright 2006-2009, Adobe Systems Incorporated
 *
 *  All Rights Reserved.
 *
 *  NOTICE:  All information contained herein is, and remains the property of
 *  Adobe Systems Incorporated and its suppliers, if any.  The intellectual and
 *  technical concepts contained herein are proprietary to Adobe Systems
 *  Incorporated and its suppliers and may be covered by U.S. and Foreign
 *  Patents, patents in process, and are protected by trade secret or copyright
 *  law.  Dissemination of this information or reproduction of this material is
 *  strictly forbidden unless prior written permission is obtained from Adobe
 *  Systems Incorporated.
 */
/**
 *  \file dp_drm.h
 *
 *  Reader Mobile SDK public interface -- DRM
 */
#ifndef _DP_DRM_H
#define _DP_DRM_H

#include "dp_core.h"
#include "dp_dev.h"
#include "dp_net.h"

/**
 *  \page drm Digital Rights Management
 *
 *  Here is a brief explanation of how the DRM system works on the client. 
 *
 *  Foundation fopr the DRM is provided by the dpdev::Device interface. Following description assumes familiarity
 *  with dpdev module.
 *
 *  To manage device activation and fulfill the content to the device, create an instance of dpdrm::DRMProcessor
 *  using the dpdev::Device object that you want to work with. You have to register networking provider (see
 *  dpnet::NetProvider) and provide dpdrm::DRMProcessorClient object
 *  that gets notified when DRM operations complete or produce some results.
 *
 *  dpdrm::DRMProcessor can only handle a single DRM workflow at a time. It is the caller's responsibility
 *  to never start multiple workflows simultaneously. Also, never create multiple dpdrm::DRMProcessor objects
 *  using the same device.
 *
 *  \section activation Activation
 *
 *  To check if the device is activated, call dpdrm::DRMProcessor::getActivations. If the list
 *  that it returns has at least 1 item, the device is activated. If the device is not activated,
 *  it needs to be activated to be able to open DRM-protected documents.
 *
 *  Device activation is typically a one-time operation. For network-capable devices it can be
 *  performed by supplying appropriate workflow codes (DW_AUTH_SIGN_IN|DW_ACTIVATE), user's
 *  AdobeID, and user's password to the dpdrm::DRMProcessor::initAdobeIDSignIn method and then 
 *  dpdrm::DRMProcessor::startWorkflows. dpdrm::DRMProcessor will communicate to Adobe activation
 *  server and in the end dpdrm::DRMProcessorClient::workflowsDone will be called. If no errors
 *  were reported with calls to dpdrm::DRMProcessorClient::reportWorkflowError, activation is successfull.
 *
 *  Once the device is activated, the content that was licensed to that user (e.g. content transferred from
 *  some other system which user already has) should work as long as this device is trusted and present in the
 *  environment.
 *
 *  \section fulfillment Fulfillment
 *
 *  Provisioning new content to the user and the device is called fulfillment. Fulfillment starts when a
 *  distributor provides the client with a little XML message called ".acsm file". It can be detected by its mime
 *  type "application/vnd.adobe.adept+xml". The content of this message should be passed to
 *  dpdrm::DRMProcessor::initWorkflows call. If ACSM file indeed contains fulfillment token (which will not
 *  always be true), DW_FULFILL and DW_DOWNLOAD bits will be set in the result. Start fulfillment workflow
 *  by calling dpdrm::DRMProcessor::startWorkflows(DW_FULFILL|DW_DOWNLOAD). When the license is acquired
 *  and the content is downloaded, dpdrm::DRMProcessorClient::workflowsDone will be called. If no errors were
 *  reported during fulfillment, it was successful. Newly-fulfilled documents will be added to the library
 *  (see dplib::Library).
 *
 *  The final and very important step of the fulfillment process is notifying the server that fulfillment is
 *  complete. dpdrm::DRMProcessorClient::workflowsDone call will provide a "follow-up" data which should be
 *  used to initialize notification workflow using dpdrm::DRMProcessor::initWorkflows. Start it using usual
 *  dpdrm::DRMProcessor::startWorkflows call. Notification should be retried until successful, so the
 *  "follow-up" data should be stored in device non-volatile storage, so that it can be retried even after
 *  the device is powered down.
 *
 *  \section loans Returnable Loans
 *
 *  DRM client support returnable loan feature. What is commonly called "loans" are actually two very different
 *  types of licenses: expiring licenses and returnable licenses. Expiring licenses simply stop working
 *  after the expiration time. Returnable licenses work until they are returned. In Content Server 4 ecosystem
 *  only returnable licenses are called loans. Note that returnable licenses do not have to expire, even though
 *  they typically are configured to expire; however, it is the ability to return them which makes them special.
 *
 *  Each device activation record
 *  holds server-signed tokens with lists of outstanding loans (called "loan tokens"). Each
 *  time a loan is taken, the loan id is added to the list and when loan is returned, loan id is removed from it.
 *  Loaned content is only readable if the corresponding loan id is on record in the main device (the first DeviceInfo
 *  object on the list).
 *
 *  When a fulfillment transaction is returnable, dplib::ContentRecord objects that it creates will be tagged with
 *  "adept:borrowed" tag. Loan is is the same as fulfillmentID (available by calling
 *  dplib::ContentRecord::getFulfillmentID).
 *
 *  To return a loan call dpdrm::DRMProcessor::initLoanReturnWorkflow followed by dpdrm::DRMProcessor::startWorkflows.
 *  One new loan token is issued and written
 *  into the device activation record, dpdrm::DRMProcessorClient::workflowsDone callback will be called. At that point all
 *  loaned content must be erased and the same notification procedure as for fulfillment started.
 *
 *  \section attached Attached Device Issues
 *
 *  Tethered devices which are not network-capable have to be activated while they are plugged into a PC. In general,
 *  they are activated to the same user account as the PC. To avoid asking the user for his AdobeID again, 
 *  tethered devices DRMProcessor should be initialized with the user credentials from the PC dpdev::Device by
 *  calling dpdrm::DRMProcessor::transferCredentialsFrom.
 *
 *  When loaned content is transferred to an attached device, corresponding loan token needs to be transferred with it.
 *  This can be done using embed::DRMProcessor::transferLoanTokensFrom. It transfers most recent loan tokens from the 
 *  source device to the target device (device managed by dpdrm::DRMProcessor object).
 *
 */

namespace dplib
{
class Library;
}

/**
 *  Reader Mobile SDK DRM interfaces. Provides compatibility with Adobe Content Server 4 (activation,
 *  fulfillment, reading authorized content, permissions).
 *
 *  See:
 *   - \ref drm
 */
namespace dpdrm
{

class DRMProvider;
class DRMProcessor;
class DRMProcessorClient;
class Activation;
class Rights;
class License;
class Permission;
class FulfillmentItem;

/**
 *  DRM provider interface. Serves as a factory for DRMProcessor and provides utility method to parse
 *  the license.
 */
class DRMProvider : public dp::Unknown
{
public:

	virtual int getInterfaceID() { return IID_DRMProvider; }
	/**
	 *  Create a new DRMProcessor based on a specific dpdev::Device.
     *  \commands
     *  \commandrefbr{createDRMProcessor}
	 */
	virtual DRMProcessor * createDRMProcessor( DRMProcessorClient * client, dpdev::Device * device ) = 0;

	/**
	 *  Parses license XML and returns a set of rights which that license provides.
     *  \commands
     *  \commandrefbr{parseLicense}
	 */
	virtual dp::ref<Rights> parseLicense( const dp::Data& rights ) = 0;
	
	/**
	 *  Returns SDK-supplied DRM provider.
	 */
	static DRMProvider * getProvider();
};

/**
 *  Flags to indicate various workflows that dpdrm::DRMProcessor can perform. Note that each
 *  workflow is only a small unit of what would typically be called a workflow in the real world.
 */
enum DRMWorkflow
{
	/**
	 *  Obtain sign-in token using vendor's account credentials (such as username and password). 
	 *  This workflow does not require DRMProcessor to have user's credentials.
	 *
	 *  This workflow is initiated by passing a special ACSM token to DRMProcessor::initWorkflows method.
	 *
	 *  Currently unused (for future TransparentID support).
	 */
	DW_SIGN_IN        = 0x0001,
	/**
	 *  Obtain user's credentials using either sign-in token or AdobeID/password.
	 *  This workflow does not require DRMProcessor to have user's credentials.
	 *
	 *  This workflow is initiated either by passing sign-in token to DRMProcessor::initWorkflows method
	 *  (not used until TransparentID is implemented) or by passing AdobeID and password
	 *  DRMProcessor::initAdobeIDSignIn method.
	 *
	 *  Upon successfull completion, user credentials are stored in DRMProcessor and written in the
	 *  activation record if dpdev::Device::getDeviceKey is not null.
	 */
	DW_AUTH_SIGN_IN   = 0x0002,
	/**
	 *  Add the vendor account from the sign-in token or AdobeID/password to the user's existing DRM account.
	 *  This workflow requires DRMProcessor to have user's credentials.
	 *
	 *  This workflow is initiated either by passing sign-in token to DRMProcessor::initWorkflows method
	 *  (not used until TransparentID is implemented) or by passing AdobeID and password
	 *  DRMProcessor::initAdobeIDSignIn method.
	 *
	 *  Upon successfull completion, vendor's account or AdobeID will be bound to the user's DRM account,
	 *  and therefore given sign-in can be used in future to get user's credentials and activate devices to
	 *  the access user's content.
	 */
	DW_ADD_SIGN_IN    = 0x0004,
	/**
	 *  Activate the device.
	 *  This workflow requires DRMProcessor to have user's credentials.
	 *
	 *  This workflow does not require any parameters, so it can simply be started by startWorkflows.
	 *
	 *  Upon successfull completion, activation token is written to the device activation record.
	 */
	DW_ACTIVATE       = 0x0010,
	/**
	 *  Fulfill the content.
	 *  This workflow requires DRMProcessor to have user's credentials and device to be activated to
	 *  that user.
	 *
	 *  This workflow is initiated by passing a special ACSM token (fulfillment token) to
	 *  DRMProcessor::initWorkflows method.
	 *
	 *  Upon successfull completion, licensing information is stored in DRMProcessor (see 
	 *  DRMProcessor::getFulfillmentItem) and updated loan token is written to the device
	 *  activation record.
	 */
	DW_FULFILL        = 0x0020,
	/**
	 *  Obtain the license for the content which does not have a valid license.
	 *  This workflow requires DRMProcessor to have user's credentials and device to be activated to
	 *  that user.
	 *
	 *  Currently unused (for future pass-along support).
	 */
	DW_ENABLE_CONTENT = 0x0040,
	/**
	 *  Return a loaned resource.
	 *  This workflow requires DRMProcessor to have user's credentials.
	 *
	 *  This workflow is initiated by calling DRMProcessor::initLoanReturnWorkflow with
	 *  the fulfillmentID (a.k.a. loanID) of the loan transaction to return.
	 *
	 *  Upon successfull completion, updated loan token is written to the device activation record.
	 */
	DW_LOAN_RETURN    = 0x0080,
	/**
	 *  Update loan token.
	 *  This workflow requires DRMProcessor to have user's credentials.
	 *
	 *  This workflow is initiated by calling DRMProcessor::initUpdateLoansWorkflow with
	 *  the URL of operator, which issued the loan, and id of the loaned resource.
	 *
	 *  Upon successfull completion, updated loan tokens are written to the device activation record.
	 */
	DW_UPDATE_LOANS   = 0x0100,
	/**
	 *  Download the content.
	 *  This workflow requires DRMProcessor to have user's credentials, device to be activated to
	 *  that user and completed fulfillment workflow performed by the DRMProcessor which is used
	 *  for downloading.
	 *
	 *  This workflow does not require any parameters, so it can simply be started by startWorkflows.
	 *
	 *  Upon successfull completion, new content records are created in the dplib::Library that corresponds
	 *  to the selected partition of the device.
	 */
	DW_DOWNLOAD       = 0x0200,
	/**
	 *  Join two DRM accounts.
	 *
	 *  This workflow is initiated by by calling DRMProcessor::initJoinAccountsWorkflow method. 
	 */
	DW_JOIN_ACCOUNTS  = 0x400,
	/**
	 *  Join two DRM accounts.
	 *
	 *  This workflow is initiated by by calling DRMProcessor::initWorkflow method. It may be combined
	 *  with DW_ACTIVATE flag.
	 */
	DW_GET_CREDENTIAL_LIST  = 0x800,
	/**
	 *  Notify the server.
	 *  This workflow requires DRMProcessor to have user's credentials.
	 * 
	 *  This workflow is initiated by by passing a special ACSM token (notification data) to
	 *  DRMProcessor::initWorkflows method. Notification data is provided by 
	 *  DRMProcessorClient::workflowsDone for some workflows (DW_FULFILL and DW_LOAN_RETURN).
	 */
	DW_NOTIFY         = 0x1000,
	/**
	 *  Convenience value - a bitmask to indicate all possible workflows.
	 */
	DW_ALL_POSSIBLE   = 0xFFFFFFFF
};

/**
 *  A code indicating user confirming or rejecting an action.
 *
 *  Currently unused (for future TransparentID support).
 */
enum ConfirmationCode
{
	CC_REJECT = 0, /**< user rejected an action */
	CC_CONFIRM = 1 /**< user confirmed an action */
};

/**
 *  Host interface to support DRM. This is an interface that
 *  the host side must implement. It contains high-level DRM notification routines.
 *
 *  See:
 *   - \ref drm
 */
class DRMProcessorClient : public dp::Unknown
{
protected:

	virtual ~DRMProcessorClient() {}

public:

	virtual int getInterfaceID() { return IID_DRMProcessorClient; }
	/**
	 *  Indicates that the workflows have finished. If followUp parameter is
	 *  not null, it indicates a follow-up workflows that must be performed
	 *  next (server notification).
	 */
	virtual void workflowsDone( unsigned int workflows, const dp::Data& followUp ) = 0;
	/**
	 *  Request a passhash for given fulfillemt item. Client is required 
	 *  to request user input and calculate the passhash. Passhash should 
	 *  be given to dpdrm::DRMProcessor::providePasshash method.
	 */
	virtual void requestPasshash( const dp::ref<dpdrm::FulfillmentItem>& fulfillmentItem ) = 0;
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
	virtual void requestInput( const dp::Data& inputXHTML ) = 0;
	/**
	 *  Request user's confirmation
	 *  "Q_ADEPT_NEW_ACCOUNT accountid accountName" - new DRM (Eden2) account is about to be created for accountName
	 *  "Q_ADEPT_ADD_ACCOUNT accountid accountName Eden2GUID" - accountName is about to be mapped to the given Eden2 GUID
	 * 
	 *  Currently unused (for future TransparentID support).
	 */
	virtual void requestConfirmation( const dp::String& code ) = 0;
	/**
	 *  Reports which DRM workflow is currently active and its progress.
	 */
	virtual void reportWorkflowProgress( unsigned int workflow, const dp::String& title, double progress ) = 0;
	/**
	 *  Report an error for a DRM workflow which is currently in progress.
	 */
	virtual void reportWorkflowError( unsigned int workflow, const dp::String& errorCode ) = 0;
	/**
	 *  Report a URL that should be opened to follow up the workflow (used for DW_JOIN_ACCOUNTS)
	 */
	virtual void reportFollowUpURL( unsigned int workflow, const dp::String& url ) = 0;
};

/**
 *  Status code for dpdrm::DRMProcessor::addPassHash and dpdrm::DRMProcessor::removePassHash methods.
 */
enum PassHashStatus
{
	PH_ERROR = 0,		/**< error occurred */
	PH_NO_CHANGE = 1,	/**< no change was needed */
	PH_ACCEPTED = 2		/**< change accepted */
};

/**
 *  Interface to support DRM-related client-server interactions. Some interactions make sense by themselves,
 *  and some must come as a part of a sequence. A minimal set of such interactions is called a \e workflow.
 *  All supported workflows are enumerated by DRMWorkflow enum.
 *
 *  Here are several natural sequences of workflows for explanatory purposes:
 *
 *  New PC or mobile device activation:
 *  - initAdobeIDSignInWorkflow call initiating DW_AUTH_SIGN_IN
 *  - startWorkflow( DW_AUTH_SIGN_IN | DW_ACTIVATE )
 *  .
 *
 *  New tethered device activation:
 *  - transferCredentialsFrom call
 *  - startWorkflow( DW_ACTIVATE )
 *  .
 *
 *  Fulfillment to PC:
 *  - initWorkflows( acsm ) - expect to return DW_FULFILL
 *  - startWorkflow( DW_FULFILL | DW_DOWNLOAD )
 *
 *  See:
 *   - \ref drm
 *  .
 */
class DRMProcessor : public dp::Releasable
{
protected:

	virtual ~DRMProcessor() {}

public:

	virtual int getInterfaceID() { return IID_DRMProcessor; }
	/**
	 *  Free this object
	 */
	virtual void release() = 0;
	/**
	 *  Get a list of active users for this device.
	 */
	virtual dp::list<Activation> getActivations() = 0;
	/**
	 *  Set user account for DRM workflows. If the user is not set, the first active user account
	 *  (as returned by DRMProcessor::getActivations()) is used.
	 */
	virtual void setUser( const dp::String& user ) = 0;
	/**
	 *  Sets the desired partition for fulfillment downloads. If this is not set, device first partition
	 *  is used.
	 */
	virtual void setPartition( dpio::Partition * partition ) = 0;
	/**
	 *  Reset the state, erasing all workflow initializations.
	 */
	virtual void reset() = 0;
	/**
	 *  Initiates DRM workflow based on ACSM file from the server (mime type "application/vnd.adobe.adept+xml").
	 *  Parameter workflows controls which workflows shoould be initialized if ACSM file contains multiple
	 *  items. Pass DW_ALL_POSSIBLE as workflows parameter to initialize all possible workflows.
	 * 
	 *  Returns ORed flags of workflows that the device state and ACSM file implies.
	 */
	virtual unsigned int initWorkflows( unsigned int workflows, const dp::Data& acsm ) = 0;
	/**
	 *  Initializes sign-in and activation using an authProvider (such as AdobeID), username and password. 
	 *  Pass either DW_AUTH_SIGN_IN or DW_ADD_SIGN_IN to initialize desired workflows. You can OR it with
	 *  DW_ACTIVATE to also add activation workflow.
	 */
	virtual unsigned int initSignInWorkflow( unsigned int workflows, const dp::String& authProvider, const dp::String& username, const dp::String& password ) = 0;
	/**
	 *  Initializes sign-in and activation using authProvider and authToken. This call will only work for
	 *  authProviders that support taking authTokens, and does not work for AdobeID.
	 *  The username is used strictly to annotate the resulting activation record, and is not passed to the activation server
	 *  Pass either DW_AUTH_SIGN_IN or DW_ADD_SIGN_IN to initialize desired workflows. You can OR it with
	 *  DW_ACTIVATE to also add activation workflow.
	 */
	virtual unsigned int initSignInWorkflow( unsigned int workflows, const dp::String& authProvider, const dp::String& username, const dp::Data& authToken) = 0;
	/**
	 *  Initializes loan return workflow
	 */
	virtual unsigned int initLoanReturnWorkflow( const dp::String& loanid ) = 0;
	/**
	 *  Initializes loan token update workflow
	 */
	virtual unsigned int initUpdateLoansWorkflow( const dp::String& operatorURL, const dp::String& resourceID ) = 0;
	/**
	 *  Initializes account join workflow. This workflow is typically initiated when an imported ("side-loaded")
	 *  resource failed to open because it is licensed to a different user. Error message (E_ADEPT_CORE_USER_NOT_ACTIVATED)
	 *  will contain user id to which the resource belongs and operatorURL of the operator who issued the license.
	 *  Account join workflow purpose is to initiate contact to the authentication provider of that new userid
	 *  and establish a link between the current DRM user id and this new user id ("join accounts").
	 *
	 *  Parameter \a user defines DRM account which should be joined
	 *  to the current active account. Parameter \a operatorURL, if non-null value is passed, is taken
	 *  from the license of the resource and \a title, if not null, is the title of the resource.
	 */
	virtual unsigned int initJoinAccountsWorkflow( const dp::String& user, const dp::String& operatorURL,
		const dp::String& title ) = 0;
	/**
	 *  Start initialized workflow(s). Typically result of one of the init***Workflow methods is passed
	 *  as workflows parameter.
	 */
	virtual void startWorkflows( unsigned int workflows ) = 0;
	/**
	 *  Provide passhash requested by dpdrm::DRMProcessorClient::requestPasshash method.
	 *
	 *  Null data indicates that user cancelled the input.
	 */
	virtual void providePasshash( const dp::Data& passhash ) = 0;
	/**
	 *  Provide user input requested by dpdrm::DRMProcessorClient::requestInput method.
	 *
	 *  Data should be encoded according to "application/x-www-form-urlencoded" 
	 *  content type
	 *
	 *  Currently unused (for future TransparentID support).
	 */
	virtual void provideInput( const dp::Data& signInInput ) = 0;
	/**
	 *  Provide user input to the confirmation requested by dpdrm::DRMProcessorClient::requestConfirmation
	 *  Value of confirmationCode parameter should be one of the values from dpdrm::ConfirmationCode enum.
	 *
	 *  Currently unused (for future TransparentID support).
	 */
	virtual void provideConfirmation( const dp::String& code, int confirmationCode ) = 0;
	/**
	 *  Transfers loan info from a source device (typically PC) to a device that this
	 *  DRMProcessor manages (typically handheld).
	 *
	 *  If multiple user credentials are found, loan tokens are transferred for all of the
	 *  active users on the target device. If user is explicitly specified by
	 *  DRMProcessor::setUser, than only loan tokens for that user are transferred.
	 *
     *  See:
     *   - \ref loans
	 */
	virtual void transferLoanTokensFrom( dpdev::Device * source ) = 0;
	/**
	 *  Transfers user's credentials from a source device (typically PC) to a DRMProcessor
	 *  that services tethered device. This allows network-based workflows to be performed with
	 *  tethered devices.
	 *
	 *  If \a all parameter is true, all user credentials will be be saved in the device
	 *  activation record (dpdev::Device::getDeviceKey must return non-null key for both devices
	 *  for that to work).
	 *
	 *  If \a all parameter is false, only a minimal subset of credentials (to open protected content)
	 *  will be saved on the device.
	 */
	virtual void transferCredentialsFrom( dpdev::Device * source, const dp::String& user, bool all ) = 0;
	/**
	 *  After a successful fulfillment, returns information about acquired resource licenses.
	 */
	virtual dp::list<FulfillmentItem> getFulfillmentItems() = 0;
	/**
	 *  After a successful fulfillment, return fulfillment id 
	 */
	virtual dp::String getFulfillmentID() = 0;
	/**
	 *  After a successful fulfillment, return true if this was returnable transaction
	 */
	virtual bool isReturnable() = 0;
	/**
	 *  Add password hash to the device activation record for the given \p operatorURL. Return values:
	 *  - dpdrm::PH_ERROR - could not add password hash; perhaps activation record is corrupted or unreadable
	 *  - dpdrm::PH_NO_CHANGE - password hash is already on record, no changes were necessary
	 *  - dpdrm::PH_ACCEPTED - password hash added successfully
	 *  .
	 */
	virtual int addPasshash( const dp::String& operatorURL, const dp::Data& passHash ) = 0;
	/**
	 *  Remove password hash from the device activation record for the given \p operatorURL. Return values:
	 *  - dpdrm::PH_ERROR - could not remove password hash; perhaps activation record is corrupted or unreadable
	 *  - dpdrm::PH_NO_CHANGE - password hash not found, no changes were necessary
	 *  - dpdrm::PH_ACCEPTED - password hash removed successfully
	 *  .
	 */
	virtual int removePasshash( const dp::String& operatorURL, const dp::Data& passHash ) = 0;
	/**
	 *  Calculate password hash based on the username and password provided. 
	 *  Passhash is always 20 bytes long.
	 */
	virtual dp::Data calculatePasshash( const dp::String& username, const dp::String& password ) = 0;
	/**
	 *  Create a new dpdrm::DRMProcessor, use dpdrm::DRMProcessor::release to free it.
	 */
	static DRMProcessor * createDRMProcessor( DRMProcessorClient * client, dpdev::Device * device, dplib::Library * library );

};

/**
 *  An object that represents a user with a valid activation record. A "valid activation record" implies:
 *  - there is an activation token for that user in the device activation record
 *  - activation token deviceType and fingerprint match those of the device
 *  - activation token signature is valid
 *  - activation token is not expired
 *  - there is private license key and a valid license certificate for that user
 *  .
 */
class Activation : public dp::RefCounted
{
public:
	virtual int getInterfaceID() { return IID_Activation; }
	/**
	 *  Get user id (normally in the form of "urn:uuid:XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX").
	 *  User id identifies user's Adobe DRM account (a.k.a. "Eden2 account").
	 */
	virtual dp::String getUserID() = 0;
	/**
	 *  Get device id (normally in the form of "urn:uuid:XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX").
	 *  Device id identifies a particular activation of that device by a particular user. Different
	 *  users will have different device ids for the same device object.
	 */
	virtual dp::String getDeviceID() = 0;
	/**
	 *  Get activation expiration or zero if this activation does not expire.
	 *  Activation expiration is a rare case, mostly useful for kiosk type devices (when device
	 *  type is set to "public").
	 * 
	 *  See also:
	 *  - dp::String::stringToTime
	 *  - dp::String::timeToString
	 *  .
	 */
	virtual dp::time_t getExpiration() = 0;
	/**
	 *  Get the Authority which provided authentication (ie AdobeID, or others)
	 */
	virtual dp::String getAuthority( ) = 0;
	/**
	 *  Get the user displayable identifier provided by the Authority for this account
	 *  It may not be the actual account name, but instead a label.
	 */
	virtual dp::String getUsername( ) = 0;
	/**
	 *  Return true if the activation record contains user's credentials (pkcs12). Most DRM
	 *  operations require user's credentials, but credentials only can be stored on the devices
	 *  that provide secure device key (see dpdev::Device::getDeviceKey). If either pkcs12 entry
	 *  is missing (typical for tethered devices) or dpdev::Device::getDeviceKey returns null
	 *  DRM operations are only possible if user's credentials are transferred from another
	 *  device using DRMProcessor::transferCredentialsFrom method.
	 *
	 *  Note that missing user's credentials do not prevent DRM-protected content from working.
	 */
	virtual bool hasCredentials() = 0;
};

/**
 *  An object that represents a single fulfilled item which includes rights granted (license),
 *  metadata and resource download information.
 */
class FulfillmentItem : public dp::RefCounted
{
protected:

	~FulfillmentItem() {}

public:
	
	virtual int getInterfaceID() { return IID_FulfillmentItem; }
	/**
	 *  Each Rights object is guaranteed to contain exactly one License.
	 *  Main reason why Rights is exposed here is so that serialized form
	 *  can be obtained using Rights::serialize().
	 */
	virtual dp::ref<Rights> getRights() = 0;
	/**
	 *  Metadata query. Same names can be passed as to dpdoc::Document::getMetadata.
	 */
	virtual dp::String getMetadata( const dp::String& name ) = 0;
	/**
	 *  HTTP(S) method to download resource ("GET" or "POST")
	 */
	virtual dp::String getDownloadMethod() = 0;
	/**
	 *  URL to download resource. Can potentially be null for secondary resources
	 *  (when a single document contains multiple DRM resources), but Content Server 4
	 *  does not support secondary resources.
	 */
	virtual dp::String getDownloadURL() = 0;
	/**
	 *  Data to post for POST download method. ACSM content type must be used
	 *  ("application/vnd.adobe.adept+xml").
	 */
	virtual dp::Data getPostData() = 0;
};

/**
 *  An object that represents a set of rights granted for a document. Rights
 *  contain a number of licenses, each license applying to a single resource
 *  (conceptually, a document can contain multiple DRM-protected resources) and
 *  a single user (conceptually, a document can contain licenses for more than one
 *  user).
 *
 *  Internally, Rights object also holds license validation information
 *  (signing server certificate), but there is no public API to obtain it.
 */
class Rights : public dp::RefCounted
{
protected:
	virtual ~Rights() {}

public:
	
	virtual int getInterfaceID() { return IID_Rights; }
	/**
	 *  Enumerates all existing licenses from the this rights object without
	 *  regard to their validity. This is needed only in very special cases,
	 *  in general, getValidLicenses should be used instead.
	 */
	virtual dp::list<License> getLicenses() = 0;
	/**
	 *  Merges all the licenses that are valid and returns the result.
	 *  License is valid if its integrity can be verified, it is not expired and the
	 *  user to whom the license is provided is activated. If the device
	 *  parameter is provided, only look for users activated on that device. If the
	 *  device parameter is not provided, uses all trusted devices.
	 */
	virtual dp::list<License> getValidLicenses( dpdev::Device * device = 0 ) = 0;
	/**
	 *  Reads the rights object as XML. This corresponds to the META-INF/rights.xml
	 *  file in EPUB (see OCF specification).
	 */
	virtual dp::Data serialize() = 0;
};

/**
 *  An object that represents DRM license for a particular resource granted to a particular user.
 *  License contains resource id, operator URL (URL of the Content Server that produced that license),
 *  license URL (identifier of the server that signed the license), user id, and permissions granted
 *  by the license.
 *
 *  The set of permissions for the document are encoded in using simple XML grammar. Here is an example:
 *  <pre>
 *  &lt;permissions xmlns="http://ns.adobe.com/adept"&gt;
 *    &lt;display/&gt;
 *    &lt;excerpt&gt;
 *      &lt;device&gt;deviceid&lt;/device&gt;
 *      &lt;until&gt;expiration date&lt;/until&gt;
 *      &lt;count initial="10" max="20" incrementInterval="time per unit in sec"/&gt;
 *    &lt;/excerpt&gt;
 *    &lt;print&gt;
 *      &lt;maxResolution&gt;150&lt;/maxResolution&gt;
 *      &lt;loan&gt;loanid&lt;/loan&gt;
 *    &lt;/print&gt;
 *    &lt;print&gt;
 *      &lt;maxResolution&gt;300&lt;/maxResolution&gt;
 *      &lt;count initial="10" max="20" incrementInterval="360000"/&gt;
 *    &lt;/print&gt;
 *  &lt;/permissions&gt;
 *  </pre>
 *
 *
 *  Display element is a permission to render the document on screen, excerpt element is a permission for copy to
 *  the clipboard, print element is a permission to render the document for printing. Each permission can have
 *  additional constraints on it. Device element restricts usage to a particular device only, until element
 *  contains permission expiration time (using using W3C date and time format: http://www.w3.org/TR/NOTE-datetime),
 *  count element restricts the usage to a certain number of times/pages, and loan element restricts the
 *  usage until the time when a loan with the given id was returned. MaxResolution constraint can be used only
 *  inside the print element and limits the bitmap resolution that can be sent to the printer. Permission elements
 *  that are not understood by the host (including permissions that contain constraints that are not understood)
 *  must be ignored. Top-level permissions element can contain multiple permissions of the same type (e.g. print);
 *  typically they would differ by the associated constraints. A permission of some type is considered granted if
 *  there is at least one permission element of a given type where all constraints are satisfied.
 *
 */
class License : public dp::RefCounted
{
protected:
	virtual ~License() {}

public:

	virtual int getInterfaceID() { return IID_License; }
	/**
	 *  User id to whom the license is issued (normally in the form of "urn:uuid:XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX").
     *  \commands
     *  \commandrefbr{licenseGetUserID}
	 */
	virtual dp::String getUserID() = 0;
	/**
	 *  Id of the resource for which license is issued (normally in the form of "urn:uuid:XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX").
     *  \commands
     *  \commandrefbr{licenseGetResourceID}
	 */
	virtual dp::String getResourceID() = 0;
	/**
	 *  Voucher id of the resource for which license is issued (no particular form, can be SKU or ISBN). This is normally
	 *  only used for PDF documents packaged with ACS3.
     *  \commands
     *  \commandrefbr{licenseGetVoucherID}
	 */
	virtual dp::String getVoucherID() = 0;
	/**
	 *  ID of the license signing server that signed this license
     *  \commands
     *  \commandrefbr{licenseGetLicenseURL}
	 */
	virtual dp::String getLicenseURL() = 0;
	/**
	 *  URL of the Content Server server that issued this license
     *  \commands
     *  \commandrefbr{licenseGetOperatorURL}
	 */
	virtual dp::String getOperatorURL() = 0;
	/**
	 *  Content Server fulfillment id that resulted in issuing this license (may not available, in
	 *  which case null is returned).
     *  \commands
     *  \commandrefbr{licenseGetFulfillmentID}
	 */
	virtual dp::String getFulfillmentID() = 0;
	/**
	 *  Content Server distributor id that initiated transaction that resulted in issuing this
	 *  license (may not available, in which case null is returned). Normally in the form of
	 *  "urn:uuid:XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX".
     *  \commands
     *  \commandrefbr{licenseGetDistributorID}
	 */
	virtual dp::String getDistributorID() = 0;
	/**
	 *  Name of the person to whom the license is issued if the license is bound to name. Currently,
	 *  Content Server does not issue such licenses.
     *  \commands
     *  \commandrefbr{licenseGetLicensee}
	 */
	virtual dp::String getLicensee() = 0;
	/**
	 *  Get permissions of the given type granted in this license. Type can be
	 *  one of "display", "print", "excerpt", "play".
     *  \commands
     *  \commandrefbr{licenseGetPermissions}
	 */
	virtual dp::list<Permission> getPermissions( const dp::String& type ) = 0;
	/**
	 *  For consumable permissions, this gives the remaining
	 *  number of units of the given type (print, excerpt) that can be consumed
	 *  by this user.
	 *
	 *  Note on clock manipulation protection: SDK has no means to detect that the clock is
	 *  artificially set in the future (e.g. to gain access to not-yet-incurred incremental
	 *  count). However if the clock is set in the future, some incremental permissions are
	 *  consumed and then clock is set back to the present, consumable permissions will be
	 *  blocked (and getCurrentCount would return 0) until the clock reaches the time when
	 *  consumption took place.
	 *
     *  \commands
     *  \commandrefbr{licenseGetCurrentCount}
	 */
	virtual int getCurrentCount( const dp::String& type ) = 0;
	/**
	 *  or consumable permission, consume given number of units of given
	 *  type (print, excerpt)
     *  \commands
     *  \commandrefbr{licenseConsume}
	 */
	virtual void consume( const dp::String& type, int count ) = 0;
	/**
	 *  Get license "flavor". RM SDK supports multiple methods (or "falvors") of managing content rights.
	 *  This is the list of currently supported flavors and the RM SDK version where it
	 *  first appeared:
	 *  - [empty string] - no restrictions found in file
	 *  - user - locked to an ACS4 user-activated device
	 *  - pdf - PDF permissions (PDF only)
	 *  - licensee (9.0) - locked to a licensee 
	 *  - passHash (9.1) - locked to a passhash
	 *  - user_s1 (9.1) - user, variant s1
	 *  .
	 *  This API only returns valid flavor after the license was validated. In other words, it only works
	 *  for licenses that resulted from dpdrm::Rights::getValidLicenses call.
     *  \commands
     *  \commandrefbr{licenseGetFlavor}
	 */
	virtual dp::String getFlavor() = 0;

};

/**
 *  An object that represents a particular type of permission granted to the user
 *  with constraints (e.g. expiration date).
 */
class Permission : public dp::RefCounted
{
protected:
	virtual ~Permission() {}

public:

	virtual int getInterfaceID() { return IID_Permission; }
	/**
	 *  Permission type (display, print, excerpt, play)
	 *
     *  \commands
     *  \commandrefbr{permissionGetPermissionType}
	 */
	virtual dp::String getPermissionType() = 0;
	/**
	 *  Return license expiration time or 0 is this license is permanent.
	 *
     *  \commands
     *  \commandrefbr{permissionGetExpiration}
	 */
	virtual dp::time_t getExpiration() = 0;
	/**
	 *  Return the loan id if this license is can be returned early and null if
	 *  this is not returnable license.
	 *
	 *  See
	 *  - DRMProcessor::initLoanReturnWorkflow
	 *  .
	 *
     *  \commands
     *  \commandrefbr{permissionGetLoanID}
	 */
	virtual dp::String getLoanID() = 0;
	/**
	 *  Return the device id (see Activation::getDeviceID) to which this
	 *  license is restricted. Null is returned if this constraint is missing.
	 *
     *  \commands
     *  \commandrefbr{permissionGetDeviceID}
	 */
	virtual dp::String getDeviceID() = 0;
	/**
	 *  Return the device type (see dpdev::Device::getDeviceType) to which this
	 *  license is limited. Null is returned if this constraint is missing.
	 *
     *  \commands
     *  \commandrefbr{permissionGetDeviceType}
	 */
	virtual dp::String getDeviceType() = 0;
	/**
	 *  Get maximum resolution for rendering this document. Currently this is
	 *  only applicable to print; resolution returned is in dots per inch. Return
	 *  0 if resolution is not contrained.
	 *
     *  \commands
     *  \commandrefbr{permissionGetMaxResolution}
	 */
	virtual double getMaxResoultion() = 0;
	/**
	 *  Get part bit mask that restricts access to the individual portions of the
	 *  document. Null if returned when no such constraint is given in the license.
	 *
	 *  Currently Content Server does not issue licenses with part
	 *  constraints.
	 *
     *  \commands
     *  \commandrefbr{permissionGetParts}
	 */
	virtual dp::Data getParts() = 0;
	/**
	 *  Check if there are consumption constraints for this permission.
	 *
     *  \commands
     *  \commandrefbr{permissionIsConsumable}
	 */
	virtual bool isConsumable() = 0;
	/**
	 *  If this is consumable permission, return initial number of units.
	 *
     *  \commands
     *  \commandrefbr{permissionGetInitialCount}
	 */
	virtual int getInitialCount() = 0;
	/**
	 *  If this is consumable permission, return maximum number of units. 0 means
	 *  that maximum is not limited.
	 *
     *  \commands
     *  \commandrefbr{permissionGetMaxCount}
	 */
	virtual int getMaxCount() = 0;
	/**
	 *  If this is consumable permission, return the increment interval in seconds. Each
	 *  time the increment interval time passes a new permission unit is added (not
	 *  exceeding the maximum). 0 means that consumable permission does not get incremented
	 *  with time.
	 *
     *  \commands
     *  \commandrefbr{permissionGetIncrementInterval}
	 */
	virtual double getIncrementInterval() = 0;
};

}

#endif // _DP_DRM_H
