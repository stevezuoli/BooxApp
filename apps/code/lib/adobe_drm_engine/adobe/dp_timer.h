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
 *  \file dp_timer.h
 *
 *  Reader Mobile SDK public interface -- timer (delayed execution)
 */
#ifndef _DP_TIMER_H
#define _DP_TIMER_H

#include "dp_core.h"

/**
 *  Reader Mobile SDK timer services
 */
namespace dptimer
{

class TimerProvider;
class Timer;
class TimerClient;

/**
 *  Timer services provider. Can be used to create a timer.
 */
class TimerProvider : public dp::Unknown
{
protected:
	virtual ~TimerProvider() {}

public:
	virtual int getInterfaceID() { return IID_TimerProvider; }
	/**
	 *  Create a new Timer object that will call back a given TimerClient. Given TimerClient
     *  object must exist until the Timer is released.
	 */
	virtual Timer * createTimer( TimerClient * client ) = 0;
	/**
	 *  Set TimerProvider that can be used by other modules of Reader Mobile SDK.
	 */
	static void setProvider( TimerProvider * provider );
	/**
	 *  Return the previously-registered TimerProvider.
	 */
	static TimerProvider * getProvider();
};

/**
 *  Timer object that can be used to schedule a call to TimerClient at a later time.
 */
class Timer : public dp::Releasable
{
protected:
	virtual ~Timer() {}

public:

	virtual int getInterfaceID() { return IID_Timer; }
	/**
	 *  Release this object. Timer can no longer be used after this call is made.
	 *  Scheduled timeout (if any) is cancelled.
	 */
	virtual void release() = 0;
	/**
	 *  Schedule a call to this Timer's TimerClient in the given number of milliseconds.
	 *  0 means call as soon as possible.
	 */
	virtual void setTimeout( int millisec ) = 0;
	/**
	 *  Cancel previously scheduled timeout, if any.
	 */
	virtual void cancel() = 0;
};

/**
 *  Interace that the Timer calls when the timeout expires.
 */
class TimerClient : public dp::Unknown
{
protected:
	virtual ~TimerClient() {}

public:
	virtual int getInterfaceID() { return IID_TimerClient; }
	/**
     *  Called by the Timer when the scheduled timeout expires.
     */
	virtual void timerFired( Timer * timer ) = 0;
};

}

#endif // _DP_TIMER_H
