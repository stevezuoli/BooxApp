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
 *  \file dp_fuse.h
 *
 *  Reader Mobile SDK public interface -- SDK initialization and integration
 */
#ifndef _DP_FUSE_H
#define _DP_FUSE_H

#include "dp_timer.h"

namespace dp
{

/**
 *  Register PDF DocumentProvider
 */
void documentRegisterPDF();

/**
 *  Register OPS DocumentProviders (EPUB XML-based content types: XHTML, SVG, DTBook).
 */
void documentRegisterOPS();

/**
 *  Register bitmap image DocumentProviders (EPUB content types: JPEG, PNG and GIF).
 */
void documentRegisterImages();

/**
 *  Register EPUB DocumentProvider (this calls dp::documentRegisterOPS() and
 *  dp::documentRegisterImages() internally).
 */
void documentRegisterEPUB();

/**
 *  Register standard primary platform DeviceProvider
 */
void deviceRegisterPrimary();

/**
 *  Register standard DeviceProviders for external devices
 */
void deviceRegisterExternal();

/**
 *  Enumerate removable partitions and call dpdev::DeviceProvider::mountRemovablePartition
 *  for each of them
 */
void deviceMountRemovablePartitions();

/**
 *  Register OpenSSL-based CryptProvider
 */
void cryptRegisterOpenSSL();

/**
 *  dptimer::TimerClient object that can be used by a master timer in timerRegisterMasterTimer
 */
dptimer::TimerClient * timerGetMasterClient();

/**
 *  Register dptimer::TimerProvider implementation that multiplexes all timer requests into a single
 *  dptimer::Timer object. The provided object (master timer) should use dp::timerGetMasterClient()
 *  to get appropriate TimerClient (master timer client).
 */
void timerRegisterMasterTimer( dptimer::Timer * timer );

/**
 *  Flags for dp::platformInit \a support parameter 
 */
enum PlatformInit
{
	PI_LIBRARY = 0x01,				/**< dplib::Library support */
	PI_GENERIC_DEVICES = 0x10,		/**< Generic mass-storage device */
	PI_LOADABLE_DEVICES = 0x20,		/**< Loadable devices */
	PI_ACTIVESYNC_DEVICES = 0x40,	/**< ActiveSync device support (meaningful on Windows only) */
	PI_DEFAULT = 0xFFFFFFFF			/**< Default SDK init (all features) */
};

/**
 *  Return codes for dp::platformInit
 */
enum InitStatus
{
	IS_OK,
	IS_DUPLICATE_CALL,
	IS_ALREADY_RUNNING,
	IS_FAIL
};

/**
 *  Initialize platform-dependent SDK support
 */
int platformInit( unsigned int support );

/**
 *  Broadcast message to all other applications that use SDK.
 *  \param messageURI message URI, e.g. "http://example.com/mymessage"
 *  \param data message body
 */
void broadcast( const dp::String& messageURI, const dp::Data& data );

/**
 *  Broadcast message callback type
 */
typedef void (*broadcastFunc_t)( const dp::String& messageURI, const dp::Data& data );

/**
 *  Register a callback for broadcast messages. Callback will be called when
 *  another application sends a broadcast message. Messages are not broadcasted
 *  within the application itself.
 */
void registerForBroadcast( broadcastFunc_t func );

#ifndef WIN32

/**
 *  Get UNIX file descriptor used to listen for broadcast messages (see dp::broadcast and 
 *  dp::registerForBroadcast). When this file descriptor becomes readable, some broadcast
 *  messages have arrived and dp::processBroadcasts should be called. This can be used in
 *  select system call.
 *  
 *  This method is not available on Windows platforms.
 */
int getBroadcastFD();
/**
 *  Read and dispatch pending broadcast messages. If no messages are ready it returns
 *  immediately. See dp::getBroadcastFD() to get file descriptor that can be used to
 *  detect messages being ready.
 *  
 *  This method is not available on Windows platforms.
 */
void processBroadcasts();

#endif

}

#endif // _DP_FUSE_H

