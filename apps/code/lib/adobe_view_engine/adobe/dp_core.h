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
 *  \file dp_core.h
 *
 *  Reader Mobile SDK public interface -- core objects
 */
#ifndef _DP_CORE_H
#define _DP_CORE_H

#include <stddef.h>
#include "dp_bind_iid.h"
#include "dp_types.h"

// pre-declare for UFT integration; UFT is not a part of the SDK public interface
namespace uft
{

class String;
class UTF16String;
class Buffer;
class Vector;

}

/**
 *  Reader Mobile SDK core objects
 */
namespace dp
{

class DataManager;
class Data;
class String;
class UTF16String;
class StringList;

/**
 *  Utility structure for wrapping custom byte arrays and string classes in dp::Data, dp::String
 *  and dp::UTF16String
 */
struct DataRec
{
	DataManager * m_manager; /**< data manager */
	void * m_handle; /**< data handle */
};

/**
 *  Interface for wrapping custom byte arrays and string classes in dp::Data, dp::String
 *  and dp::UTF16String. Each custom byte array or string should be represented by a handle.
 *  DataManager interface is called to copy (or add a reference count) and release a handle
 *  as well as to get byte array's data pointer and length. When implementing DataManager 
 *  for strings, note that length is always length in bytes (even for UTF16 strings) -e not
 *  including terminating null character which still -e must be provided.
 */
class DataManager
{
protected:

	virtual ~DataManager() {}

public:

	/**
	 *  Return a unique value identifying this class. Returning address of a static variable
	 *  is recommended. This is needed to identify a particular DataManager subclass even when
	 *  run-time type information is turned off in the compiler.
	 */
	virtual void * getTypeId() = 0;
	/**
	 *  Return data pointer and length of the byte array or string represented by the handle.
	 *  Length is in bytes, for strings it does not include terminating null character (which still
	 *  must be provided).
	 */
	virtual const void * dataPtr( void * handle, size_t * len ) = 0;
	/**
	 *  Clone dp::Data object. This is called in dp::Data assignment operator and copy
	 *  constructor. This can bump up a reference count for frameworks that support
	 *  reference counting or make a copy of the data. Note that DataManager can be
	 *  replaced in the cloned dp::Data object. 
	 */
	virtual void clone( const DataRec * from, DataRec * to ) = 0;
	/**
	 *  Release the handle. Called in dp::Data destructor and assignment operator.
	 */
	virtual void release( void * handle ) = 0;
};

/**
 *  A simple wrapper class representing unstructured data. Data wrapped by this
 *  class is assumed to be non-mutable (should not change during this object's
 *  lifetime).
 *
 *  This class is designed with multiple framework compatibility in mind. Various
 *  unstructured data objects can be wrapped in dp::Data by implementing a custom
 *  dp::DataManager.
 * 
 *  SDK provides built-in default implementation that can be used for simplicity.
 */
class Data : private DataRec
{
public:

	friend class String;
	friend class UTF16String;

	/**
	 *  Creates null dp::Data
	 */
	Data()
	{
		m_manager = 0;
		m_handle = 0;
	}

	/**
	 *  Creates dp::Data from the given data and length using default DataManager.
	 *  Data is copied into a new buffer.
	 */
	Data( const unsigned char * data, size_t len );

	/**
	 *  Create dp::Data using custom DataManger.
	 */
	Data( DataManager * manager, void * handle )
	{
		m_manager = manager;
		m_handle = handle;
	}

	/**
	 *  Copy constructor (delegated to DataManger).
	 */
	Data( const Data& other )
	{
		if( other.m_manager )
			other.m_manager->clone( &other, this );
		else
		{
			m_manager = 0;
			m_handle = 0;
		}
	}

	/**
	 *  Destructor (delegated to DataManger).
	 */
	~Data()
	{
		if( m_manager )
			m_manager->release(m_handle);
	}

