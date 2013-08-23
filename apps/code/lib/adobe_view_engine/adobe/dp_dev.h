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
 *  \file dp_dev.h
 *
 *  Reader Mobile SDK public interface -- DRM-capable devices
 */
#ifndef _DP_DEV_H
#define _DP_DEV_H

#include "dp_types.h"
#include "dp_core.h"
#include "dp_io.h"

/**
 *  Reader Mobile SDK DRM-capable device support
 *
 *  Reader Mobile SDK environment is assumed to contain a certain number of devices. Each device is
 *  represented by dpdev::Device interface. Most handheld devices should only have a single dpdev::Device
 *  object that represents the device itself.  
 *
 *  Each device provides access to its storage through partitions represented by dpio::Partition. Each
 *  partition can be thought as a filesystem. Partition 0 corresponds to the device internal
 *  storage (if any) and partitions 1, 2, etc. correspond to removable storage (e.g. memory cards or USB
 *  mass storage).
 *
 *  Each device has fingerprint - a small piece of binary data which is most likely to be different for
 *  different physical devices. Fingerprint serves as an anchor for the DRM system.
 *
 *  Devices should be able to hold a small piece of data called activation record. Activation record can
 *  store a signed token that binds a particular user to the device fingerprint. When this token is present
 *  on in the device activation record, the device is said to be "activated" for that user.
 *
 *  Devices can be trusted or not. A trusted activated device has to be present in the environment
 *  to be able to open DRM-protected documents licensed to a particular user. Only devices which can be
 *  reasonably belived to be in physical proximity can be trusted (e.g. the device where the content
 *  is being viewed is always trusted, devices plugged with USB can be trusted or not, devices connected
 *  through network probably cannot be trusted). Non-trusted devices can still be activated.
 */
namespace dpdev
{

class DeviceProvider;
class DeviceListener;
class Device;

/**
 *  A class that manages a particular kind of devices.
 */
class DeviceProvider : public dp::Unknown
{
protected:
	virtual ~DeviceProvider() {}

public:

	virtual int getInterfaceID() { return IID_DeviceProvider; }
	/**
	 *  Get the DeviceProvider identifier. Identifier is a short string that identifies a 
	 *  DeviceProvider for testing and diagnostic purposes.
	 *
	 *  Currently used identifiers:
	 *  - <b>primary</b> - device itself
	 *  - <b>generic</b> - generic mass-storage-based device
	 *  - <b>activesync</b> - device connected through Windows ActiveSync
	 *  .
	 *
	 *  In addition, loadable devices are identified by the key that they use in registry/preferences
	 *  to identify themselves.
	 *
     *  \commands
     *  \commandrefbr{createDevice}
     *  \commandrefbr{deviceGetProviderIdentifier}
	 */
	virtual dp::String getIdentifier() = 0;
	/**
	 *  Get this DeviceProvider index in the list of providers. When this provider is added
	 *  to the list DeviceProvider::addProvider will return the index that just added provider
	 *  should use.
     *  \commands
     *  \commandrefbr{deviceGetProviderIdentifier}
	 */
	virtual int getIndex() = 0;
	/**
	 *  Get a particular device that this DeviceProvider manages. Returns null if index is out
	 *  of range.
	 */
	virtual Device * getDevice( int index ) = 0;
	/**
	 *  Notify the device provider that a new partition was attached to this machine. If DeviceProvider
	 *  determines that a newly inserted partition belongs to the device kind that it supports, it should
	 *  create a new device (or device partition), notify master listener (which will notify all
	 *  resgistered device listeners) and return true. Otherwise return false.
     *  \commands
     *  \commandrefbr{createDevice}
	 */
	virtual bool mount( const dp::String& rootURL, const dp::String& name, const dp::String& type ) = 0;
	/**
	 *  Notify the device provider that a new partition was detached from this machine. If DeviceProvider
	 *  determines that the detached partition belongs to one of devices it manages, it should
	 *  notify master listener (which will notify all resgistered device listeners), remove the new device
	 *  (or just the device partition), and return true. Otherwise return false.
	 */
	virtual bool unmount( const dp::String& rootURL ) = 0;
	/**
	 *  Register a new device listener. All device listeners are global and are notified about events
	 *  relevant for any device.
     *  \commands
     *  \commandrefbr{deviceAddListener}
	 */
	static void addListener( DeviceListener * listener );
	/**
	 *  Get the "master listener" that can be used to notify all the listeners registered using 
	 *  DeviceProvider::addListener. All dpdev::DeviceListener notifications must go through the master
	 *  listener to make sure that all listeners are notified.
	 */
	static DeviceListener * getMasterListener();
	/**
	 *  Register a new DeviceProvider. Returns an index
	 *  that this DeviceProvider is assigned. DeviceProvider cannot be deregistered and
	 *  its index never changes.
	 */
	static int addProvider( DeviceProvider * provider );
	/**
	 *  Get DeviceProvider by index. Returns NULL when index is out of range.
     *  \commands
     *  \commandrefbr{createDevice}
	 */
	static DeviceProvider * getProvider( int index );
	/**
	 *  Notify DeviceProviders that a removable partition added. Document
	 *  path for removable partitions is always "Digital Editions" subfolder for rootURL folder.
	 */
	static bool mountRemovablePartition( const dp::String& rootURL, const dp::String& name, const dp::String& type );
	/**
	 *  Notify DeviceProviders that a removable partition removed
	 */
	static bool unmountRemovablePartition( const dp::String& rootURL );

};

/**
 *  DRM-capable device
 *
 *  dp::Unknown::getOptionalInterface handling:
 *  For Windows Mobile devices using name "IRAPISession" will return a pointer
 *  to IRAPISession object. That object has to be released with IUnknown::Release method.
 */
class Device : public dp::Unknown
{
protected:
	virtual ~Device() {}

public:

