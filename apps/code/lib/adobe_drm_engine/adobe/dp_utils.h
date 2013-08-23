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
 *  \file dp_utils.h
 *
 *  Reader Mobile SDK public interface -- utility objects
 */
#ifndef _DP_UTILS_H
#define _DP_UTILS_H

#include <dp_io.h>

/**
 *  Reader Mobile SDK utility objects
 */
namespace dputils
{

/**
 * Guard prevents dputils::GuardedObject from being destroyed until this
 * Guard object is destroyed.
 */ 
template <class Guarded>
class Guard
{
public:

	/**
	 *  Construct dputils::Guard that prevents guarded object from being destroyed prematurely.
	 */
	Guard( Guarded * guarded ) : 
		m_guarded(guarded) 
	{ 
		guarded->m_inUse++; 
	}

	~Guard() 
	{
		if( --m_guarded->m_inUse == 0 && m_guarded->m_released ) 
			m_guarded->deleteThis(); 
	}

private:
	Guarded * m_guarded;
};

/**
 *  Convenience base class for guared objects.
 */
class GuardedObject
{
public:

	GuardedObject() : 
		m_inUse(0), 
		m_released(false) 
	{
	}

	virtual void release() 
	{
		m_released = true; 
		if( m_inUse == 0 ) 
			deleteThis(); 
	}

	virtual void deleteThis() = 0;

protected:
	int m_inUse;
	bool m_released;
};

/**
 *  Convenience base class to implement guarded dpio::Stream object. A challenge with dpio::Stream
 *  iterface implementation is that its dpio::Stream::release method can be called from within
 *  dpio::StreamClient callback which is normally called from other dpio::Stream method.
 */
class GuardedStream : public dpio::Stream, public dputils::GuardedObject
{
	friend class dputils::Guard<dputils::GuardedStream>;

public:

	GuardedStream() : 
		m_receiver(NULL) 
	{
	}

	virtual void release() 
	{
		m_receiver = NULL; 
		GuardedObject::release(); 
	}

	virtual void setStreamClient( dpio::StreamClient * receiver ) 
	{ 
		m_receiver = receiver; 
	}

protected:

	/**
	 *  dpio::StreamClient that was assigned to this stream by dpio::Stream::setStreamClient
	 */
	dpio::StreamClient *	m_receiver;
};

typedef Guard<GuardedStream> StreamGuard;

}

#endif