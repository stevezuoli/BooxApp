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
 *  \file dp_crypt.h
 *
 *  Reader Mobile SDK public interface -- bootstrapping helper class
 */
#ifndef _DP_BOOTSTRAP_H
#define _DP_BOOTSTRAP_H

#include "dp_all.h"

/**
 *  Reader Mobile SDK bootstrapping utilities (useful for other programming language bindings)
 */
namespace dpbootstrap
{

/**
 *  Helper class to access SDK static C++ methods through class interface
 */
class Bootstrap : public dp::Unknown
{
public:

	virtual int getInterfaceID()
	{
		return IID_Bootstrap;
	}
	
	dpio::Stream* createDataStream(const dp::String& contentType, const dp::Data& data, dpio::StreamClient* client, dp::Callback* callback)
	{
		return dpio::Stream::createDataStream( contentType, data, client, callback );
	}
    
	dpio::Stream* createDataURLStream(const dp::String& dataURL, dpio::StreamClient* client, dp::Callback* callback)
	{
		return dpio::Stream::createDataURLStream( dataURL, client, callback );
	}
    
	dp::Data readSynchronousStream(dpio::Stream* stream)
	{
		return dpio::Stream::readSynchronousStream( stream );
	}
    
	void setVersionInfo(const dp::String& name, const dp::String& ver)
	{
		dp::setVersionInfo( name, ver );
	}
    
	dp::String getVersionInfo(const dp::String& name)
	{
		return dp::getVersionInfo( name );
	}
    
	dpio::Partition* findPartitionForURL(const dp::String& fileURL)
	{
		return dpio::Partition::findPartitionForURL( fileURL );
	}
    
	bool isMobileOS()
	{
		return dpdev::isMobileOS();
	}
    
	void addDeviceListener(dpdev::DeviceListener* listener)
	{
		dpdev::DeviceProvider::addListener(listener);
	}
    
	dpdev::DeviceListener* getDeviceMasterListener()
	{
		return dpdev::DeviceProvider::getMasterListener();
	}
    
	int addDeviceProvider(dpdev::DeviceProvider* provider)
	{
		return dpdev::DeviceProvider::addProvider(provider);
	}
    
	dpdev::DeviceProvider* getDeviceProvider(int index)
	{
		return dpdev::DeviceProvider::getProvider(index);
	}
    
	bool mountRemovablePartition(const dp::String& rootURL, const dp::String& name, const dp::String& type)
	{
		return dpdev::DeviceProvider::mountRemovablePartition( rootURL, name, type );
	}
    
	bool unmountRemovablePartition(const dp::String& rootURL)
	{
		return dpdev::DeviceProvider::unmountRemovablePartition( rootURL );
	}
    
	void addDocumentProvider(dpdoc::DocumentProvider* provider)
	{
		dpdoc::DocumentProvider::addProvider(provider);
	}
    
	dpdoc::Document* createDocument(dpdoc::DocumentClient* client, const dp::String& mimeType)
	{
		return dpdoc::Document::createDocument( client, mimeType );
	}
    
	dpdrm::DRMProvider* getDRMProvider()
	{
		return dpdrm::DRMProvider::getProvider();
	}
    
	dplib::Library* getPartitionLibrary(dpio::Partition* partition)
	{
		return dplib::Library::getPartitionLibrary(partition);
	}
    
	dpnet::NetProvider* getNetProvider()
	{
		return dpnet::NetProvider::getProvider();
	}
    
	void setNetProvider(dpnet::NetProvider* provider)
	{
		dpnet::NetProvider::setProvider(provider);
	}
    
	dpres::ResourceProvider* getResourceProvider()
	{
		return dpres::ResourceProvider::getProvider();
	}
    
	void setResourceProvider(dpres::ResourceProvider* provider)
	{
		dpres::ResourceProvider::setProvider(provider);
	}
    
	void setCryptProvider(dpcrypt::CryptProvider* provider)
	{
		dpcrypt::CryptProvider::setProvider(provider);
	}
    
	dpcrypt::CryptProvider* getProvider()
	{
		return dpcrypt::CryptProvider::getProvider();
	}
    
	void setTimerProvider(dptimer::TimerProvider* provider)
	{
		dptimer::TimerProvider::setProvider(provider);
	}
    
	dptimer::TimerProvider* getTimerProvider()
	{
		return dptimer::TimerProvider::getProvider();
	}
    
};

}

#endif // _DP_BOOTSTRAP_H