	virtual int getInterfaceID() { return IID_Device; }
	/**
	 *  Request device to obtain its key. This may involve asynchronous process (e.g. communicating
	 *  with the user through non-modal dialog: entering password, accessing keychain, etc.). When
	 *  the key acquisition process is finished (either with success or failure),
	 *  dpdev::DeviceListener::deviceKeyReady method is called.
	 */
	virtual void prepareDeviceKey() = 0;
	/**
	 *  Get dpdev::DeviceProvider that manages this device.
     *  \commands
     *  \commandrefbr{deviceGetProviderIndex}
     *  \commandrefbr{deviceGetProviderIdentifier}
	 */ 
	virtual DeviceProvider * getProvider() = 0;
	/**
	 *  Get this device index in this device provider's list (in other words, index that should be passed to
	 *  dpdev::DeviceProvider::getDevice method to obtain this device).
     *  \commands
     *  \commandrefbr{deviceGetIndex}
	 */
	virtual int getIndex() = 0;
	/**
	 *  Read device human-readable name. If a user can give a name to a particular device instance,
	 *  it should be returned here. In other words, it should say something like "John's PDA" rather
	 *  than "Acme FX 340"
     *  \commands
     *  \commandrefbr{deviceGetDeviceName}
	 */
	virtual dp::String getDeviceName() = 0;
	/**
	 *  Reads the device type. These are currently supported types:
	 *  - \e standalone - represents a PC (i.e. non-handheld) computer in normal settings
	 *  - \e public - represents a PC computer in "kiosk" mode (many one-session users)
	 *  - \e tethered - represents a handheld device that does not have network access on its own
	 *  - \e mobile - represents a handheld device that can potentially have network access on its own
	 *  .
     *  \commands
     *  \commandrefbr{deviceGetDeviceType}
	 */
	virtual dp::String getDeviceType() = 0;
	/**
	 *  Reads the device fingerprint. Fingerprint is a small piece of data that identifies a particular
	 *  device instance. It is recommended that fingerprints are 20 bytes long and it must not be longer.
	 *  Different devices must have different fingerprints, although coincidentally duplicate fingerprints
	 *  can be tolerated. 
	 *
	 *  For custom device implementation, fingerprint calculation algorithm should be reviewed by Adobe.
	 *  Fingerprint should be always calculated based on the actual hardware (e.g. by reading serial number)
	 *  and not taken from some sort of data storage that can be accessed and modified by the user. If it
	 *  cannot be done (e.g. for generic mass-storage device interface), isTrusted method must return
	 *  false. Specifically, on devices which expose generic mass-storage interface (through device.xml and
	 *  activation.xml files) fingerprint <b>must not</b> be read from either of these files.
     *  \commands
     *  \commandrefbr{deviceGetFingerprint}
	 */
	virtual dp::Data getFingerprint() = 0;
	/**
	 *  Reads the device key. Device key should be a random device-specific sequence of bytes that is
	 *  stored securely on the device (e.g. in Key Chain or encrypted by Data Protection APIs). It is
	 *  imperative that once user credentials are written to the device, it is not possible for
	 *  an attacker to recover the key (e.g. in case the device is lost). If the device supports PIN
	 *  or some sort of login and user opts not to use it, the required level of protection
	 *  is not being able to read the key without modifying device hardware and/or software. If such key is
	 *  not feasible to implement on a device, a device <b>must</b> return NULL. Such devices won't
	 *  have user credentials written in the activation info (see dpdrm::Activation::hasCredentials),
	 *  so they won't be able communicate with network servers for activation and fulfillment.
	 *
	 *  Device key must be 16 byte long.
	 *
	 *  Adobe-provided implementations for MacOS and Windows (both mobile and desktop) and generic 
	 *  mass storage device satisfy that requirement. Linux device implementation does not have that
	 *  capability and thus cannot be used "as is".
	 *
	 *  Here are some strategies for secure key implementation.
	 *  - Generate random key using secure randomness source (e.g. dpcrypt::CryptProvider::getRandomBytes).
	 *  - On the device, store the key encrypted by a key derived from the user-entered PIN (if user set it up)
	 *    or some kind of hardware-dependent key (if user opted out of using PIN). If the PIN is short,
	 *    it may be a good idea to erase the key after several unsuccessful attempts to enter it.
	 *  - If implementing custom PC-side dpdev::Device interface to communicate to the connected
	 *    device, implement handshake protocol, where user has to enter device PIN (either on the device
	 *    itself or on the PC) before communicating the key to the desktop. Device authentication (e.g. PIN)
	 *    can be cached in secure storage on PC, so that user does not have to enter it again.
	 *  - alternatively, only provide key to PC until pkcs12 element is written in the activation record
	 *    (pkcs12 element contains user's credential encrypted with the device key). Once pkcs12 element is
	 *    written, don't transmit the key from the device and return null from this method.
	 *  .
     *  \commands
     *  \commandrefbr{deviceGetDeviceKey}
	 */
	virtual dp::Data getDeviceKey() = 0;
	/**
	 *  Returns the activation info (as XML) if the device is activated, null or empty string if it is not.
	 *  Activation info is a small amount of information that DRM system needs to store on the device. It
	 *  is best when it is stored in the location not directly accessible by the user.
     *  \commands
     *  \commandrefbr{deviceGetActivationRecord}
	 */
	virtual dp::Data getActivationRecord() = 0;
	/**
	 *  Set the activation info (as XML). To deactivate the device pass null or empty string.
     *  \commands
     *  \commandrefbr{deviceSetActivationRecord}
	 */
	virtual void setActivationRecord( const dp::Data& data ) = 0;
	/**
	 *  Get the partition that allow filesystem access to this device. Zero-based index can be used to iterate
	 *  through the individual partitions on the device. Built-in partitions should always come first.
	 *  Returns NULL if index is out of range.
     *  \commands
     *  \commandrefbr{createPartition}
	 */
	virtual dpio::Partition * getPartition( int index ) = 0;
	/**
	 *  Get version information for the software installed on the device. See dp::setVersionInfo for
	 *  the specific strings.
     *  \commands
     *  \commandrefbr{deviceGetVersionInfo}
	 */
	virtual dp::String getVersionInfo( const dp::String& name ) = 0;
	/**
	 *  Check if this device's fingerprint can be trusted. dpdev::Device implementations that represent
	 *  the computer/device itself should return true. Other devices may return true if device identity
	 *  can be reasonably assured.
     *  \commands
     *  \commandrefbr{deviceIsTrusted}
	 */
	virtual bool isTrusted() = 0;
};

/**
 *  Interface that gets notified when devices or partitions are added or removed.
 */
class DeviceListener : public dp::Unknown
{
protected:
	virtual ~DeviceListener() {}

public:
	virtual int getInterfaceID() { return IID_DeviceListener; }
	/**
	 *  Report an error occuring with a particular device
	 */
	virtual void reportError( int providerIndex, int deviceIndex, const dp::String& errorCode ) = 0;
	/**
	 *  Called when device key requested by dpdev::Device::prepareDeviceKey is either ready or
	 *  unavialable.
	 */
	virtual void deviceKeyReady( int providerIndex, int deviceIndex ) = 0;
	/**
	 *  Called when a new device is added
	 */
	virtual void deviceAdded( int providerIndex, int deviceIndex, Device * device ) = 0;
	/**
	 *  Called when existing device is removed
	 */
	virtual void deviceRemoved( int providerIndex, int deviceIndex, Device * device ) = 0;
	/**
	 *  Called when a new partition is added
	 */
	virtual void partitionAdded( int providerIndex, int deviceIndex, int partitionIndex, dpio::Partition * partition ) = 0;
	/**
	 *  Called when existing partition is removed
	 */
	virtual void partitionRemoved( int providerIndex, int deviceIndex, int partitionIndex, dpio::Partition * partition ) = 0;
};

/**
 * returns true if this is mobile/embedded OS
 * this affects how primary partition and library (manifest) is layed out
 */
bool isMobileOS();

}

#endif