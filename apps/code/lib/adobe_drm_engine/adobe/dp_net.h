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
 *  \file dp_net.h
 *
 *  Reader Mobile SDK public interface -- networking
 */
#ifndef _DP_NET_H
#define _DP_NET_H

#include "dp_core.h"
#include "dp_io.h"

/**
 *  Networking support
 */
namespace dpnet
{

class NetProvider;

/**
 *  Reader Mobile SDK HTTP networking interface
 */
class NetProvider : public dp::Unknown
{
public:

	virtual int getInterfaceID() { return IID_NetProvider; }
	/**
	 *  Prepare for a network request (HTTP or HTTPS, GET or POST). Request should be
	 *  initiated when dpio::Stream::requestBytes or dpio::Stream::requestInfo is called on the
	 *  returned object. When request is processed,
	 *  dpio::StreamClient methods should be called with the result obtained from the server. Note that for best
	 *  performance this call should not block waiting for result from the server. Instead the communication
	 *  to the server should happen on a separate thread or through event-based system.
	 *
	 *  Note that client can be NULL. It that case you should expect dpio::Stream::setStreamClient call
	 *  at later time.
	 */
	virtual dpio::Stream * open( const dp::String& method, const dp::String& url, dpio::StreamClient * client,
		unsigned int cap, dpio::Stream * dataToPost = 0 ) = 0;

	/**
     *  Register a network provider for use by other Reader Mobile SDK modules.
     */
	static void setProvider( NetProvider * provider );

	/**
     *  Get the previously-registered NetProvider.
     */
	static NetProvider * getProvider();
};

}

#endif // _DP_NET_H