	/**
	 *  Assignment (delegated to DataManger).
	 */
	const Data& operator=( const Data& other )
	{
		DataManager * manager = m_manager;
		void * handle = m_handle;
		if( other.m_manager )
			other.m_manager->clone( &other, this );
		else
		{
			m_manager = 0;
			m_handle = 0;
		}
		if( manager )
			manager->release(handle);
		return other;
	}

	/**
	 *  Obtain pointer to the actual data and (optionally) its length.
	 */
	const unsigned char * data(size_t * len = 0) const
	{
		if( m_manager )
			return static_cast<const unsigned char*>(m_manager->dataPtr(m_handle, len));
		else if( len )
			*len = 0;
		return 0;
	}

	/**
	 *  Get data length in bytes.
	 */
	size_t length() const
	{
		size_t len = 0;
		if( m_manager )
			m_manager->dataPtr(m_handle, &len);
		return len;
	}

	/**
	 *  Check if this object contains no data (null Data).
	 */
	bool isNull() const
	{
		return m_manager == 0;
	}

	/**
	 *  Convenience method to create dp::Data object from internal Reader Mobile SDK buffer object.
	 */
	Data( const uft::Buffer& buffer );
	
	/**
	 *  Convenience method to create internal Reader Mobile SDK buffer object from dp::Data.
	 */
	operator uft::Buffer() const;

};

/**
 *  Implementation struct for TransientData; do not use directly.
 */
struct TransientDataRec
{
	/**
	 * pointer to the buffer that this structure wraps
	 */
	const unsigned char * m_buf;
	/**
	 *  length of the buffer that this structure wraps
	 */
	size_t m_length;
};

/**
 *  dp::Data that wraps buffer without copying it. It uses a special dp::DataManager to
 *  copy wrapped buffer only when dp::Data copy constructor. This is mostly useful to pass
 *  parameters to the methods that take parameters of "const dp::Data&" type efficiently.
 */
class TransientData : public Data
{
public:

	/**
	 *  Create TransientData from the buffer pointer and data length. Buffer must remain valid
	 *  and do not changed while TransientData object exists.
	 */
	TransientData( const unsigned char * buf, size_t len );

private:

	TransientDataRec m_rec;
};

/**
 *  A simple wrapper class representing UTF-8 string data. String wrapped by this
 *  class is assumed to be non-mutable (should not change during this object's
 *  lifetime).
 *
 *  This class is designed with multiple framework compatibility in mind. Various
 *  unstructured data objects can be wrapped in dp::String by implementing a custom
 *  dp::DataManager.
 * 
 *  SDK provides built-in default implementation that can be used for simplicity.
 */
class String : public Data
{
public:

	/**
	 *  Create null String
	 */
	String()
	{
	}

	/**
	 *  Create String from null-terminated UTF-8 character array using default DataManager.
	 *  String is copied to a new memory location.
	 */
	String( const char * utf8 );

	/**
	 *  Create String from UTF-8 character array of given length using default DataManager.
	 *  String is copied to a new memory location.
	 */
	String( const char * utf8, size_t len );

	/**
	 *  Create String from null-terminated UTF-16 character array using default DataManager.
	 *  String is converted to UTF-8 and copied to a new memory location.
	 */
	String( const dp::utf16char * utf16 );

	/**
	 *  Create String from UTF-16 character array of given length using default DataManager.
	 *  String is converted to UTF-8 and copied to a new memory location.
	 */
	String( const dp::utf16char * utf16, size_t len );

	/**
	 *  Create dp::String using custom DataManger.
	 */
	String( DataManager * manager, void * handle )
		: Data(manager, handle)
	{
	}

	/**
	 *  Create dp::String from another.
	 */
	String( const String& other )
		: Data(other)
	{
	}

	/**
	 *  dp::String assignment operator.
	 */
	const String& operator=( const String& other )
	{
		Data::operator=(other);
		return other;
	}

	/**
	 *  Get pointer to null-terminated UTF-8 character data.
	 */
	const char * utf8() const
	{
		return reinterpret_cast<const char*>(data());
	}

