/*************************************************************************
*
* ADOBE CONFIDENTIAL
* ___________________
*
*  Copyright 2010 Adobe Systems Incorporated
*  All Rights Reserved.
*
* NOTICE:  All information contained herein is, and remains
* the property of Adobe Systems Incorporated and its suppliers,
* if any.  The intellectual and technical concepts contained
* herein are proprietary to Adobe Systems Incorporated and its
* suppliers and are protected by trade secret or copyright law.
* Dissemination of this information or reproduction of this material
* is strictly forbidden unless prior written permission is obtained
* from Adobe Systems Incorporated.
**************************************************************************/
/**
 *  \file dp_res.h
 *
 *  Reader Mobile SDK public interface -- global resources
 */
#ifndef _DP_RES_H
#define _DP_RES_H

#include "dp_core.h"
#include "dp_io.h"

/**
 *  Global support
 */
namespace dpres
{

class ResourceProvider;

/**
 *  Reader Mobile SDK global resource interface. A global resource is valid
 *  throughout the applications run and may remain cached even when all
 *  documents are closed.
 */
class ResourceProvider : public dp::Unknown
{
public:

	virtual int getInterfaceID() { return IID_ResourceProvider; }

	/**
	 *  Request a global resource download from a given url with a Stream with at least
	 *  given capabilities. Security considerations are responsibilities of the host.
	 *  If NULL is returned, request is considered to be failed.
	 */
	virtual dpio::Stream * getResourceStream( const dp::String& urlin, unsigned int capabilities ) = 0;

	/**
     *  Register a resource provider for use by other Reader Mobile SDK modules.
     */
	static void setProvider( ResourceProvider * provider );

	/**
     *  Get the previously-registered ResourceProvider.
     */
	static ResourceProvider * getProvider();
};

}

#endif // _DP_RES_H
