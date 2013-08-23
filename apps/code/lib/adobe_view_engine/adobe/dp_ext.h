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
 *  \file dp_ext.h
 *
 *  Reader Mobile SDK public interface -- interfaces for external devices with loadable driver modules
 */
#ifndef _DP_EXT_H
#define _DP_EXT_H

/**
 *  External devices with loadable driver modules. These interfaces are specifically
 *  designed to not require any additional include files.
 */
namespace dpext
{

/**
 *  Supported loadable device types. This corresponds to dpdev::Device::getDeviceType.
 */
enum LoadableDeviceType
{
	DT_MOBILE = 3,		/**< "mobile" - handheld on/off-line attachable device */
	DT_TETHERED = 4,	/**< "tethered" - handheld off-line attachable device */
	DT_STORAGE = 5,		/**< "storage" - storage-only attachable device */
	DT_TOKEN = 7		/**< "token" - storageless attachable device */
};

/**
 *  Interface that loadable devices use
 */
class LoadableDeviceInfo
{
protected:

	virtual ~LoadableDeviceInfo() {}

public:

	/**
	 *  \return must return 1.
	 */
	virtual int getInterfaceVersion() = 0;
	/**
	 *  must return NULL
	 */
	virtual void * getOptionalInterface( const char * name ) = 0;
	/**
	 *  Reads the device name. Returned string must remain valid until this device is released
	 */
	virtual const char * getDeviceName() = 0;
	/**
	 *  Reads the device fingerprint; validation parameter must be 1.
	 */
	virtual const unsigned char * getFingerprint( size_t * fingerprintLengthOut, int validation,
		const unsigned char * nonce, size_t nonceSize ) = 0;
	/**
	 *  Reads the device key. The size of the key is written to the keyLength. 
	 */
	virtual const unsigned char * getKey( size_t * keyLength ) = 0;
	/**
	 *  Reads the device type. Should be one of the constantas defined by dpdev::LoadableDeviceType
	 */
	virtual int getDeviceType() = 0;
	/**
	 *  Returns the activation info (as XML) if the device is activated, NULL if it is not.
	 *  Returned string must remain valid until this device is released or until either
	 *  getDeviceActivationInfo() or setDeviceActivationInfo().
	 */
	virtual const char * getDeviceActivationInfo() = 0;
	/**
	 *  Sets the activation information for the device (as XML).
	 */
	virtual void setDeviceActivationInfo( const char * activation ) = 0;
	/**
	 *  Get the folders that allow filesystem access to this device. Zero-based index can be used to iterate
	 *  through the individual partitions on the device. Built-in partitions should always come first.
	 *  Returns NULL if index is out of range. Returned string must remain valid until this device is released
	 *  or until next getDevicePartitionPath call.
	 */
	virtual const char * getDevicePartitionPath( size_t index ) = 0;
	/**
	 *  Get the name of the given device partition.
	 */
	virtual const char * getDevicePartitionName( size_t index ) = 0;
	/**
	 *  Get the type of the given device partition. See dpio::Partition::getPartitionType
	 */
	virtual const char * getDevicePartitionType( size_t index ) = 0;
	/**
	 *  Get version information for the software installed on the device. See dp::setVersionInfo for
	 *  the specific strings.
	 */
	virtual const char * getVersionInfo( const char * name ) = 0;
	/**
	 *  Release this object.
	 */
	virtual void release() = 0;
	/**
	 *  Not used
	 */
	virtual bool reserved1() = 0;
};

/**
 *  An iterface to register for device attachments/detachments notifications.
 */
class LoadableDeviceIteratorListener
{
protected:

	virtual ~LoadableDeviceIteratorListener() {}

public:

	/**
	 *  Called by a device iterator when the device list or device itself (e.g. attached partitions) changes.
	 */
	virtual void deviceListChanged() = 0;
};

/**
 *  An object to iterate loadable devices attached to this system. To implement a device type,
 *  implement your own device iterator
 */
class LoadableDeviceIterator
{
protected:

	virtual ~LoadableDeviceIterator() {}

public:

	/**
	 *  Release this object
	 */
	virtual void release() = 0;
	/**
	 *  return 1.
	 */
	virtual int getInterfaceVersion() = 0;
	/**
	 *  return NULL.
	 */
	virtual void * getOptionalInterface( const char * name ) = 0;
	/**
	 *  Never gets called
	 */
	virtual void reserved1() = 0;
	/**
	 *  Never gets called
	 */
	virtual void reserved2( void * host ) = 0;
	/**
	 *  Gets the count of the devices
	 */
	virtual size_t getDeviceCount() = 0;
	/**
	 *  Get information about the specified device. Index 0 always refers to the machine itself, other devices may
	 *  be attached devices. Returns NULL if index if out of range (this can be used to enumerate all the attached
	 *  devices).
	 */
	virtual LoadableDeviceInfo * getDeviceInfo( size_t index ) = 0;
	/**
	 *  Adds a listener which will be notified when devices are plugged or unplugged.
	 */
	virtual void addDeviceIteratorListener( LoadableDeviceIteratorListener * other ) = 0;
	/**
	 *  Removes a listener.
	 */
	virtual void removeDeviceIteratorListener( LoadableDeviceIteratorListener * other ) = 0;
};

}

#endif