	/**
	 *  Convenience method to create dp::String object from internal Reader Mobile SDK string object.
	 */
	String( const uft::String& buffer );
	
	/**
	 *  Convenience method to create internal Reader Mobile SDK string object from dp::String.
	 */
	operator const uft::String() const;

	/**
	 *  Convenience method to create internal Reader Mobile SDK string object from dp::String.
	 */
	const uft::String uft() const;

	/**
	 *  Utility method to URL-decode a string.
	 */
	static dp::String urlDecode( const dp::String& str );
	/**
	 *  Utility method to URL-encode a string.
	 */
	static dp::String urlEncode( const dp::String& str );
	/**
	 *  Utility method to Base64-decode a string.
	 */
	static dp::Data base64Decode( const dp::String& src );
	/**
	 *  Utility method to Base64-encode binary data.
	 */
	static dp::String base64Encode( const dp::Data& data );

	/**
	 *  Convert time (milliseconds since 1970) to W3CDTF date/time string
	 */
	static dp::String timeToString( const dp::time_t time );
	/**
	 *  Parse W3CDTF date/time string into time (milliseconds since 1970)
	 */
	static dp::time_t stringToTime( const dp::String& str );
};

/**
 *  A simple wrapper class representing UTF-16 string data. String wrapped by this
 *  class is assumed to be non-mutable (should not change during this object's
 *  lifetime).
 *
 *  This class is designed with multiple framework compatibility in mind. Various
 *  unstructured data objects can be wrapped in dp::UTF16String by implementing a custom
 *  dp::DataManager.
 * 
 *  SDK provides built-in default implementation that can be used for simplicity.
 */
class UTF16String : public Data
{
public:

	/**
	 *  Creates null UTF16String
	 */
	UTF16String()
	{
	}

	/**
	 *  Creates UTF16String from UTF-8 String
	 */
	UTF16String( const String& str );

	/**
	 *  Creates UTF16String from UTF-8 null terminated character array
	 */
	UTF16String( const char * utf8 );

	/**
	 *  Creates UTF16String from UTF-16 null terminated character array
	 */
	UTF16String( const dp::utf16char * utf16 );

	/**
	 *  Creates UTF16String using custom data manager
	 */
	UTF16String( DataManager * manager, void * handle )
		: Data(manager, handle)
	{
	}

	/**
	 *  Creates UTF16String from from another UTF16String
	 */
	UTF16String( const UTF16String& other )
		: Data(other)
	{
	}

	/**
	 *  Assignment operator
	 */
	const UTF16String& operator=( const UTF16String& other )
	{
		Data::operator=(other);
		return other;
	}

	/**
	 *  Get null-terminated UTF-16 character array pointer for this string
	 */
	const dp::utf16char * utf16() const
	{
		return reinterpret_cast<const dp::utf16char*>(data());
	}

	/**
	 *  Get this string UTF-16 character count. Note that this method hides dp::Data::length.
	 *  This method returns \e character count, whereas dp::Data::length returns \e byte count
	 */
	size_t length() const
	{
		// in UTF16 characters
		return Data::length()/2;
	}

	/**
	 *  Convenience method to create dp::UTF16String from internal Reader Mobile SDK string object.
	 */
	UTF16String( const uft::UTF16String& buffer );
	
	/**
	 *  Convenience method to create internal Reader Mobile SDK string object from dp::UTF16String.
	 */
	operator uft::UTF16String();

};

/**
 *  Generic interface to query for interface id and also for additional interfaces by name
 */
class Unknown
{
protected:
	virtual ~Unknown() {}

public:

	/**
	 *  Return most-derived interface ID which this object implements
	 */
	virtual int getInterfaceID() { return IID_Unknown; }
	/**
	 *  Query an optional interface that may be implemented by this object.
	 *
	 *  If the interface name is recognized, the pointer to that interface is returned. Otherwise
	 *  retuns NULL. Unless otherwise noted, the lifetime of the returned interface is the same as
	 *  the lifetime of this dp::Unknown object.
	 */
	virtual void * getOptionalInterface( const char * name ) { return 0; }
};

/**
 *  Generic callback interface. Exact semantics of the call depends on the context.
 */
class Callback : public Unknown
{
protected:
	virtual ~Callback() {}

public:

