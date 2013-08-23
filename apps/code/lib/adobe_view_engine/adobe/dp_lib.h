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
 *  \file dp_lib.h
 *
 *  Reader Mobile SDK public interface -- managing information about documents
 */
#ifndef _DP_LIB_H
#define _DP_LIB_H

#include "dp_core.h"

namespace dpio
{
class Partition;
}

/**
 *  Reader Mobile SDK document and metadata management
 */
namespace dplib
{

class Library;
class LibraryListener;
class ContentRecord;
class ContentTag;

/**
 *  An object that represents information about a single document file in a Library.
 */
class ContentRecord : public dp::RefCounted
{
protected:
	virtual ~ContentRecord() {}

public:

	virtual int getInterfaceID() { return IID_ContentRecord; }
	/**
	 *  Get the Library which holds this ContentRecord.
     *  \commands
     *  \commandrefbr{contentRecordGetLibrary}
	 */
	virtual Library * getLibrary() = 0;
	/**
	 *  Get document metadata cached in this ContentRecord.
	 *  See dpdoc::Document::getMetadata for metadata names.
     *  \commands
     *  \commandrefbr{contentRecordGetMetadata}
	 */
	virtual dp::String getMetadata( const dp::String& name ) = 0;
	/**
	 *  Set metadata cached in this ContentRecord. Note that this does not change metadata in
	 *  the document itself. See dpdoc::Document::getMetadata for metadata names.
     *  \commands
     *  \commandrefbr{contentRecordSetMetadata}
	 */
	virtual void setMetadata( const dp::String& name, const dp::String& value ) = 0;
	/**
	 *  Return creation time of this ContentRecord.
     *  \commands
     *  \commandrefbr{contentRecordGetCreationTime}
	 */
	virtual dp::time_t getCreationTime() = 0;
	/**
	 *  Return time when the document when it was last read 
     *  \commands
     *  \commandrefbr{contentRecordGetLastReadTime}
	 */
	virtual dp::time_t getLastReadTime() = 0;
	/**
	 *  Return bookmark of the location in the document when it was last read 
     *  \commands
     *  \commandrefbr{contentRecordGetLastReadBookmark}
	 */
	virtual dp::String getLastReadBookmark() = 0;
	/**
	 *  Set the bookmark of the position in the document when it was last read. This
	 *  also sets last read time to the current time.
     *  \commands
     *  \commandrefbr{contentRecordSetLastReadBookmark}
	 */
	virtual void setLastReadBookmark( const dp::String& bookmark ) = 0;
	/**
	 *  Set application-specific private data assigned by ContentRecord::setPrivateData.
     *  \commands
     *  \commandrefbr{contentRecordSetPrivateData}
	 */
	virtual dp::String getPrivateData( const dp::String& ns, const dp::String& name ) = 0;
	/**
	 *  Set application-specific private data for this ContentRecord. Each application
	 *  should define its own private XML namespace to avoid conflicts. For instance, this
	 *  may be last used font size or zoom level for a book. 
     *  \commands
     *  \commandrefbr{contentRecordGetPrivateData}
	 */
	virtual void setPrivateData( const dp::String& ns, const dp::String& name, const dp::String& data ) = 0;
	/**
	 *  Get URL of the document file. URL is expected to point to the partition which is managed
	 *  by the Library which holds this ContentRecord.
     *  \commands
     *  \commandrefbr{contentRecordGetContentURL}
	 */
	virtual dp::String getContentURL() = 0;
	/**
	 *  Get URL of the thumbnail file. URL is expected to point to the partition which is managed
	 *  by the Library which holds this ContentRecord. This method may return NULL right after
	 *  a ContentRecord is created. By the time dplib::LibraryListener::contentRecordAdded is called
	 *  it is guaranteed to be resolved.
	 *
	 *  Thumbnail should be an RGB color PNG file with width 90 and height 130. If the document has inherent aspect ratio
	 *  (e.g. PDF) which is different from 9:13, then one of the dimensions should be reduced. The recommended way to
	 *  create thumbnails is the following:
	 *  - create dpdoc::Renderer for the document
	 *  - set viewport size to provisional surface size Ws=360 Hs=520 (4 times larger than 90x130)
	 *  - query document natural size using dpdoc::Renderer::getNaturalSize (Wn,Hn)
	 *  - calculate thumbnail scale = min(Ws/Wn,Hs/Hn)
	 *  - adjust target surface size to Ws=scale*Wn, Hs=scale*Hn, rounding up to the nearest multiple of 4
	 *  - create bitmap-based dpdoc::Surface with size (Ws,Hs)
	 *  - set viewport size to Ws,Hs
	 *  - set navigation matrix to (scale 0 0 scale 0 0)
	 *  - render by calling dpdoc::Renderer::paint with the surface as a parameter
	 *  - calculate thumbnail size Wt=Ws/4, Ht=Hs/4
	 *  - scale down surface bitmap to the thumbnail size by averaging over each 4x4 pixel block (not nearest neighbor!)
	 *  - save thumbnail bitmap as PNG file using given thumbnail URL 
	 *  - call ContentRecord::thumbnailChangeNotify() to notify listeners (including other applications)
	 *  .
	 *
     *  \commands
     *  \commandrefbr{contentRecordGetThumbnailURL}
	 */
	virtual dp::String getThumbnailURL() = 0;
	/**
	 *  Notify dplib::LibraryListener objects that this ContentRecord thumbnail was changed.
	 *  This should be called every time thumbnail file is written or deleted.
     *  \commands
     *  \commandrefbr{contentRecordThumbnailChangeNotify}
	 */
	virtual void thumbnailChangeNotify() = 0;
	/**
	 *  Get URL of the annotation file. URL is expected to point to the partition which is managed
	 *  by the Library which holds this ContentRecord. This method may return NULL right after
	 *  a ContentRecord is created. By the time dplib::LibraryListener::contentRecordAdded is called
	 *  it is guaranteed to be resolved.
	 *
	 *  Annotations for documents are written by multiple applications. To ensure interoperability
	 *  annotations must be written using specific XML format: \ref annotations
	 *
     *  \commands
     *  \commandrefbr{contentRecordGetAnnotationURL}
	 */
	virtual dp::String getAnnotationURL() = 0;
	/**
	 *  Notify dplib::LibraryListener objects that this ContentRecord annotation was changed
	 *  This should be called every time annotation file is written or deleted.
     *  \commands
     *  \commandrefbr{contentRecordAnnotationChangeNotify}
	 */
	virtual void annotationChangeNotify() = 0;
	/**
	 *  Check if this ContentRecord was tagged by a specified tag or any of its descentants.
     *  \commands
     *  \commandrefbr{contentRecordIsTaggedBy}
	 */
	virtual bool isTaggedBy( const dp::ref<ContentTag>& tag ) = 0;
	/**
	 *  Tag this ContentRecord with the specified tag.
     *  \commands
     *  \commandrefbr{contentRecordAddTag}
	 */
	virtual void addTag( const dp::ref<ContentTag>& tag ) = 0;
	/**
	 *  Remove a given \a tag and all of its descentants from this ContentRecord. Passing NULL \a tag
	 *  removes all tags.
     *  \commands
     *  \commandrefbr{contentRecordRemoveTag}
	 */
	virtual void removeTag( const dp::ref<ContentTag>& tag ) = 0;
	/**
	 *  Get a list of tags tagging this ContentRecord. Note that ancestor tags that tag this record
	 *  implicitly will not be listed (unless explicitly added).
     *  \commands
     *  \commandrefbr{contentRecordGetTags}
	 */
	virtual dp::list<ContentTag> getTags() = 0;
};

/**
 *  An object that represents information about a tag (can be thought of as "a bookshelf")
 */
class ContentTag : public dp::RefCounted
{
protected:
	virtual ~ContentTag() {}

public:

	virtual int getInterfaceID() { return IID_ContentTag; }
	/**
	 *  Get the Library which holds this tag.
     *  \commands
     *  \commandrefbr{contentTagGetLibrary}
	 */
	virtual Library * getLibrary() = 0;
	/**
	 *  Get the metadata describing this tag
     *  \commands
     *  \commandrefbr{contentTagGetMetadata}
	 */
	virtual dp::String getMetadata( const dp::String& name ) = 0;
	/**
	 *  Set the metadata describing this tag
     *  \commands
     *  \commandrefbr{contentTagSetMetadata}
	 */
	virtual void setMetadata( const dp::String& name, const dp::String& value ) = 0;
	/**
	 *  A string that identifies the tag. Tags on different partitions are considered equivalent
	 *  if their tag ids are the same. User-defined tags should start with "private::". Tags that
	 *  start with adept:: are reserved. Meaning of these tags is predefined:
	 *  - adept::purchased
	 *  - adept::borrowed
	 *  .
	 *
	 *  A tag ID of a tag with a parent contains parent tag, two colons and child identifier,
	 *  e.g. tag ID private::textbooks::physics implies a parent tag with ID private::textbooks
     *  \commands
     *  \commandrefbr{contentTagGetTagID}
	 */
	virtual dp::String getTagID() = 0;
	/**
	 *  Parent tag of this tag, if any.
     *  \commands
     *  \commandrefbr{contentTagGetParent}
	 */
	virtual dp::ref<ContentTag> getParent() = 0;
	/**
	 *  Set application-specific private data assigned by ContentTag::setPrivateData.
     *  \commands
     *  \commandrefbr{contentTagSetPrivateData}
	 */
	virtual dp::String getPrivateData( const dp::String& ns, const dp::String& name ) = 0;
	/**
	 *  Set application-specific private data for this ContentTag. Each application
	 *  should define its own private XML namespace to avoid conflicts.
     *  \commands
     *  \commandrefbr{contentTagSetPrivateData}
	 */
	virtual void setPrivateData( const dp::String& ns, const dp::String& name, const dp::String& data ) = 0;
};

/**
 *  An interface to get notifications about a dplib::Library objects.
 *  \commands
 *  \commandrefbr{libraryAddListener}
 */
class LibraryListener : public dp::Unknown
{
protected:
	virtual ~LibraryListener() {}

public:

	virtual int getInterfaceID() { return IID_LibraryListener; }
	/**
	 *  Called when the library is fully loaded (i.e. all content records are read from the
	 *  partition)
	 */
	virtual void libraryLoaded( Library * library ) = 0;
	/**
	 *  Notification that a ContentRecord was added to the library. This can happen during
	 *  initialization (content records loading asynchronously), or when a new content record
	 *  is created by this or another RM-SDK-based application.
	 */
	virtual void contentRecordAdded( Library * library, const dp::ref<ContentRecord>& record ) = 0;
	/**
	 *  Called when a content record is removed.
	 */
	virtual void contentRecordRemoved( Library * library, const dp::ref<ContentRecord>& record ) = 0;
	/**
	 *  Called when a content record is changed.
	 */
	virtual void contentRecordChanged( Library * library, const dp::ref<ContentRecord>& record ) = 0;
	/**
	 *  Called when an annotation is created or changed.
	 */
	virtual void annotationChanged( Library * library, const dp::ref<ContentRecord>& record ) = 0;
	/**
	 *  Called when the thumbnail is created or changed.
	 */
	virtual void thumbnailChanged( Library * library, const dp::ref<ContentRecord>& record ) = 0;
	/**
	 *  Called when a tag is added to the library
	 */
	virtual void contentTagAdded( Library * library, const dp::ref<ContentTag>& tag ) = 0;
	/**
	 *  Called when a tag is removed the library
	 */
	virtual void contentTagRemoved( Library * library, const dp::ref<ContentTag>& tag ) = 0;
	/**
	 *  Called when a tag is changed
	 */
	virtual void contentTagChanged( Library * library, const dp::ref<ContentTag>& tag ) = 0;
	/**
	 *  Called before library is destroyed (because underlying partition was destroyed).
	 */
	virtual void libraryPartitionRemoved( Library * library ) = 0;

};

/**
 *  Collection of content records and tags stored on a particular device partition.
 */
class Library : public dp::Unknown
{
public:

	virtual int getInterfaceID() { return IID_Library; }
	/**
	 *  Returns partition managed by this Library
     *  \commands
     *  \commandrefbr{libraryGetPartition}
	 */
	virtual dpio::Partition * getPartition() = 0;
	/**
	 *  False indicates that existing content records are not yet fully loaded (and are
	 *  being loaded asynchronously).
     *  \commands
     *  \commandrefbr{libraryIsLoaded}
	 */
	virtual bool isLoaded() = 0;
	/**
	 *  Create content record for a given content url
     *  \commands
     *  \commandrefbr{libraryCreateContentRecord}
	 */
	virtual dp::ref<ContentRecord> createContentRecord( const dp::String& contentURL ) = 0;
	/**
	 *  Clone an existing content record changing the content URL to the url given
	 *  by a parameter. This is useful for copying and moving content files.
     *  \commands
     *  \commandrefbr{libraryCloneContentRecord}
	 */
	virtual dp::ref<ContentRecord> cloneContentRecord( const dp::ref<ContentRecord>& existingCR, const dp::String& newContentURL ) = 0;
	/**
	 *  Returns the content record for the content with the given URL, if that content is in the
	 *  library and library is fully loaded. If the library is only partially loaded it may return
	 *  NULL.
     *  \commands
     *  \commandrefbr{libraryGetContentRecordByURL}
	 */
	virtual dp::ref<ContentRecord> getContentRecordByURL( const dp::String& url ) = 0;
	/**
	 *  Delete content record from the library
     *  \commands
     *  \commandrefbr{libraryRemoveContentRecord}
	 */
	virtual void removeContentRecord( const dp::ref<ContentRecord>& contentRecord ) = 0;
	/**
	 *  Returns the list of all top-level tags in the Library (tags without a parent) if parent is null.
	 *  Returns the list of all tags in the Library which have a given tag as a parent if parent is not null.
     *  \commands
     *  \commandrefbr{libraryGetTags}
	 */
	virtual dp::list<ContentTag> getTags( const dp::ref<ContentTag>& parent ) = 0;
	/**
	 *  Returns the list of all content records in the Library is \a tag is null.
	 *  Returns the list of content records tagged with a given tag or any of its descentants
	 *  if \a tag is not null.
     *  \commands
     *  \commandrefbr{libraryGetContentRecords}
	 */
	virtual dp::list<ContentRecord> getContentRecords( const dp::ref<ContentTag>& tag ) = 0;
	/**
	 *  Creates a tag with the given ID and a given parent or returns an existing one if it is already there
     *  \commands
     *  \commandrefbr{libraryGetTagByID}
	 */
	virtual dp::ref<ContentTag> getTagByID( const dp::String& tag ) = 0;
	/**
	 *  Removes the tag and all its descentant tags from the library and from all
	 *  of the books to which it is attached
     *  \commands
     *  \commandrefbr{libraryRemoveTag}
	 */
	virtual void removeTag( const dp::ref<ContentTag>& tag ) = 0;
	/**
	 *  Add a library listener
     *  \commands
     *  \commandrefbr{libraryAddListener}
	 */
	virtual void addListener( LibraryListener * listener ) = 0;
	/**
	 *  Remove previously-added listener
	 */
	virtual void removeListener( LibraryListener * listener ) = 0;
	/**
	 *  Instantiates (if needed) and returns dplib::Library for a given partition. Only a single
	 *  dplib::Library object per partition can exist.
     *  \commands
     *  \commandrefbr{createLibrary}
	 */
	static Library * getPartitionLibrary( dpio::Partition * partition ); 
};

}

#endif // DP_LIB_H
