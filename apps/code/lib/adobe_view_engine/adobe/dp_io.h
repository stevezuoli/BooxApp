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
 *  \file dp_io.h
 *
 *  Reader Mobile SDK public interface -- input/output
 */
#ifndef _DP_IO_H
#define _DP_IO_H

#include "dp_core.h"

namespace dpdev
{
class Device;
}

/**
 *  Reader Mobile SDK I/O services
 */
namespace dpio
{

class Stream;
class StreamClient;
class Partition;


/**
 *  Functionality that a Stream provides.
 */
enum StreamCapabilities
{
	SC_SYNCHRONOUS = 1,		/**< Stream will always provide requested data before the call requesting it returns. */ 
	SC_BYTE_RANGE = 2,		/**< Stream can be accessed in arbitrary order. */ 
	SC_MAPPABLE = 4			/**< Stream can be mapped into memory */
};

/**
 *  \brief Interface that describes a (potentially asynchronous) data stream that one can get data from. See \ref streams.
 */
class Stream : public dp::Releasable
{
protected:

	virtual ~Stream() {}

public:

	virtual int getInterfaceID() { return IID_Stream; }
	/**
	 *  Release this stream. Stream methods must not be called after this call.
	 *  Stream must not call its StreamClient methods after this call. This method
	 *  may be called even from within StreamClient callbacks. dputils::GuardedStream
	 *  can simplify correct implementation.
	 */
	virtual void release() = 0;
	/**
	 *  Set receiver of the stream data and information about the stream.
	 */
	virtual void setStreamClient( StreamClient * receiver ) = 0;
	/**
	 *  Get stream capabilities.
	 */
	virtual unsigned int getCapabilities() = 0;
	/**
	 *  Request information (mime type and length) about the stream. Typically for non-random-access
	 *  streams this call will be followed by the request for the data. Implementors may request
	 *  data from underlying system (e.g issue HTTP GET instead of HTTP POST), but coming data must
	 *  not be passed to the StreamReceiver until it is explicitly requested with requestBytes().
	 */
	virtual void requestInfo() = 0;
	/**
	 *  Request the content of the stream.
	 */
	virtual void requestBytes( size_t offset, size_t len ) = 0;
	/**
	 *  Report I/O erors when writing this stream
	 */
	virtual void reportWriteError( const dp::String& error ) = 0;
	/**
	 *  Utility function to create a stream from data and content type. Callback (if any) is called
	 *  when the stream is released
	 */
	static Stream * createDataStream( const dp::String& contentType, const dp::Data& data, 
		StreamClient * client, dp::Callback * callback );
	/**
	 *  Utility function to create a stream from "data:" URL. Callback (if any) is called
	 *  when the stream is released
	 */
	static Stream * createDataURLStream( const dp::String& dataURL, StreamClient * client,
		dp::Callback * callback );
	/**
	 *  Utility function that reads and releases the stream fully if it is synchronous and data is available
	 *  immediately
	 */
	static dp::Data readSynchronousStream( Stream * stream );
};

/**
 *  \brief Interface that receives the data from a Stream.
 */
class StreamClient : public dp::Unknown
{
protected:

	virtual ~StreamClient() {}

public:

	virtual int getInterfaceID() { return IID_StreamClient; }
	/**
	 *  Deliver stream property, e.g. HTTP header, such as "Content-Type".
	 *  If content type is unknown, "application/octet-stream" will be passed.
	 */
	virtual void propertyReady( const dp::String& name, const dp::String& value ) = 0;
	/**
	 *  Deliver information about total stream length in bytes.
	 */
	virtual void totalLengthReady( size_t length ) = 0;
	/**
	 * Signal that all the properties are delivered
	 */
	virtual void propertiesReady() = 0;
	/**
	 *  Deliver a chunk of the stream's content.
	 */
	virtual void bytesReady( size_t offset, const dp::Data& data, bool eof ) = 0;
	/**
	 *  report an error
	 */
	virtual void reportError( const dp::String& error ) = 0;
};

/**
 *  An object that represents a "fixed" or "removable" virtual file system partition. A partition can be
 *  based on a real OS file system or on some other I/O APIs (e.g. AcriveSync).
 */
class Partition : public dp::Unknown
{
protected:

	virtual ~Partition() {}

public:

	virtual int getInterfaceID() { return IID_Partition; }
	/**
	 *  Get the dpdev::Device where this Partition is located.
	 */
	virtual dpdev::Device * getDevice() = 0;
	/**
	 *  Get the index of this partition (in terms of dpdev::Device::getPartition method)
	 *
     *  \commands
     *  \commandrefbr{partitionGetIndex}
	 */
	virtual int getIndex() = 0;
	/**
	 *  Get this partition name (human readable). This can be, for instance, volume label.
	 *
     *  \commands
     *  \commandrefbr{partitionGetPartitionName}
	 */
	virtual dp::String getPartitionName() = 0;
	/**
	 * Returns partition type. Currently defined names (case-sensitive!):
	 *  <ul>
	 *  <li><b>Fixed</b> - permanently attached to the device</li>
	 *  <li><b>Removable</b> - can be detached from the device (generic)</li>
	 *  <li><b>MMC</b> - MMC Card</li>
	 *  <li><b>CompactFlash</b> - Compact Flash Card</li>
	 *  <li><b>SmartMedia</b> - Smart Media Card</li>
	 *  <li><b>SecureDigital</b> - Secure Digital Card</li>
	 *  <li><b>MemoryStick</b> - Memory Stick Card</li>
	 *  <li><b>xD</b> - xD Card</li>
	 *  </ul>
	 *
     *  \commands
     *  \commandrefbr{partitionGetPartitionType}
	 */
	virtual dp::String getPartitionType() = 0;
	/**
	 *  Get the folder where content files can potentially be found.
	 *
     *  \commands
     *  \commandrefbr{partitionGetRootURL}
	 */
	virtual dp::String getRootURL() = 0;
	/**
	 *  Get the folder where content files will be stored. It is "(My )Digital Editions" subfolder
	 *  of the user's OS-specific "(My )Documents" folder for desktop PC devices and "Digital Editions"
	 *  subfolder of a removable partition.
	 *
     *  \commands
     *  \commandrefbr{partitionGetDocumentFolderURL}
	 */
	virtual dp::String getDocumentFolderURL() = 0;
	/**
	 *  Get the folder where temporary files should be created. This is only used for the first
	 *  partition of the primary device for temporary content storage during fulfillment. It may
	 *  return null string for other partitions.
	 *
     *  \commands
     *  \commandrefbr{partitionGetTemporaryFolderURL}
	 */
	virtual dp::String getTemporaryFolderURL() = 0;
	/**
	 *  Open a stream to read the content of a file (possibly asynchronously).
	 */
	virtual Stream * readFile( const dp::String& fileURL, StreamClient * client, unsigned int caps ) = 0;
	/**
	 *  Creates an empty file with unique name, use dp::Unknown::getOptionalInterface with "URL" as
	 *  a parameter to get created file name.
	 */
	virtual void createUniqueFile( const dp::String& baseURL, const dp::String& suffix, dp::Callback * callback ) = 0;
	/**
	 *  Write the content of a stream into a file (possibly asynchronously) and release the stream.
	 */
	virtual void writeFile( const dp::String& fileURL, Stream * streamToWrite, dp::Callback * callback ) = 0;
	/**
	 * Remove a file (possibly asynchronously) and invoke the callback.
	 */
	virtual void removeFile( const dp::String& fileURL, dp::Callback * callback ) = 0;
	/**
	 *  Create a partition which is based on OS file I/O method.
	 */
	static Partition * createFileSystemPartition( dpdev::Device * owningDevice, int index,
				const dp::String& name, const dp::String& type,
				const dp::String& rootFolderURL, const dp::String& docFolderURL );
	/**
	 *  Set partition index for partition created by Partition::createFileSystemPartition
	 */
	static void setFileSystemPartitionIndex( Partition * partition, int index );
	/**
	 *  Release partition created by Partition::createFileSystemPartition
	 */
	static void releaseFileSystemPartition( Partition * partition );
	/**
	 *  Scan the devices and find a partition, so that partition root URL is a prefix
	 *  of the given file URL.
	 *
     *  \commands
     *  \commandrefbr{partitionFindForURL}
	 */
	static Partition * findPartitionForURL( const dp::String& fileURL );
};

}

#endif // DP_IO_H