	virtual int getInterfaceID() { return IID_Callback; }

	/**
	 *  Progress notification. 
	 */
	virtual void reportProgress( double progress ) {}

	/**
	 *  Error notification. See \ref errors
	 */
	virtual void reportError( const dp::String& error ) = 0;
	/**
	 *  Invoke the generic callback.
	 */
	virtual void invoke( Unknown * param ) = 0;
};

/**
 * A base class for objects with lifetime which is controlled by a release call
 */
class Releasable : public Unknown
{
public:

	virtual int getInterfaceID() { return IID_Releasable; }
	/**
	 *  Release the object
	 */
	virtual void release() = 0;
};

/**
 *  A base class for objects with lifetime determined by reference counting.
 */
class RefCounted : public Unknown
{
protected:
	virtual ~RefCounted() {}

public:

	virtual int getInterfaceID() { return IID_RefCounted; }
	/**
	 *  Increment reference count
	 */
	virtual void addRef() = 0;
	/**
	 *  Decrement reference count releasing the object if no more references are present.
	 */
	virtual void release() = 0;
};

/**
 *  A smart pointer object to handle reference counting for dp::RefCounted subclasses.
 */
template<class RC> class ref
{
private:
	RC * m_ptr;

public:

	/**
	 *  Default constructor - creates null pointer.
	 */
	ref() : m_ptr(0) {}
	/**
	 *  Create a smart pointer from the raw pointer, incrementing reference count.
	 */
	ref( RC * ptr ) : m_ptr(ptr) { if(ptr) ptr->addRef(); }
	/**
	 *  Create a smart pointer from another smart pointer, incrementing reference count.
	 */
	ref( const ref<RC>& other ) : m_ptr(other.m_ptr) { if(m_ptr) m_ptr->addRef(); }
	/**
	 *  Destructor - decremented reference count.
	 */
	~ref() { if(m_ptr) m_ptr->release(); }
	/**
	 *  Obtain raw pointer from smart pointer. Reference count is not touched.
	 */
	operator RC*() const { return m_ptr; }
	/**
	 *  Return false for null pointer, true otherwise.
	 */
	operator bool() const { return m_ptr != 0; }
	/**
	 *  Obtain raw pointer from smart pointer for member access. Reference count is not touched.
	 */
	RC* operator->() const { return m_ptr; }
	/**
	 *  Assignment operator. Reference count of the assigned pointer is incremented. 
	 *  Then, reference count of this object is decremented.
	 */
	const ref<RC>& operator=( const ref<RC>& other ) 
	{
		if( other.m_ptr )
			other.m_ptr->addRef();
		if( m_ptr )
			m_ptr->release();
		m_ptr = other.m_ptr;
		return *this;
	}
};

/**
 *  A read-only list of ref-counted objects.
 */
template<class RC> class List : public RefCounted
{
protected:
	virtual ~List() {}
public:
	virtual int getInterfaceID() { return IID_List; }
	/**
	 *  Returns number of items in the list.
	 */
	virtual size_t length() = 0;
	/**
	 *  Returns k-th element of the list.
	 */
	virtual ref<RC> operator[] ( size_t k ) = 0;
	/**
	 *  operator[] alias for bindings
	 */
	ref<RC> item( size_t k ) { return (*this)[k]; }
};

/**
 *  A ref-counted read-only list of ref-counted objects. Essentialy
 *  this is a smart pointer for dp::List object.
 */
template<class RC> class list
{
private:
	List<RC> * m_ptr;

public:
	/**
	 *  Create null list (not the same as empty!)
	 */
	list() : m_ptr(0) {}
	/**
	 *  Wrap List object into list (increments refcount)
	 */
	list( List<RC> * ptr ) : m_ptr(ptr) { if(ptr) ptr->addRef(); }
	/**
	 *  Create list from another list (increments refcount)
	 */
	list( const list<RC>& other ) : m_ptr(other.m_ptr) { if(m_ptr) m_ptr->addRef(); }
	/**
	 *  Destroy list (decrements refcount)
	 */
	~list() { if(m_ptr) m_ptr->release(); }
	/**
	 *  list length (calls List::length)
	 */
	size_t length() const { return m_ptr->length(); }
	/**
	 *  list element access (calls List::operator[])
	 */
	ref<RC> operator[] ( size_t index ) const { return (*m_ptr)[index]; }
	/**
	 *  list element access (calls List::operator[])
	 */
	ref<RC> operator[] ( int index ) const { return (*m_ptr)[index]; }
	/**
	 *  Check if the list is null (does not check for emptyness)
	 */
	operator bool() const { return m_ptr != 0; }
	/**
	 *  Assignment operator. Increments assigned list refcount and decrements this list refcount.
	 */
	const list<RC>& operator=( const list<RC>& other ) 
	{
		if( other.m_ptr )
			other.m_ptr->addRef();
		if( m_ptr )
			m_ptr->release();
		m_ptr = other.m_ptr;
		return *this;
	}
	/**
	 * special cast which is useful for language bindings
	 */
	operator List<RefCounted> *() const { return (List<RefCounted>*)m_ptr; }
};

template<class T> class PointerVector;

/**
 *  Utility vector of pointers class; not intended for direct use; use PointerVector instead.
 */
class RawPointerVector
{
	template<class T> friend class dp::PointerVector;

private:

	RawPointerVector() : m_list(0), m_length(0), m_size(0) {}
	~RawPointerVector();

	void * operator[]( size_t index ) const { return m_list[index]; }
	size_t length() const { return m_length; }
	void append( void * ptr ) { insert(m_length, ptr); }
	void insert( size_t index, void * ptr );
	bool remove( void * ptr );
	void remove( size_t index );

	// not implemented
	RawPointerVector( const RawPointerVector& );
	const RawPointerVector& operator=( const RawPointerVector& );

private:
	void * *	m_list;
	size_t		m_length;
	size_t		m_size;
};

/**
 *  Utility class implementing a vector of pointers to class T. This class manages
 *  memory to store the array of pointers, but not objects where these pointers
 *  point. In other words, its destructor won't destroy objects stored in this
 *  vector.
 */
template<class T> class PointerVector
{
public:

	/**
	 *  Creates empty pointer vector.
	 */
	PointerVector() {}
	/**
	 *  Destroy vector. Objects in the vector are not destroyed.
	 */
	~PointerVector() {}
	/**
	 *  Get vector's element by index
	 */
	T * operator[]( size_t index ) const { return static_cast<T*>(m_impl[index]); }
	/**
	 *  Get vector's element count
	 */
	size_t length() const { return m_impl.length(); }
	/**
	 *  Add a new pointer at the end
	 */
	void append( T * ptr ) { m_impl.append(ptr); }
	/**
	 *  Add a new pointer at the specified position
	 */
	void insert( size_t index, T * ptr ) { m_impl.insert(index, ptr); }
	/**
	 *  Find a pointer and remove it from the vector.
	 *
	 *  \return true if element is fond and removed, false if it is not found
	 */
	bool remove( T * ptr ) { return m_impl.remove(ptr); }
	/**
	 *  Remove element at the specified index
	 */
	void remove( size_t index ) { m_impl.remove(index); }

private:

	// not implemented
	PointerVector( const PointerVector<T>& );
	const PointerVector<T>& operator=( const PointerVector<T>& );

private:

	RawPointerVector m_impl;
};

/**
 *  Utility object that represents a list of strings.
 */
class StringList : public RefCounted
{
protected:
	virtual ~StringList() {}

public:
	virtual int getInterfaceID() { return IID_StringList; }
	/**
	 *  String count in this list
     *  \commands
     *  \commandrefbr{getErrorListLength}
	 */
	virtual size_t length() = 0;
	/**
	 *  Get item in the list by its index
     *  \commands
     *  \commandrefbr{getErrorString}
	 */
	virtual String item(size_t index) = 0;

	/**
	 *  A convenience function to wrap uft::Vector into a dp::StringList
	 */
	static ref<StringList> makeStringListFromVector( const uft::Vector& stringVector );
};

/**
 * Abstract interface to an error list. An error list contains all state errors for some object (e.g. Document).
 */
class ErrorList : public StringList
{
protected:	
	/**
	  * Virtual destructor
	  */
	virtual ~ErrorList() {}
public:
	virtual int getInterfaceID() { return IID_ErrorList; }
	/**
	 * Allows the host to determine if any fatal errors are on the error list.
     */
	virtual bool hasFatalErrors() = 0;
	/**
	 * Allows the host to determine if any errors other than fatal errors are on the error list.
     */
	virtual bool hasErrors() = 0;
	/**
	 * Allows the host to determine if any warnings are on the error list.
     */
	virtual bool hasWarnings() = 0;
	/**
	 * Removes all errors from the list.
	 */
	virtual void clear() = 0;
};

/**
 *  Set the version information for a particular subsystem. It can be queried with embed::getVersionInfo later.
 *
 *  All applications must define the following version strings:
 *  - "product" - Full name of the product
 *  - "jpeg" - JPEG library version
 *
 *  Applications that make use of embed::DRMProcessor also must define the following version strings:
 *  - "clientOS" - Operating System name, e.g. "Windows XP"
 *  - "clientVersion" - Application name, version and build number 
 *  - "clientLocale" - two-letter lowercase language code
 * 
 *  SDK defines these version strings internally
 *  - "png" - PNG library version
 *  - "zlib" - ZLib library version
 *  - "hobbes" - SDK version
 *  - "hobbes.major" - SDK major version
 *  - "hobbes.minor" - SDK minor version
 *  - "hobbes.build" - SDK build number
 */
void setVersionInfo( const dp::String& name, const dp::String& version );

/**
 *  Query the version information for a particular subsystem that was set by embed::setVersionInfo.
 */
dp::String getVersionInfo( const dp::String& name );

/**
 *  Helper class to set the version information in static initializer. Use something like
 *
 *  static dp::VersionInfo version( "my_component", "my_version" );
 *
 *  See also dp::setVersionInfo
 */
class VersionInfo
{
public:

	/**
	 *  Set version info in static initializer.
	 */
	VersionInfo( const char * name, const char * version ) { dp::setVersionInfo( name, version ); }
};

/**
 *  Support for managing the lifetime of wrapper objects for scripting languages (which
 *  are understood here as programming languages without explicit memory management)
 */
class BindingManager
{
protected:
	virtual ~BindingManager() {}

public:

	/**
	 *  Notify peer wrapper object(s) - if any - that this native object is going to be destroyed.
	 */
	virtual void invalidateScriptWrapperFor( Unknown * nativeObj ) = 0;
	/**
	 *  Increment reference count for a native wrapper, preventing corresponding peer script object
	 *  from being destroyed. If parameter is not a wrapper, does not do anything.
	 */
	virtual void addRefNativeWrapper( Unknown * nativeWrapper ) = 0;
	/**
	 *  Decrement reference count for a native wrapper, potentially destroying corresponding peer
	 *  script object. If parameter is not a wrapper, does not do anything.
	 */
	virtual void releaseNativeWrapper( Unknown * nativeWrapper ) = 0;
	/**
	 *  Determine if the call was made from the privileged code
	 */
	virtual bool isPrivileged() = 0;
	/**
	 *  Set lifetime manager. Only one is allowed at a time.
	 */
	static void setBindingManager( BindingManager * manager );
	/**
	 *  Get current Binding manager. Default one is non-NULL, but it does not do anything.
	 */
	static BindingManager * getBindingManager();
};

}

#endif // _DP_CORE_H
