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
 *  \file dp_doc.h
 *  Reader Mobile SDK public interface -- document parsing and rendering
 */
#ifndef _DP_DOC_H
#define _DP_DOC_H

// Adobe patent application tracking #P705, entitled "Mechanism to interpret multi layer file type embedding", inventors: Peter Sorotokin, Richard Wright
// AdobePatentID="P705"

#include "dp_types.h"
#include "dp_core.h"
#include "dp_io.h"
#include "dp_drm.h"

/**
 *  Reader Mobile SDK document parsing and rendering
 */
namespace dpdoc
{

class Document;
class Renderer;
class TOCItem;
class Location;
class RangeInfo;
class RendererClient;
class DocumentClient;
class Surface;
class Event;
class MouseEvent;
class TextEvent;
class KeyboardEvent;
class ContentIterator;
class DisplayHandler;

/**
 *  Structure that holds a standard 2D transformation matrix. Coordinate transformation can be
 *  described by these formulas:
 *
 *  <pre><i>
 *      x' = a x + c y + e
 *      y' = b x + d y + f
 *  </i></pre>
 *
 *  See \ref systems.
 */
struct Matrix
{
	double a;	/**< a component */
	double b;	/**< b component */
	double c;	/**< c component */
	double d;	/**< d component */
	double e;	/**< e component */
	double f;	/**< f component */

	/**
	 *  Default constructor creates an identity matrix.
	 */
	Matrix() : a(1), b(0), c(0), d(1), e(0), f(0) {}

	/**
	 *  Constructor to initialize matrix members to the desired properties.
	 */
	Matrix( double av, double bv, double cv, double dv, double ev, double fv )
		: a(av), b(bv), c(cv), d(dv), e(ev), f(fv) {}
};

/**
 * \brief Paging mode of the content.
 */
enum PagingMode
{
	PM_HARD_PAGES,		/**< single-page-based view that only shows a single page at a time (supported for PDF and EPUB) */
	PM_HARD_PAGES_2UP,	/**< double-page-based view that shows 2 pages at a time (not supported) */
	PM_FLOW_PAGES,		/**< a paginated view, where a screen takes up the whole viewport and the content is reflowed (only supported for PDF) */ 
	PM_SCROLL_PAGES,	/**< scrollable page-based view showing a sequence of pages (only supported for PDF) */
	PM_SCROLL			/**< HTML-browser-like view that can be scrolled and does not have pages (not supported) */ 
};

/**
 *  \brief Functionality that a Renderer provides. These are bit flags that ORed together.
 */
enum RendererCapabilities
{
	RC_SCALE = 1,	/**< Renderer can scale the content; matrices of the form (sx 0 0 sy 0 0) are supported. */ 
	RC_MATRIX = 2,	/**< Renderer can arbitratily transform its content; arbitrary matrices are supported. */ 
	RC_CLIP = 4,	/**< Renderer can clip content to the viewport. */
	RC_BLEND = 8	/**< Renderer can blend with the background; see \ref rendering. */
};

/**
 *  Search flags for use in Renderer::findNext() method.
 */
enum SearchFlags
{
	SF_MATCH_CASE		= 1,	/**< Match the case of characters in the string being searched. */
	SF_BACK				= 2,	/**< Search toward the beginning of the document. */
	SF_WHOLE_WORD		= 4,	/**< Match whole word only. */
	SF_WRAP				= 8,	/**< Wrap search: start from the beginning when the end is reached. */
	SF_IGNORE_ACCENTS	= 0x10	/**< Ignore all accents on latin characters. SF_MATCH_CASE and SF_IGNORE_ACCENTS may be used in any combination. */
};

/**
 *  Flags for hit testing (Renderer::hitTest() method).
 */
enum HitTestFlags
{
	HF_EVENT		= 1,	/**< Event-style hit testing - element which event handler should be invoked */
	HF_SELECT		= 2,	/**< Selection-style hit testing - which location in the document to use for selection */
	HF_FORCE		= 4		/**< Find some meaningful location even if nothing is hit (e.g. by looking for a closest element), this value can be ORed with others */
};

/**
 *  Possible highlight types. Used by Renderer::addHighlight(), Renderer::naviagteToHighlight(), Renderer::getHighlight(), and Renderer::removeHighlight.
 */
enum HighlightType
{
	HT_NONE = 0,	/**< no highlight; only legal as a value of the MouseLocationInfo::highlightType structure */
	HT_SELECTION,	/**< a highlight that represents a selection; can change as a result of mouse events */
	HT_ACTIVE,		/**< a highlight that represents a focused object; can change as a result of focus events */
	HT_ANNOTATION,	/**< a highlight that represents a bookmark or annotation */
	HT_NUM_HIGHLIGHT_TYPES
};

/**
 *  Flags for dpdoc::Renderer::setPlayMode()
 */
 enum PlayFlag
 {
	PF_PLAY_MOVIE = 1,		/**< Play animations and video streams embedded in the document. */
	PF_PLAY_SOUND = 2,		/**< Play audio embedded in the document. */
	PF_LOOP = 4,			/**< Loop playback of the animaions, video and audio in the document. */
	PF_FAST = 8,			/**< Value performance over quality. */
	PF_TRANSPARENT = 0x10	/**< Don't draw background. Corresponds to wmode */
 };


/**
 * Type of objects for a ContentInterator to iterate over
 */
enum ContentVariety
{
	CV_TEXT = 1,                 /**< This type of iterator points to a position between two characters in the document text */
	CV_LINK = 2                  /**< This type of iterator points to a link - Not supported yet */
};

/**
 *  Return value for dpdoc::Renderer::performAction
 */
enum ActionResult
{
	AR_UNKNOWN		= 0,	/**< action is not supported */ 
	AR_NO_ACTION	= 1,	/**< no such action present in document */ 
	AR_SUCCESS		= 2,	/**< action performed successfully */
	AR_ERROR		= 3		/**< error performing the action */
};

 /**
 *  Information about page decorations for dpdoc::PM_SCROLL_PAGES paging mode. All lengths are in
 *  the document coordinate space. Colors are in 0xRRGGBB format.
 */
struct PageDecoration
{
	double pageGap;						/**< distance between the bottom edge of the page N and top edge of the page N+1 */
	double borderWidth;					/**< width of the border around the page edge */
	double shadowWidth;					/**< width of the shadow around the bottom-right border edge */
	double shadowOffset;				/**< offset of the left and top edges of the shadow relative to the top and left edge of the border */
	unsigned int pageBackgroundColor;	/**< color of the blank page */
	unsigned int shadowColor;			/**< color of the page shadow */
	unsigned int borderColorTL;			/**< border color for top and left edges */
	unsigned int borderColorBR;			/**< border color for bottom and right edges */

	PageDecoration()
	: pageGap(8), borderWidth(1), shadowWidth(1.5), shadowOffset(2.5), pageBackgroundColor(0xFFFFFF),
		shadowColor(0x66645C), borderColorTL(0x66645C), borderColorBR(0x000000)
	{
	}
};

/**
 *  Factory class for dpdoc::Document. Many providers for different content types can be registered.
 */
class DocumentProvider : public dp::Unknown
{
public:

	virtual int getInterfaceID() { return IID_DocumentProvider; }
	/**
	 *  Creates a Document that can handle given mime type.
	 *  \return new Document or NULL if this mime type cannot be handled.
	 */
	virtual Document * createDocument( DocumentClient * client, const dp::String& mimeType ) = 0;

	/**
	 *  Add a DocumentProvider object to the list of registered providers. Providers registered later have priority.
	 */
	static void addProvider( DocumentProvider * provider );
};

/**
 *  \brief An object that represents a navigatable location in the document.
 */
class Location : public dp::RefCounted
{
protected:

	/**
	 *  Destructor should not be called directly, use release() mehod instead.
	 */
	virtual ~Location() {}

public:

	virtual int getInterfaceID() { return IID_Location; }
	/**
	 *  Get a bookmark that represents this location as precisely as possible. Returned string must not be
	 *  used after this Location is released.
	 *  \return URL-encoded bookmark string
	 *  \commands
	 *  \commandrefbr{cnvtLocToBookmk}
	 */
	virtual dp::String getBookmark() = 0;
	/**
	 *  Compare two location by their position in the document. Not all locations
	 *  can be compared precisely. If value returned in \p precise variable is false,
	 *  and returned value is zero, locations point <i>approximately</i> to the same
	 *  position in the document (e.g. the same page).
	 *
	 *  Comparison operation can be fairly expensive. Only Locations that originated
	 *  from the same document can be compared.
	 *
	 *  \param other location to which this location should be compared
	 *  \return 0 - locations point to the same position; -1 - this comes first, 1 - other comes first 
	 */
	virtual int compare( const dp::ref<Location>& other ) = 0;

	/**
	 *  Find the page position for a given location within the document. The
	 *  page position contains the page number as described in getPageCount as
	 *  the integer part and a position within that page as the fractional
	 *  part. The page number is a stable, intrinsical function of the
	 *  document.
	 *
	 *  \return Page position
 	 */
	virtual double getPagePosition() = 0;
};

/**
 *  Class that represents a metadata element
 */
class MetadataItem : public dp::RefCounted
{
protected:

	~MetadataItem() {}

public:

	virtual int getInterfaceID() { return IID_MetadataItem; }
	/**
	 *  Metadata value
	 */
	virtual dp::String getValue() = 0;
	/**
	 *  Metadata attribute
	 */
	virtual dp::String getAttribute( const dp::String& ns, const dp::String& name ) = 0;
};

/**
 *  Document range, represented by two dpdoc::Location objects
 */
struct Range
{
	dp::ref<Location> beginning;	/**< the beginning of the range */
	dp::ref<Location> end;			/**< the end of the range */
};

/**
 *  Information about the interal document link
 */
struct LinkInfo
{
	dp::ref<Location> beginning;	/**< beginning of the link origin range */
	dp::ref<Location> end;			/**< end of the link origin range */
	dp::ref<Location> target;		/**< link target */
};

/**
 *  Return values for dpdoc::Document::getNaturalUnit()
 */
enum NaturalUnit
{
	NU_PIXEL = 0,	/** unit of measurment in the document is a pixel (as defined in the CSS specification) */
	NU_POINT = 1,	/** unit of measurment in the document is a point (1/72th of an inch) */
};

/**
 * Abstract interface for an object that can parse a particular kind of content (such as PDF, XHTML or JPEG).
 */
class Document : public dp::Releasable
{
protected:

	virtual ~Document() {}

public:

	virtual int getInterfaceID() { return IID_Document; }
	/**
	 *  Release resources associated with this instance of a Document. No calls to this instance
	 *  must be made after release is called. See \ref lifetime.
	 */
	virtual void release() = 0;
	/**
	 *  Get the interface version which this Document implements. Once the interface is considered
	 *  to be frozen (it is not yet), any modification to it will be accompanied by incrementing
	 *  return value for this method.
	 *  \return currently must return 1.
     *  \commands
     *  \commandrefbr{getDocumentInterfaceVersion}
	 */
	virtual int getInterfaceVersion() = 0;
	/** 
	 *  Returns the native unit of measurment for this document. Some types of content has dimensions
	 *  expressed more naturally in pixels, for other types points make more sense. Whatever choice renderer makes
	 *	for the unit, that unit is considered to be a unit length in the document's coordinate system (see \ref systems).
	 *  See dpdoc::NaturalUnit enum for return values.
	 */
	 virtual int getNaturalUnit() = 0;
	/**
	 *  Assigns URL to load. This initiates document loading.
	 *  The \p url string is valid only for the duration of the call, Document must make a copy
	 *  if it needs url for later use.
	 *  \param url URL to load
	 */
	virtual void setURL( const dp::String& url ) = 0;
	/**
	 *  Extracts named metadata item from the document.
	 *
	 *  To query metadata, Dublin Core names should be used, e.g.:
	 *  - "DC.title" - title of the document
	 *  - "DC.creator" - who created the content (author)
	 *  - "DC.date" - when the document was created
	 *  .
	 *
	 *  Note the casing of the prefix and element names. These are taken from
	 *  http://dublincore.org/documents/dc-html/ and DC prefix is considered to be
	 *  predefined.
	 *
	 *  Note on EPUB metadata:
	 *  Metadata recorded using opf:meta element can be queried simply using name
	 *  attribute value. Custom-namespace metadata elements are not supported by this API. 
	 *
	 *  Some metadata items can have additional attributes such as language (xml:lang) or type
	 *  (xsi:type). These can be queried using dpdoc::MetadataItem methods
	 *
	 *  \param name metadata name, UTF8-encoded
	 *  \param index 0-based index for values that can have multiple values
	 *  \return dpdoc::MetadataItem if metadata is present, NULL if its not
	 */
	virtual dp::ref<MetadataItem> getMetadata( const dp::String& name, int index ) = 0;
	/**
	 *  Returns the root of the document's table of content (TOC). If this type of document
	 *  cannot have a table of content or TOC functionality is not implemented, can return NULL.
	 *  Root TOC item's title is meaningless and can be NULL. Root TOC item can be empty (have
	 *  no children).
	 */
	virtual TOCItem * getTocRoot() = 0;
	/**
	 *  Returns location that corresponds to the beginning of the document.
	 *  \return new Location object.
     *  \commands
     *  \commandrefbr{getDocumentBeginning}
	 */
	virtual dp::ref<Location> getBeginning() = 0;
	/**
	 *  Returns location that corresponds to the end of the document (or better to say "just after
	 *  the end of the document, that difference is important for findNext()). 
	 *  \return new Location object.
     *  \commands
     *  \commandrefbr{getDocumentEnd}
	 */
	virtual dp::ref<Location> getEnd() = 0;
	/**
	 *  Returns location that corresponds to the bookmark. A \e bookmark is an ASCII string.
	 *  Non-ASCII, non-printable ASCII characters as well as space, '%', and '+' must
	 *  be URL-encoded (or represented in some other way). Precise format of the bookmark
	 *  is document-type specific, but any valid fragment identifier (that normally can go after # sign in a URL,
	 *  including '#' itself) should be a valid bookmark.
	 *  \return new Location object.
	 *  \commands
	 *  \commandrefbr{cnvtBookmkToLoc}
	 */
	virtual dp::ref<Location> getLocationFromBookmark( const dp::String& bookmark ) = 0;
	/**
	 *  Find the occurence of the given text \p stringToFind after the given range (between location
	 *  \p start and location \p end). The start location specifies the first location to be searched.
	 *  In other words, if the search cannot wrap (SF_WRAP is not set), start is before end on forward
	 *  seaches and after end on backward searches. When SF_WRAP is set, either the start or end may
	 *  be first, but wrapping will only occur if start is after end for forward searches or end is
	 *  before start for backward searches.	 This method does not navigate to the found location nor
	 *  selects it.
	 *  \param start starting location for the search.
	 *  \param end end location for the search.
	 *  \param flags 0 or combination of dpdoc::SearchFlags values ORed together.
	 *  \param stringToFind UTF8-encoded string to search for.
	 *  \param resStart where to return the location of the beginning of the found text.
	 *  \param resEnd where to return the location of the end of the found text.
	 *  \return \b true if text was found, \b false otherwise.
     *  \commands
     *  \commandrefbr{findText}
	 */
	virtual bool findText( const dp::ref<dpdoc::Location>& start, const dp::ref<dpdoc::Location>& end, unsigned int flags, const dp::String& stringToFind,
		Range * result ) = 0;
	/**
	 *  Query the text between two locations. If there is not text between these locations, returns
	 *  empty string.
	 *  See \ref ranges.
	 *  \param start beginning of the range
	 *  \param end the end of the range
	 *  \return text as UTF8 string. 
	 */
	virtual dp::String getText( const dp::ref<Location>& start, const dp::ref<Location>& end ) = 0;
	/**
	 *  Find the size of the document in pages. The pages in this count are
	 *  those defined in the original document. They do not necessarily corre
	 *  spond to a screen full of content.
	 */
	virtual double getPageCount() = 0;
	/**
	 *  Find the location for the given page position. See 
	 *  getPagePositionFromLocation for a description of page positions.
	 *  \param pos Page position
	 *  \return Location within document
     *  \commands
     *  \commandrefbr{getLocationFromPagePosition}
	 */
	virtual dp::ref<Location> getLocationFromPagePosition( double pos ) = 0;
	/**
	 *  Gets the page name for a page position. This page name is the
	 *  number that would be shown at the top or bottom of the page in a
	 *  printed book. It is a UTF-8 character string and it may not restricted
	 *  to contain just numbers. Refer to getPagePositionFromLocation for a description
	 *  of a page position. The fractional portion of the page position is
	 *  ignored in this call.
	 *  \param pos Page position
 	 *  \return UTF-8 string containing page name
	 */
	virtual dp::String getPageName( double pos ) = 0;
	/**
	 *  Get the page position for the page name specified. See
	 *  getPagePositionFromLocation for a description of page numbers and
	 *  getPageName for a descritpion of page names. The page position returned
	 *  by the call will always be a whole number.
	 *  \param name UTF-8 string containing the page name
	 *  \return Page position. Returns a negative value if the page name does
	 *   not occur within the document.
	 */
	virtual double getPagePosition( const dp::String& name) = 0;
	/**
	 *  Sets the information for assigning synthetic page numbers.
	 *  \param page Assigns the name of the first synthetic page to the given integer. Synthetic page
	 *  names will be generated string representation of the sequence of integers starting
	 *  from that number.
	 *  \param pageCount the number of pages designated for this document. The page numbers should be 
	 *  evenly split amongst this number of pages based on character counts.
	 */
	virtual void setSyntheticPageInfo(int page, int pageCount) = 0;
	/**
	 *  Assigns URL to of the page map to use. This initiates page map loading.
	 *  The \p url string is valid only for the duration of the call, Document must make a copy
	 *  if it needs url for later use.
	 *  \param url URL to load
	 */
	virtual void setPageMapURL( const dp::String& url ) = 0;
	 /**
	  *  Get an iterator that will iterate over the content in the document. The variety parameter
	  *  specifies what content the iterator will see. The position parameter provides the initial position.
	  */
	virtual ContentIterator * getContentIterator( int variety, const dp::ref<Location>& position ) = 0;
	/**
	 *  Creates a renderer to render the content loaded into the document.
	 *  Provides RendererHost that the new Renderer will use to access host-provided functionality.
	 *  The RendererHost provided must remain valid for the duration of Renderer life
	 *  (until Renderer::release() is called). Not all Document objects support multiple
	 *  renderers.
	 *  \param client RendererClient interface for this Renderer callbacks.
	 */
	virtual Renderer * createRenderer( RendererClient * client ) = 0;
	/**
	 * Gets a pointer to the error list for this document. This method never returns NULL. If no errors
	 * or warnings exist on this Document, this call returns a pointer to an empty list with a length of 0.
     * \commands
     * \commandrefbr{getErrorList}
	 */
	virtual dp::ref<dp::ErrorList> getErrorList() = 0;
	/**
	 *  Provides password for password-protected documents. This method should be called when password is requested
	 *  by DocumentClient::requestDocumentPassword() method. See also \ref password.
	 */
	virtual void setDocumentPassword( const dp::String& password ) = 0;
	/**
	 * Provides decryption key for the document (or some subresource in the document).
	 */
	virtual void setLicense( const dp::String& type, const dp::String& resourceId, const dp::Data& license ) = 0;
	/**
	 *  Get document content as a stream, includes all licensing information. Can only be used
	 *  if the document is loaded from a synchronous random-access stream.
	 *
	 *  When writing is done DocumentClient::documentSerialized() will be called.
	 */
	virtual dpio::Stream * serializeDocumentAndLicense() = 0;
	/**
	 *  Returns a list of document licenses
     *  \commands
     *  \commandrefbr{getRights}
	 */
	virtual dp::ref<dpdrm::Rights> getRights() = 0;
	/**
	 *  Assignes a context device for this document. This document is assumed to be being fulfilled for use
	 *  on the specified device. Document can be assigned context device only before Document::setURL method
	 *  is called. Also, Document::createRenderer will always return NULL for such documents if they are DRM-protected.
	 */
	virtual void setContextDevice( dpdev::Device * device ) = 0;
	/**
	 *  Creates a Document that can handle given mime type. Loops through all registered DocumentProvider
	 *  objects and tries to create a document with a given type using each one of them.
	 *  \return new Document or NULL if this mime type cannot be handled.
	 */
	static Document * createDocument( DocumentClient * client, const dp::String& mimeType );

};

/**
 *  Page number pair
 */
struct PageNumbers
{
	int beginning;	/**< page at the beginning of the screen */
	int end;		/**< page at the end of the screen */

	PageNumbers() : beginning(0), end(0) {}
};

/**
 *  Structure to represent a rectangle
 */
struct Rectangle
{
	double xMin;	/**< min x */
	double yMin;	/**< min y */
	double xMax;	/**< max x */
	double yMax;	/**< max y */

	Rectangle() : xMin(0), yMin(0), xMax(0), yMax(0) {}
};

/**
 *  Abstract interface for an object that can render a particular kind
 *  of content (such as PDF, XHTML or JPEG). Can be created by calling
 *  Document::createRenderer().
 * 
 */
 class Renderer : public dp::Releasable
{
protected:

	/**
	 *  Destructor should not be called directly, use release() mehod instead.
	 */
	virtual ~Renderer() {}

public:

	virtual int getInterfaceID() { return IID_Renderer; }
	/**
	 *  Release resources associated with this instance of a Renderer. No calls to this instance
	 *  must be made after release is called. See \ref lifetime.
	 */
	virtual void release() = 0;
	/**
	 *  Get the interface version which this Renderer implements. Once the interface is considered
	 *  to be frozen (it is not yet), any modification to it will be accompanied by incrementing
	 *  return value for this method.
	 *  \return currently must return 1.
	 */
	virtual int getInterfaceVersion() = 0;
	/**
	 *  Get the capabilities this Renderer instance provides. Capabilities are not well-defined until document
	 *  is at least partially loaded (loading state becomes at least dpdoc::LS_INCOMPLETE).
	 *  \return a set of dpdoc::RendererCapabilities enum values ORed together
	 */
	virtual unsigned int getCapabilities() = 0;
	/**
	 *  Navigates to a location in the document. That can involve scrolling of the document or changing current
	 *  screen (see \ref navigation).
	 *  \param loc location to navigate to.
	 */
	virtual void navigateToLocation( const dp::ref<Location>& loc ) = 0;
	/**
	 *  Returns location that most closely corresponds to the current position in the document. Returned location
	 *  object has to be released with Location::release() method.
	 *  \return new Location object.
     *  \commands
     *  \commandrefbr{getLocation}
	 */
	virtual dp::ref<Location> getCurrentLocation() = 0;
	/**
	 *  Determines if the Renderer can render the document to the surface with the given pixel format.
	 *  See dpdoc::PixelLayout enum for details on pixel formats. Simple renderers might be unable to render certain
	 *  kinds of content into incompatible pixel formats (e.g. color images into black-and-white surface).
	 *  \param pixelLayout desired pixel format.
	 *  \return if document can be rendered into the surface with such pixel format.
	 */
	virtual bool supportsPixelLayout( int pixelLayout ) = 0;
	/**
	 *  Returns location that corresponds to the beginning of the current screen. Returned location
	 *  object has to be released with Location::release() method.
	 *  \return new Location object.
	 */
	virtual dp::ref<Location> getScreenBeginning() = 0;
	/**
	 *  Returns location that corresponds to the end of the current screen. Returned location
	 *  object has to be released with Location::release() method.
	 *  \return new Location object.
	 */
	virtual dp::ref<Location> getScreenEnd() = 0;
    /**
     *  Get the page number for the first and last information on the current screen. This has the same
     *  effect as calling:
     *      static_cast<int>(document->getPagePositionFromLocation(renderer->getScreenBeginning()) and
     *      static_cast<int>(document->getPagePositionFromLocation(renderer->getScreenEnd())
     *  In some cases, getPageNumbersForScreen is much more efficient than getting a location and converting
     *  it to a position. The method should be used whenever just the integer portion of the position of
     *  the start and end of the screen is desired.
     *  \param pageNumbers struct which will be filled with page numbers.
	 *  \return true
     */ 
    virtual bool getPageNumbersForScreen( PageNumbers * pageNumber ) = 0;
	/**
	 *  Gets the natural size of the content in either CSS pixels or points (1/72th of an inch). (Note that
	 *  CSS notion of pixel is not necessarily the same thing as device pixel). See dpdoc::Document::getNaturalUnit().
	 *  Not all types of content have meaningful natural size without external constraints (e.g. XHTML). In such case,
	 *  0/0 can be returned. However, after setViewport()
	 *  method is called, getNaturalSize() should, in general be able to calculate document's natural size. 
	 *  Changing default font size can cause natural size to change.
	 *  \param width address where Renderer should place natural width
	 *  \param height address where Renderer should place natural height
	 *  \param pixels address where Renderer should place true if width and height are expressed in CSS pixels and false for points.
	 */
	virtual bool getNaturalSize( Rectangle * dimensions ) = 0;
	/**
	 *  Gets a box containing the marked area of the current page in the documents logical coordinate system. The box
	 *  will contain all the marks on the page, but is not guaranteed to be the smallest possible box containg the
	 *  marks.  As with getNaturalSize, not all types of content can provide a meaningful result for getMarkedArea
	 *  until setViewport() is called.
     *  \commands
     *  \commandrefbr{getMarkedArea}
	 */
	virtual bool getMarkedArea( Rectangle * area ) = 0;

	/**
	 *  Assigns the size of the viewport in the document's coordinate system. (See \ref systems.) This size might be
	 *  used for line-wrapping (or rescaling) by some content. If renderer supports it (see getCapabilities()), 
	 *  host can ask it to clip to viewport. If viewport clipping is enabled, no pixels that don't intersect with the
	 *  viewport should be altered.
	 *  \param width viewport width
	 *  \param height viewport height
	 *  \param clip should renderer clip to the viewport
	 */
	virtual void setViewport( double width, double height, bool clip ) = 0;
	/**
	 *  Assigns environment matrix. See \ref systems.
	 *  \param t matrix that defines transformation of the renderer coordinate system into the device coordinate system.
	 */
	virtual void setEnvironmentMatrix( const Matrix& t ) = 0;
	/**
	 *  Assigns navigation matrix. See \ref navigation and \ref systems.
	 *  \param t matrix that defines transformation of the document coordinate system into the renderer coordinate system.
	 */
	virtual void setNavigationMatrix( const Matrix& t ) = 0;
	/**
	 *  Reads navigation matrix. See \ref navigation and \ref systems.
	 *  \param t matrix that defines transformation of the renderer coordinate system into device coordinate system.
	 */
	virtual bool getNavigationMatrix( Matrix * t ) = 0;
	/**
	 *  Draws a rectangular portion of the document onto a drawing surface. Pixels with coordinates
	 *  <code>xMin \<= x \< xMax</code> and <code>yMin \<= y \< yMax</code> can be painted. If renderer has
	 *  dpdoc::RC_BLEND capability, it should paint on top of the content already on the surface, blending with
	 *  it if needed. Otherwise it should not assume anything about the state of the surface before drawing.
	 *  Current screen is drawn, environment and navigation matrices are used (see \ref systems), clipping to
	 *  the viewport is performed if requested by host (see setViewport()).
	 *  \param xMin min x coordinate
	 *  \param yMin min y coordinate
	 *  \param xMax max x coordinate
	 *  \param yMax max y coordinate
	 *  \param surface surface to draw on
     *  \commands
     *  \commandrefbr{render}
     *  \commandrefbr{renderRect}
	 */
	virtual void paint( int xMin, int yMin, int xMax, int yMax, Surface * surface ) = 0;
	/**
	 *  Inspect the content of the current screen. Only implemented for EPUB.
	 *  \param handler callback that will get information about drawable content elements on the current screen.
	 *  \param types bitmask describing content elements of interest; flags are defined by dpdoc::DisplayElementType.
     *  \commands
     *  \commandrefbr{walkScreen}
	 */
	virtual void walkScreen( DisplayHandler * handler, unsigned int types ) = 0;
	/**
	 *  Dispatch an event (such as a mouse or keyboard event) to the Renderer.
	 *  \param evt and event to dispatch.
     *  \commands
     *  \commandrefbr{focusIn}
     *  \commandrefbr{focusOut}
     *  \commandrefbr{keyDown}
     *  \commandrefbr{keyUp}
     *  \commandrefbr{mouseClick}
     *  \commandrefbr{mouseDown}
     *  \commandrefbr{mouseEnter}
     *  \commandrefbr{mouseExit}
     *  \commandrefbr{mouseMove}
     *  \commandrefbr{mouseUp}
     *  \commandrefbr{navigateActivate}
     *  \commandrefbr{navigateFirst}
     *  \commandrefbr{navigateLast}
     *  \commandrefbr{navigateLeave}
     *  \commandrefbr{navigateNext}
     *  \commandrefbr{navigatePrevious}
	 */
	virtual void handleEvent( Event * evt ) = 0;
	/**
	 *  Determine if a point in the document coordinate system on current screen is a part of document
	 *  or not. Documents can have transparent areas, and if a given point falls within it, there
	 *  is "no hit". If \p locRes passed in is not NULL, document location which best corresponds to
	 *  \p x and \p y coordinates is stored there.
	 *  \param x x coordinate in the device coordinate system
	 *  \param y y coordinate in the device coordinate system
	 *  \param flags several of HitTestFlags values ORed together
	 *  \return location if point belongs to the document and null if it does not
	 */
	virtual dp::ref<Location> hitTest( double x, double y, unsigned int flags ) = 0;
	/**
	 *  Helps the host to determine if this document needs to be given focus. Hosts that process keyboard
	 *  input can determine if there are any focusable components inside this document (e.g. links).
	 *  \return true if there are potentially focusable components in this document, false otherwise.
	 */
	virtual bool isFocusable() = 0;
	/**
	 *  Return Renderer paging mode. See \ref paging.
	 *  \return one of values of the PagingMode enum.
	 */
	virtual int getPagingMode() = 0;
	/**
	 *  Set Renderer's paging mode. See \ref paging. This may or may not be honored by the Renderer.
	 *  If the requested paging mode is supported, renderer must switch to it. 
	 */
	virtual void setPagingMode( int pagingMode ) = 0;
	/**
	 *  Set information for page drawing in dpdoc::PM_SCROLL_PAGES paging mode
	 */
	virtual void setPageDecoration( const PageDecoration& pageDecoration ) = 0;
	/**
	 *  Make next screen full of content current. If this was the last screen, the call has no effect.
	 *  For scrolled documents, scroll forward one screen full of content. See \ref paging.
	 *  \return true if went to a next screen, false if this was the last screen
	 */
	virtual bool nextScreen() = 0;
	/**
	 *  Make next screen full of content current. If this was the first screen, the call has no
	 *  effect. For scrolled documents, scroll backward one screen full of content.
	 *  See \ref paging.
	 *  \return true if went to a previous screen, false if this was the first screen
	 */
	virtual bool previousScreen() = 0;
	/**
	 *  Returns true if the current screen is the first in the document view.
	 */
	virtual bool isAtBeginning() = 0;
	/**
	 *  Returns true if the current screen is the last in the document view.
	 */
	virtual bool isAtEnd() = 0;
	/**
	 *  Get current default font size. Font size is not supported for PDF.
	 *  Initial font size for EPUB is 12 points.
	 *  \return current font size in points (1/72 in)
     *  \commands
     *  \commandrefbr{getDefaultFontSize}
	 */
	virtual double getDefaultFontSize() = 0;
	/**
	 *  Set current default font size. Font size is not supported for PDF.
	 *  Initial font size for EPUB is 12 points.
	 *  \param fontSize font size to set in points (1/72 in)
	 */
	virtual void setDefaultFontSize( double fontSize ) = 0;
	/**
	 *  Highlight the content between two locations.
	 *  See \ref ranges and \ref highlighting.
	 *  \param highlightType type type of the highlight, one of the dpdoc::HighlightType enum values.
	 *  \param start beginning of the range to highlight
	 *  \param end the end of the range to highlight
	 *  \return index of added highlight. A negative return value indicates the highlight cound not be added.
	 *  \commands
	 *  \commandrefbr{addActiveHighlight}
	 *  \commandrefbr{addAnnotationHighlight}
	 *  \commandrefbr{addSelectHighlight}
	 */
	virtual int addHighlight( int highlightType, const dp::ref<Location>& start, const dp::ref<Location>& end ) = 0;
	/**
	 *  Change highlight color. Only lower 24 bits of the color parameter are used.
	 *  Lowest 8 bits represent blue, next 8 bits represent green and next 8 bits - red.
	 *  \param highlightType type type of the highlight, one of the dpdoc::HighlightType enum values.
	 *  \param index index of the desired highlight.
	 *  \param color color of the desired highlight.
	 */
	virtual void setHighlightColor( int highlightType, int index, unsigned int color ) = 0;
	/**
	 *  Query the highlight color.
	 *  \param highlightType type type of the highlight, one of the dpdoc::HighlightType enum values.
	 *  \param index index of the desired highlight.
     *  \commands
     *  \commandrefbr{getActiveHighlightColor}
     *  \commandrefbr{getAnnotationHighlightColor}
     *  \commandrefbr{getSelectHighlightColor}
	 */
	virtual unsigned int getHighlightColor( int highlightType, int index ) = 0;
	/**
	 *  Navigate to a highlight
	 *  See \ref highlighting.
	 *  \param highlightType type type of the highlight, one of the dpdoc::HighlightType enum values.
	 *  \param index index of the desired highlight.
	 */
	virtual void navigateToHighlight( int highlightType, int index ) = 0;
	/**
	 *  Get the number of highlights of the type specified.
	 *  See \ref highlighting.
	 *  \param highlightType type type of the highlight to be counted, one of the dpdoc::HighlightType enum values.
	 *  \return number of highlights.
     *  \commands
     *  \commandrefbr{getActiveHighlightCount}
     *  \commandrefbr{getAnnotationHighlightCount}
     *  \commandrefbr{getSelectHighlightCount}
	 */
	virtual int getHighlightCount ( int highlightType ) = 0;
	/**
	 *  Query highlighted text locations. Returned locations must be released with Location::release call.
	 *  \param highlightType type type of the highlight, one of the dpdoc::HighlightType enum values.
	 *  \param index index of the desired highlight.
	 *  \param start where to place the beginning of the range
	 *  \param end where to place the end of the range
	 *  \return \b true if there is highlight of the specified type and index, \b false otherwise. 
     *  \commands
     *  \commandrefbr{getActiveHighlight}
     *  \commandrefbr{getAnnotationHighlight}
     *  \commandrefbr{getSelectHighlight}
	 */
	virtual bool getHighlight( int highlightType, int index, Range * range ) = 0;
	/**
	 *  Remove the highlighting specified by the type and index.
	 *  \param highlightType type type of the highlight, one of the dpdoc::HighlightType enum values.
	 *  \param index index of the desired highlight.
	 */
	virtual void removeHighlight( int highlightType, int index ) = 0;
	/**
	 *  Remove all highlighting of the specified type.
	 *  \param highlightType type type of the highlight, one of the dpdoc::HighlightType enum values.
	 */
	virtual void removeAllHighlights( int highlightType ) = 0;

	/**
	 *  Returns the number of links on the current screen.
	 *
	 *  Note: This API is needed for compatibility reasons and should be avoided.
	 *
	 *  The better way to handle links is by sending events to the renderer.
     *  \commands
     *  \commandrefbr{getLinkCount}
	 */
	virtual int getLinkCount() = 0;
	/**
	 *  Returns the link information. Link range is the range of the document's content
	 *  that corresponds to the link origin (e.g. between <code>&lt;a;gt;</code> and <code>&lt;/a;gt;</code>
	 *  in XHTML). Link target is location in this document where this link would navigate if activated.
	 *  Only local links are supported through this API.
	 *
	 *  Note: This API is needed for compatibility reasons and should be avoided.
	 *
	 *  The better way to handle links is by sending events to the renderer.
	 *
	 *  \param linkIndex index of the link on the current page.
	 *  \return \b true if link information could be returned, \b false otherwise.
     *  \commands
     *  \commandrefbr{getLinkLocations}
	 */
	virtual bool getLinkInfo( int linkIndex, LinkInfo * target ) = 0; 
	/**
	 *  Returns an object that holds bounding boxes for the location range in the document's coordinate system
	 *  for current screen only.
	 *
	 *  Note: this method may \b not be the best starting point for highlighting the content of the document
	 *  because of the non-rectangular-shaped selections (e.g. text on a path) and rotated text.
	 *  The preferred method for highlighting is using addHighlight() method.
	 *
	 *  Returned RangeInfo object must be released before any APIs on this renderer are called.
	 *
	 *  \param start beginning of the range
	 *  \param end the end of the range
	 *  \return the page number where the \p start location is (or -1 if page number is not yet known).
	 */
	virtual RangeInfo * getRangeInfo( const dp::ref<Location>& start, const dp::ref<Location>& end ) = 0;
	/**
	 * Set page margins. In general, renderer should try to leave blank
	 * margins of at least the specified width. Viewport width and height includes margins.
	 */
	virtual void setMargins( double top, double right, double bottom, double left ) = 0;
	/**
	 *  Set CSS media type (see http://www.w3.org/TR/REC-CSS2/media.html).
	 *  Setting this can trigger media-dependent stylesheets. Default is "screen"
	 */
	virtual void setCSSMediaType( const dp::String& media ) = 0;
	/**
	 *  Set the play mode, which is a combination of the dpdoc::PlayFlag flags. Initial play mode
	 *  is 0.
	 */
	virtual void setPlayMode( unsigned int playMode ) = 0;
	/**
	 *  Get the play mode, which is a combination of the dpdoc::PlayFlag flags. Initial play mode
	 *  is 0.
	 */
	virtual unsigned int getPlayMode() = 0;
	/**
	 *
	 *  Enables or disables display of page numbers in some types of documents where the page
	 *  number display is generated by the renderer.
     *  \commands
     *  \commandrefbr{enablePageNumbers}
     *  \commandrefbr{disablePageNumbers}
	 */
	virtual void showPageNumbers( bool enableDisplay ) = 0;
	/**
	 *  Enables or disables iteration over external links in the document. By default navigation
	 *  events (with event kind dpdoc::EK_NAVIGATE) only highlight local links in the document
	 *  (those that target some other portion of the document, not, say, Internet URLs). Enabling
	 *  external links allow highlighting of all the links, both external and local. Note that
	 *  this setting currently does not have an effect on Renderer::getLinkInfo and Renderer::getLinkCount
	 *  APIs - those APIs only work on local links.
     *  \commands
     *  \commandrefbr{enableExternalLinks}
     *  \commandrefbr{disableExternalLinks}
	 */
	virtual void allowExternalLinks( bool enableIteration ) = 0;
	/**
	 *  Sets the renderer in the hibernation state: the renderer keeps the state, but shuts
	 *  down all other services. If the result of the call is true, there was no state to
	 *  keep and equivalent renderer can be re-created from sources.
	 */
	virtual bool hibernate() = 0;
	/**
	 *  Wakes up the document after hibernation.
	 */
	virtual void wakeUp() = 0;
	/**
	 *  Perform action in the document. Currently only "open" action is supported in PDF.
	 *  Returns dpdoc::ActionResult
     *  \commands
     *  \commandrefbr{performAction}
	 */
	virtual int performAction( const dp::String& action ) = 0;
};

/**
 *  \brief An object that represents an item in the document's table of contents.
 */
 class TOCItem : public dp::Releasable
{
protected:

	/**
	 *  Destructor should not be called directly, use release() mehod instead.
	 */
	virtual ~TOCItem() {}

public:

	virtual int getInterfaceID() { return IID_TOCItem; }
	/**
	 *  Release this object. All TOCItem objects originated from a given Renderer (directly or through other TOCItem
	 *  objects) \e must be released before that Renderer is released.
	 */
	virtual void release() = 0;
	/**
	 *  Return the title of this table of content item. Returned string must not be
	 *  used after this TOCItem is released.
	 *  \return UTF8-encoded text.
	 */
	virtual dp::String getTitle() = 0;
	/**
	 *  Get Location to navigate to this table of content item. Location must be released
	 *  with Location::release() method.
	 *  \return item's location in the document.
	 */
	virtual dp::ref<Location> getLocation() = 0;
	/**
	 *  Get number of subitems in this TOC item.
	 */
	virtual int getChildCount() = 0;
	/**
	 *  Get a subitem in this TOC item. Subitem must be released with release() method.
	 */
	virtual TOCItem * getChild( int index ) = 0;
};

/**
 *  Information about a continuous subset of the document’s content. Returned
 *  by Renderer::getRangeInfo() method. The methods in this object refer to the
 *  current screen of the Renderer object used to create the class. To get info-
 *  mation about another screen of content, the Renderer object must be repos-
 *  tioned to that screen.
 */
 class RangeInfo : public dp::Releasable
{
protected:
  
	/**
	 *  Destructor should not be called directly, use release() mehod instead.
	 */
	virtual ~RangeInfo() {}

public:
	virtual int getInterfaceID() { return IID_RangeInfo; }
	/**
	 *  Release this object. All RangeInfo objects originated from a given
	 *  Renderer must be released before that Renderer is released.
	 */
	virtual void release() = 0;
	/**
	 *  Determines if this RangeInfo starts before this screen.	This method
	 *  will return true whether the Range ends before this screen or continues
	 *  on to it.
	 *  \return true if range starts before the current screen, else false.
	 */
	virtual bool startsBeforeThisScreen() = 0;

	/**
	 *  Determines if this RangeInfo ends before this screen. If this method
	 *  returns true, getBoxCount will return 0.
	 *  \return true if range ends before the current screen, else false.
	 */
	virtual bool endsBeforeThisScreen() = 0;

	/**
	 *  Determines if this RangeInfo starts after this screen. If this method
	 *  returns true, getBoxCount will return 0.
	 *  \return true if range starts after the current screen, else false.
	 */
	virtual bool startsAfterThisScreen() = 0;

	/**
	 *  Determines if this RangeInfo ends after this screen. This method will
	 *  return true whether the Range continues from this screen or starts
	 *  after it.
	 *  \return true if range ends after the current screen, else false.
	 */
	virtual bool endsAfterThisScreen() = 0;

	/**
	 *  Number of boxes that enclose the range content on the current screen. 
	 */
	virtual int getBoxCount() = 0;

	/**
	 *  Get box dimensions. If \p withTransform is false, box dimensions is in the
	 *  device coordinate system. If \p withTransform is true, box dimensions are
	 *  in the coordinate system returned in RangeInfo::getBoxTransform. Since the box can be
	 *  rotated, ignoring transformation (passing false) can cause box area to
	 *  be significantly enlarged.
	 *  \param boxIndex (in) zero-based index of the box, must be less than
	 *	 getBoxCount()
	 */
	virtual bool getBox( int boxIndex, bool withTransform, Rectangle * box ) = 0;
	/**
	 *  Get box transform
	 */
	virtual bool getBoxTransform( int boxIndex, Matrix * transform ) = 0;
};

/**
 * Flags for ContentIterator with a CV_TEXT iterator. If a 0 is specified for the flags, each call to next or previous
 * on the iterator will return a single character. If a JOIN flag is included consecutive characters are that type will
 * included in the string and the position will move to just after the final character. If an IGNORE_TRAILING flags is
 * included, consecutive characters of that type will be skipped over at the end of the text while positioning the
 * iterator, but they will not be included in the returned string. If multiple flags are specified, the types they
 * specify will be merged. For example, if CI_JOIN_ALPHA and CI_JOIN_NUMERIC are specified any combination of alphabetic
 * and numeric characters will be included in the resulting string.
 *
 * The following defines which characters are included in these sets. The values are Unicode code points.
 * - ALPHA
 *		- 0x0041 - 0x007A Basic Latin
 *		- 0x00C0 - 0x00D6 Latin-1 Supplement
 *		- 0x00D8 - 0x00F6 Latin-1 Supplement
 *		- 0x00F8 - 0x00FF Latin-1 Supplement
 *		- 0x0100 - 0x017F Latin Extended-A
 *		- 0x0180 - 0x024F Latin Extended-B
 *		- 0x0386          Greek
 *		- 0x0388 - 0x03FF Greek
 *		- 0x0400 - 0x0481 Cyrillic
 *		- 0x048A - 0x04FF Cyrillic
 *		- 0x0500 - 0x052F Cyrillic Supplement
 *		- 0x1E00 - 0x1EFF Latin Extended Additional
 * - NUMERIC
 *		- 0x0030 - 0x0039 ASCII digits
 * - SPACE
 *		- 0x0009          Horizontal Tab
 *		- 0x000A          Line Feed
 *		- 0x000B          Vertical Tab
 *		- 0x000C          Form Feed
 *		- 0x000D          Carriage Return
 *		- 0x0020          Space
 * - PUNCUATION
 *		- 0x0021          Exclamation
 *		- 0x002C          Comma
 *		- 0x002E          Period
 *		- 0x003A - 0x003B Colon and Semicolon
 *		- 0x003F          Question
 *		- 0x00A1          Inverted Exclamation
 *		- 0x00BF          Inverted Question
 * - Soft Hyphen
 *		- 0x00AD          Soft Hyphen
 *
 */
enum ContentIterationFlags
{
	CI_JOIN_ALPHA = 1,				/**< Join consecutive alphabetic characters */
	CI_JOIN_NUMERIC = 2,			/**< Join consecutive numeric characters */
	CI_IGNORE_TRAILING_SPACE = 4,	/**< Ignore trailing and leading space characters */
	CI_IGNORE_TRAILING_PUNC = 8,	/**< Ignore trailing and leading punctuation characters */
	CI_IGNORE_SHY = 16				/**< Ignore all soft hyphens */
};
 
/**
 * Provides a means of iterating through the content in a document or on a page. A ContentIterator
 * attempts to proceed through the document in reading order. For some document types, the reading
 * order may not be well specified.
 *
 * If any operation on a ContentIterator is aborted because a call to canContinueProcessing
 * returned false, the position and state of the ContentIterator are indeterminate. Any further
 * operations on the ContentIterator will produce undefined results.
 */
class ContentIterator : public dp::Releasable
{
protected:
  
	/**
	 *  Destructor should not be called directly, use release() mehod instead.
	 */
	virtual ~ContentIterator() {}

public:
	virtual int getInterfaceID() { return IID_ContentIterator; }
	/**
	 *  Release this object. All ContentIterator objects originated from a given
	 *  Document must be released before that Document is released.
	 */
	virtual void release() = 0;
	/**
	 *  Moves iterator on to the next item. Parameter can be any combination of dpdoc::ContentIterationFlags values.
	 *  A null string is returned if the end of the document is reached or an error occurs. If it is due to an error,
	 *  the error will be reported by a call to the reportDocumentError method in the DocumentClient object.
	 */
	virtual dp::String next( unsigned int flags ) = 0;
	/**
	 *  Moves iterator back to the previous item. Parameter can be any combination of dpdoc::ContentIterationFlags
	 *  values. A null string is returned if the start of the document is reached or an error occurs. If it is due
	 *  to an error, the error will be reported by a call to the reportDocumentError method in the DocumentClient
	 *  object.
	 */
	virtual dp::String previous( unsigned int flags ) = 0;
	/**
	 *  Gets a location for the current position of this iterator
	 */
	virtual dp::ref<Location> getCurrentPosition() = 0;
};

/**
 *  Document loading state. Values from this enum will be passed to DocumentClient::reportLoadingState method.
 */
enum LoadingState
{
	LS_INITIAL = 0,		/**< Document is not ready. */
	LS_INCOMPLETE = 1,	/**< Document is partially loaded, progressive rendering can start. */
	LS_COMPLETE = 2,	/**< Document is fully loaded and ready to be used. */
	LS_ERROR = 3		/**< Document referenced by Document::setURL could not be loaded */
};

/**
 *  Pointer cursor type. These correspond to CSS21 with addition of some CSS3 values (all implemented
 *  by Mozilla (except for CT_NONE) and most by IE, hover over descriptions to see corresponding cursors).
 *  Host can provide a reasonable remapping if it is not possible or practical to support all values.
 *  (For instance a lot of resizing cursors normally collapsed into a smaller subset.)
 */
enum CursorType
{
	CT_NONE,			/**< No cursor visible. */
	CT_AUTO,			/**< \htmlonly <span style="cursor:auto"> \endhtmlonly Host determines the cursor to display based on the current context. \htmlonly </span> \endhtmlonly */ 
	CT_CROSSHAIR,		/**< \htmlonly <span style="cursor:crosshair"> \endhtmlonly A simple crosshair (e.g., short line segments resembling a "+" sign). \htmlonly </span> \endhtmlonly */ 
	CT_DEFAULT,			/**< \htmlonly <span style="cursor:default"> \endhtmlonly The platform-dependent default cursor; often rendered as an arrow. \htmlonly </span> \endhtmlonly */ 
	CT_POINTER,			/**< \htmlonly <span style="cursor:pointer"> \endhtmlonly The cursor is a pointer that indicates a link. \htmlonly </span> \endhtmlonly */ 
	CT_MOVE,			/**< \htmlonly <span style="cursor:move"> \endhtmlonly Indicates something is to be moved. \htmlonly </span> \endhtmlonly */
	CT_E_RESIZE,		/**< \htmlonly <span style="cursor:e-resize"> \endhtmlonly Indicate that eastern edge of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_EW_RESIZE,		/**< \htmlonly <span style="cursor:ew-resize"> \endhtmlonly Indicate that vertical edge of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_NE_RESIZE,		/**< \htmlonly <span style="cursor:ne-resize"> \endhtmlonly Indicate that north-eastern corner of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_NESW_RESIZE,		/**< \htmlonly <span style="cursor:nesw-resize"> \endhtmlonly Indicate that north-eastern/south-western corner of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_NW_RESIZE,		/**< \htmlonly <span style="cursor:nw-resize"> \endhtmlonly Indicate that north-western corner of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_NWSE_RESIZE,		/**< \htmlonly <span style="cursor:nwse-resize"> \endhtmlonly Indicate that north-western/south-eastern corner of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_N_RESIZE,		/**< \htmlonly <span style="cursor:n-resize"> \endhtmlonly Indicate that northen edge of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_NS_RESIZE,		/**< \htmlonly <span style="cursor:ns-resize"> \endhtmlonly Indicate that horizontal edge of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_SE_RESIZE,		/**< \htmlonly <span style="cursor:se-resize"> \endhtmlonly Indicate that south-eastern corner of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_SW_RESIZE,		/**< \htmlonly <span style="cursor:sw-resize"> \endhtmlonly Indicate that south-western corner of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_S_RESIZE,		/**< \htmlonly <span style="cursor:s-resize"> \endhtmlonly Indicate that southern edge of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_W_RESIZE,		/**< \htmlonly <span style="cursor:w-resize"> \endhtmlonly Indicate that western edge of a box is to be moved. \htmlonly </span> \endhtmlonly */
	CT_COL_RESIZE,		/**< \htmlonly <span style="cursor:col-resize"> \endhtmlonly Indicate that the item/column can be resized horizontally; often rendered as arrows pointing left and right with a vertical bar separating them. \htmlonly </span> \endhtmlonly */ 
	CT_ROW_RESIZE,		/**< \htmlonly <span style="cursor:row-resize"> \endhtmlonly Indicate that the item/row can be resized vertically; often rendered as arrows pointing up and down with a horizontal bar separating them. \htmlonly </span> \endhtmlonly */
	CT_TEXT,			/**< \htmlonly <span style="cursor:text"> \endhtmlonly Indicate text that may be selected; often rendered as an I-beam. \htmlonly </span> \endhtmlonly */
	CT_VERTICAL_TEXT,	/**< \htmlonly <span style="cursor:vertical-text"> \endhtmlonly Indicate vertical-text that may be selected; often rendered as a horizontal I-beam. \htmlonly </span> \endhtmlonly */
	CT_WAIT,			/**< \htmlonly <span style="cursor:wait"> \endhtmlonly Indicate that the program is busy and the user should wait; often rendered as a watch or hourglass. \htmlonly </span> \endhtmlonly */
	CT_HELP,			/**< \htmlonly <span style="cursor:help"> \endhtmlonly Help is available for the object under the cursor; often rendered as a question mark or a balloon. \htmlonly </span> \endhtmlonly */
	CT_PROGRESS,		/**< \htmlonly <span style="cursor:progress"> \endhtmlonly A progress indicator; the program is performing some processing, but is different from 'wait' in that the user may still interact with the program; often rendered as an arrow with a watch or hourglass. \htmlonly </span> \endhtmlonly */
	CT_CELL,			/**< \htmlonly <span style="cursor:cell"> \endhtmlonly Indicate that a cell or set of cells may be selected; often rendered as a thick plus-sign with a dot in the middle. \htmlonly </span> \endhtmlonly */
	CT_COPY,			/**< \htmlonly <span style="cursor:copy"> \endhtmlonly Indicates something is to be copied; often rendered as an arrow with a small plus sign next to it. \htmlonly </span> \endhtmlonly */
	CT_ALIAS,			/**< \htmlonly <span style="cursor:alias"> \endhtmlonly Indicates an alias of/shortcut to something is to be created; often rendered as an arrow with a small curved arrow next to it. \htmlonly </span> \endhtmlonly */
	CT_NO_DROP,			/**< \htmlonly <span style="cursor:no-drop"> \endhtmlonly Indicates that the dragged item cannot be dropped at the current cursor location; often rendered as a hand or pointer with a small circle with a line through it. \htmlonly </span> \endhtmlonly */
	CT_NOT_ALLOWED,		/**< \htmlonly <span style="cursor:not-allowed"> \endhtmlonly Indicate that the requested action will not be carried out. Often rendered as a circle with a line through it. \htmlonly </span> \endhtmlonly */
	CT_ALL_SCROLL		/**< \htmlonly <span style="cursor:all-scroll"> \endhtmlonly Indicates that the something can be scrolled in any direction; often rendered as arrows pointing up, down, left, and right with a dot in the middle. \htmlonly </span> \endhtmlonly */
};

/**
 *  Values for the \e kind parameter to DocumentHost::canContinueProcessing().
 */
enum ProcessingKind
{
	PK_BACKGROUND,		/**< can continue background processing */
	PK_RENDER,			/**< can continue rendering */
	PK_SEARCH,			/**< can continue search */
	PK_HIT_TEST,		/**< can continue hit testing */
	PK_FOREGROUND		/**< can continue any other foreground operation */
};

/**
 *  Information about the point in the document where the pointing device is.
 */
struct MouseLocationInfo
{
	int pointerType;		/**< the mouse cursor shape, represented by a value from the CursorType enum */
	dp::String linkURL;		/**< link URL when hovering over a link */
	dp::String tooltip;		/**< Tooltip info (e.g. alt attribute value for an img elemen) */
	int highlightType;		/**< a value from the HighlightType enum - HT_NONE if not over a highlight */
	int highlightIndex;		/**< index of the highlight under the mouse cursor */
};

/**
 *  Interface that provides hosting services to a Document.
 */
class DocumentClient : public dp::Unknown
{
protected:

	/**
	 *  Since implementor of this interface controls its lifetime, there is no need to
	 *  have a relase method or have public destructor in the interface. 
	 */
	virtual ~DocumentClient() {}

public:

	virtual int getInterfaceID() { return IID_DocumentClient; }
	/**
	 *  Get the interface version which this object implements. Once the interface is considered
	 *  to be frozen (it is not yet), any modification to it will be accompanied by incrementing
	 *  return value for this method.
	 *  \return currently must return 1.
	 */
	virtual int getInterfaceVersion() = 0;
	/**
	 *  Request resource download from a given url with a Stream with at least given capabilities.
	 *  Security considerations are responsibilities of the host. If NULL is returned,
	 *  request is considered to be failed.
	 */
	virtual dpio::Stream * getResourceStream( const dp::String& url, unsigned int caps ) = 0;
	/**
	 *  Document can call this to find out if some lengthy processing can be continued.
	 *
	 *  Processing can be of two kinds: background or foreground. Foreground processing is done
	 *  when a Document method is called to to something (e.g. render a page or search for some text).
	 *  Background processing happens in a timer call scheduled by a Document. When lengthy processing
	 *  is performed, canContinueProcessing will be called. 
	 *
	 *  If <code>false</code> is returned, Document or Renderer should release control to the host as soon as possible.
	 *
	 *  Interrupting background processing should always be safe in the sense that it won't cause
	 *  any calls on the Processor/Document/Renderer objects to fail and it should be able to resume
	 *  roughly from the point where it stopped. Interrupting foreground call will produce incorrect
	 *  results for that call; for instance if rendering call is interrupted, document will be only
	 *  partially rendered. Another call to the same method is needed to get the complete result (which in
	 *  general will have to be re-done from the beginning).
	 *
	 *  Host also can use this method to perform some sort of background processing of its own (e.g. driving dialog
	 *  message loop), however no calls to the renderer side can be made from within this call.
	 *
	 *  DocumentHost can simply always do <code>return kind != dpdoc::PK_BACKGROUND;</code>.
	 *
	 *  \param kind what kind of processing is being requested; one of the values from ProcessingKind enum
	 *  \return true if processing can continue, false if processing has to stop as soon as possible
	 */
	virtual bool canContinueProcessing( int kind ) = 0;
	/**
	 *  Update renderer's loading state. Use one of the dpdoc::LoadingState codes.
	 */
	virtual void reportLoadingState( int state ) = 0;
    /**
	 * Reports a process error on a document. The parameter must point to a valid error string.
	 */
	virtual void reportDocumentError(const dp::String& errorString) = 0;
    /**
	 * Reports a change to the ErrorList in the Document.
     */
    virtual void reportErrorListChange() = 0;
	/**
	 * Request for a decryption key needed to load the document (or some resource in the document).
	 */
	virtual void requestLicense( const dp::String& type, const dp::String& resourceId, const dp::Data& requestData ) = 0;
	/**
	 *  Notify the client that the document is password-protected. Client should ask user to enter the password and
	 *  call Document::setDocumentPassword method to supply it to the document. See also \ref password.
	 */
	virtual void requestDocumentPassword() = 0;
	/**
	 * Notification that the stream, created by Document::serializeDocumentAndLicense() was released.
	 */
	virtual void documentSerialized() = 0;
};

/**
 *  Interface that provides hosting services to a Renderer.
 *
 *  \commands
 *  \commandrefbr{disableCallbackReporting}
 *  \commandrefbr{enableCallbackReporting}
 */
class RendererClient : public dp::Unknown
{
protected:

	/**
	 *  Since implementor of this interface controls its lifetime, there is no need to
	 *  have a relase method or have public destructor in the interface. 
	 */
	virtual ~RendererClient() {}

public:

	virtual int getInterfaceID() { return IID_RendererClient; }
	/**
	 *  Get the interface version which this RendererHost implements. Once the interface is considered
	 *  to be frozen (it is not yet), any modification to it will be accompanied by incrementing
	 *  return value for this method.
	 *  \return currently must return 1.
	 */
	virtual int getInterfaceVersion() = 0;
	/**
	 *  Return number of CSS pixels in an inch.
	 */
	virtual double getUnitsPerInch() = 0;
	/**
	 *  Notifies the client that this region has changed (became "dirty") and needs to be repainted. Typically
	 *  client must repaint all dirty regions in the call to dpdoc::Renderer::paint from the top level, not
	 *  from within this callback.
	 *
	 *  Coordinates are in the device coordinate space.
	 */
	virtual void requestRepaint( int xMin, int yMin, int xMax, int yMax ) = 0;
	/**
	 *  Renderer calls this method when a link is activated.
	 */
	virtual void navigateToURL( const dp::String& url, const dp::String& target ) = 0;
	/**
	 *  Report desired cursor shape and other information about the point under the cursor. Called
	 *  as a result of the mouse event processing.
	 */
	virtual void reportMouseLocationInfo( const MouseLocationInfo& info ) = 0;
	/**
	 *  Called when either navigation matrix or current page number changes. See \ref navigation.
	 */
	virtual void reportInternalNavigation() = 0;
	/**
	 *  Called when either total page number or document's natural size changes.
	 */
	virtual void reportDocumentSizeChange() = 0;
	/**
	 *  Called by Renderer when highlighting changes.
	 */
	virtual void reportHighlightChange( int highlightType ) = 0;
    /**
	 * Reports a process error on in a renderer. The parameter must point to a valid error string.
	 */
	virtual void reportRendererError(const dp::String& errorString) = 0;
	/**
	 *  If a renderer was set to play with setPlayMode() method, report that a playback
	 *  has finished. If playback is set to loop this is called after every repeat.
	 */
	virtual void finishedPlaying() = 0;
};

/**
 *  Kinds of surface. Only raster-based surfaces are possible now.
 */
enum SurfaceKind
{
	SK_RASTER /**< bitmap-based surface */
};

/**
 *  Codes to define layout of a pixel.
 */
enum PixelLayout
{
	PL_L      = 0x000, /**< 8-bit-per-channel grayscale */
	PL_RGB    = 0x001, /**< 8-bit-per-channel RGB */
	PL_BGR    = 0x002, /**< 8-bit-per-channel BGR */
	PL_RGB16  = 0x004, /**< 16-bit RGB (not supported yet) */
	PL_BGR16  = 0x008, /**< 16-bit BGR (not supported yet) */
	PL_AC     = 0x010, /**< 8-bit alpha before color channels */
	PL_CA     = 0x020, /**< 8-bit alpha after color channels */
	PL_XC     = 0x100, /**< 8-bit padding (unused) before color channels */
	PL_CX     = 0x200, /**< 8-bit padding (unused) after color channels */

	PL_LA     = PL_L|PL_CA, /**< 8-bit-per-channel (grayscale, alpha) */
	PL_AL     = PL_L|PL_AC, /**< 8-bit-per-channel (alpha, grayscale) */
	PL_RGBA   = PL_RGB|PL_CA, /**< 8-bit-per-channel (red, green, blue, alpha) */
	PL_ARGB   = PL_RGB|PL_AC, /**< 8-bit-per-channel (alpha, red, green, blue) */
	PL_BGRA   = PL_BGR|PL_CA, /**< 8-bit-per-channel (blue, green, red, alpha) */
	PL_ABGR   = PL_BGR|PL_AC,  /**< 8-bit-per-channel (alpha, blue, green, red)*/
	PL_RGBX   = PL_RGB|PL_CX, /**< 8-bit-per-channel (red, green, blue, unused) */
	PL_XRGB   = PL_RGB|PL_XC, /**< 8-bit-per-channel (unused, red, green, blue) */
	PL_BGRX   = PL_BGR|PL_CX, /**< 8-bit-per-channel (blue, green, red, unused) */
	PL_XBGR   = PL_BGR|PL_XC  /**< 8-bit-per-channel (unused, blue, green, red)*/
};

/**
 *  Interface that describes a drawing surface.
 *
 *  All surfaces must support raster-based drawing interface.
 */
class Surface : public dp::Unknown
{
protected:

	virtual ~Surface() {}

public:

	virtual int getInterfaceID() { return IID_Surface; }
	/**
	 *  Get what kind of the surface this is. Currently only one kind is possible: dpdoc::SK_RASTER.
	 *  This kind of surface does not implement any additional functionality besides raster surface
	 *  interface.
	 *  \return a value from dpdoc::SurfaceKind enum.
	 */
	virtual int getSurfaceKind() = 0;
	/**
	 *  Get raster pixel format description. See dpdoc::PixelLayout.
	 */
	virtual int getPixelLayout() = 0;
	/**
	 *  Produce an array to adjust values of color components to compensate for nonlinear response in an output
	 *  device. See "Transfer Functions" section in PDF Reference for more details. Transfer function is
	 *  applied before dithering.
	 *
	 *  It is currently only implemented for 8 bit grayscale devices, so \a channel must always be zero.
	 *
	 *  Returns an array of 256 8-bit values or NULL if no transfer function needs to be applied.
	 */
	virtual unsigned char * getTransferMap( int channel ) = 0;
	/**
	 *  Produce an array to limit/adjust the pixel values produced by antialiasing to a subset that will not be
	 *  affected by dithering. Use Surface::initDitheringClipMap helper function to initialize your array.
	 *
	 *  It is currently only implemented for 8 bit grayscale devices, so \a channel must always be zero.
	 *
	 *  Returns an array of 256 8-bit values or NULL if no dithering will be done.
	 */
	virtual unsigned char * getDitheringClipMap( int channel ) = 0;
	/**
	 *  Return the number of significant bits in a channel value that device can display distinctly.
	 *  Renderer will apply Bayer dithering so that all other channel values will display as patterns.
	 *  Return 0 if dithering is not needed (or if you apply your own dithering method afterwards).
	 *  Note that all devices that cannot represent at least 64 shades of gray must perform
	 *  dithering.
	 */
	virtual int getDitheringDepth( int channel ) = 0;
	/**
	 *  "Check out" a rectangular raster region to draw onto. Coordinates are in the device (pixel) coordinate
	 *  system (see \ref systems). When this method returns, caller gets back an address and a stride
	 *  value. One can use them to address individual pixels using the following formula:
	 *  <code>
	 *  pixel_addr = addr + stride * (y - yMin) + pixWidth * (x - xMin)
	 *  </code>
	 *  where pixWidth is size of the pixel in bytes (see dpdoc::PixelLayout).
	 *
	 *  After drawing the region must be "checked in" with checkIn() method. Only a single region
	 *  can be checked out from any given Surface at any time.
	 *  \param xMin coordinate of the first pixel line of the raster region 
	 *  \param yMin coordinate of the first pixel column of the raster region 
	 *  \param xMax coordinate of the first line after the last pixel line of raster region 
	 *  \param yMax coordinate of the first column after the last pixel column of raster region 
	 *  \param stride address of the variable where the offset between pixel lines in bytes is returned
	 *  \return address of the first pixel of the raster region.
	 */
	virtual unsigned char * checkOut( int xMin, int yMin, int xMax, int yMax, size_t * stride ) = 0;
	/**
	 *  Finish drawing on the raster surface.
	 *  \param basePtr address returned by checkOut() method.
	 */
	virtual void checkIn( unsigned char * basePtr ) = 0;
	/**
	 *  Utility function to get pixel size in bytes for a given pixel layout
	 */
	static size_t getPixelWidth( int pixelLayout )
	{
		size_t size;
		if( pixelLayout & (dpdoc::PL_BGR|dpdoc::PL_RGB) )
			size = 3;
		else if(pixelLayout & (dpdoc::PL_BGR16|dpdoc::PL_RGB16))
			size = 2;
		else
			size = 1;
		if( pixelLayout & (dpdoc::PL_AC|dpdoc::PL_CA|dpdoc::PL_XC|dpdoc::PL_CX) )
			size++;
		return size;
	}
	/**
	 *  Utility function to initialize array for Surface::getDitheringClipMap method
	 *  for simple dithering (without transfer function).
	 *  \param arr array of 256 8-bit values to initialize
	 *  \param ditheringDepth number of significant bits in a channel value that the device can display distinctly
	 */
	static void initDitheringClipMap( unsigned char * arr, int ditheringDepth );
};

/**
 *  Flags describing types of content elements
 */
enum DisplayElementType
{
	DE_BODY_REGION = 1,		/**< a column in a body (main flow) region */
	DE_SIDE_REGION = 2,		/**< a column in a side (float-like) region */
	DE_TEXT = 4,			/**< a text span */
	DE_EMBED = 8,			/**< embedded content, like an image */
	DE_WALK_EMBED = 0x10	/**< when passed to dpdoc::Renderer::walkScreen forces walking embedded content as well */
};

/**
 *  Interface describing a drawable element
 */
class DisplayElement : public dp::Unknown
{
protected:
	virtual ~DisplayElement() {}

public:
	
	virtual int getInterfaceID() { return IID_DisplayElement; }
	/**
	 *  Get element type, described by dpdoc::DisplayElementType
	 */
	virtual unsigned int getType() = 0;
	/**
	 *  Get element beginning; can be NULL or unexpected for generated content, such as a background image
	 */
	virtual dp::ref<dpdoc::Location> getBeginning() = 0;
	/**
	 *  Get element end; can be NULL or unexpected for generated content, such as a background image
	 */
	virtual dp::ref<dpdoc::Location> getEnd() = 0;
	/**
	 *  Get element drawable area (note: this is not the same as tight bounding box)
	 */
	virtual bool getBox( Rectangle * box ) = 0;
	/**
	 *  Get element drawable area
	 */
	virtual bool getTransform( Matrix * transform ) = 0;
	/**
	 *  Get URL (only for elements of type dpdoc::CE_EMBED)
	 */
	virtual dp::String getEmbedURL() = 0;
};

/**
 *  Interface to get reports about the drawable content displayed on the current screen.
 */
class DisplayHandler : public dp::Unknown
{
protected:
	virtual ~DisplayHandler() {}

public:
	virtual int getInterfaceID() { return IID_DisplayHandler; }
	/**
	 *  Report the beginning of a drawable element of the content
	 */
	virtual void startDisplayElement( DisplayElement * ce ) = 0;
	/**
	 *  Report the end of a drawable element of the content
	 */
	virtual void endDisplayElement( DisplayElement * ce ) = 0;
};

/**
 *  Kinds of events. Returned by Event::getEventKind() method.
 */
enum EventKind
{
	EK_FOCUS,		/**< Focus event. */
	EK_NAVIGATE,	/**< Simple navigation event. */
	EK_MOUSE,		/**< Mouse event; Event object can be cast to MouseEvent */
	EK_TEXT,		/**< Text event; Event object can be cast to TextEvent */
	EK_KEYBOARD		/**< Keyboard event; Event object can be cast to KeyboardEvent */
};

/**
 *  Generic event class. Passed to Renderer::handleEvent to indicate a user interface occuring
 *  in the system. Except for focus events, Event subclasses are used. getEventKind() method
 *  can be used to identify appropriate subclass.
 */
class Event : public dp::Unknown
{
protected:

	/**
	 *  Never called directly. Events are created and destroyed by the host.
	 */
	virtual ~Event() {}

public:

	virtual int getInterfaceID() { return IID_Event; }
	/**
	 *  Query the kind of event. Different kinds of events can be represented by different
	 *  subclasses of Event object.
	 *  \return a value from dpdoc::EventKind enum.
	 */
	virtual int getEventKind() = 0;
	/**
	 *  Event type. Every event kind (see getEventKind()) defines what types of events can occur.
	 *  The same numeric has different meaning for different event kinds. Look for details in
	 *  the dpdoc::Event subclass description. For focus events, a value from dpdoc::FocusEventType
	 *  is used.
	 *  \return event type id.
	 */
	virtual int getEventType() = 0;
	/**
	 *  Renderer can call this method to identify that event has been rejected. Currently only
	 *  focus and navigate events can be rejected.
	 */
	virtual void reject() = 0;
};

/**
 *  Types of mouse events.
 */
enum MouseEventType
{
	MOUSE_MOVE,		/**< Pointing device position changed. */
	MOUSE_UP,		/**< Pointing device button released. */
	MOUSE_DOWN,		/**< Pointing device button pressed. */
	MOUSE_CLICK,	/**< Pointing device button pressed and released in the small time interval and without change in position. */
	MOUSE_ENTER,	/**< Pointing device entered the area occupied by this renderer. */
	MOUSE_EXIT		/**< Pointing device left the area occupied by this renderer. */
};

/**
 *  \brief Keyboard and mouse button state.
 *
 *  Values of the pressed keys/buttons should be ORed together.
 *  \see MouseEvent::getModifiers()
 *  \see KeyboardEvent::getModifiers()
 */
enum ModifierFlags
{
	MF_LEFT = 0x1,		/**< Left (main) button of the pointing device is pressed. */
	MF_RIGHT = 0x2,		/**< Right (context menu) button of the pointing device is pressed. */
	MF_MIDDLE = 0x4,	/**< Middle (wheel) button of the pointing device is pressed. */
	MF_SHIFT = 0x100,	/**< Shift key is pressed. */
	MF_CTRL = 0x200,	/**< Ctrl key is pressed. */
	MF_ALT = 0x400		/**< Alt key is pressed. */
};

/**
 *  Pointing device event. Event::getEventKind() call must return dpdoc::EK_MOUSE.
 */
class MouseEvent : public Event
{
public:

	virtual int getInterfaceID() { return IID_MouseEvent; }
	/**
	 *  Returns button index for up/down/click events. Roughly: 0 is left button, 1 is middle, 2 is right.
	 *  See DOM Level 3 for details.
	 */
	virtual int getButton() = 0;

	/**
	 *  A set of modifiers active when this event was generated.
	 *  \return a set of dpdoc::ModifierFlags values ORed together.
	 */
	virtual unsigned int getModifiers() = 0;
	/**
	 *  Get x coordinate of the pointing device when it caused this event 
	 *  in the device coordinate space.
	 */
	virtual int getX() = 0;
	/**
	 *  Get y coordinate of the pointing device when it caused this event 
	 *  in the device coordinate space.
	 */
	virtual int getY() = 0;
};

/**
 *  Event types for focus events (when event kind is EK_FOCUS)
 */
enum FocusEventType
{
	FOCUS_IN,	/**< This renderer got focus, i.e. keyboard events will be forwarded to it */
	FOCUS_OUT	/**< This renderer lost focus, i.e. keyboard events will no longer be forwarded to it */
};

/**
 *  Event types for navigation events (when event kind is EK_NAVIGATE). Navigation events allow host
 *  to go through interactive elements in the document (navigatable points) and activate them.
 *  This does not necessarily means navigating in the document itself (e.g scrolling), although
 *  document navigation can occur as well. For paged documents, navigation only occurs within a page
 *  (except for NAVIGATE_ACTIVATE which can cause arbitrary action to occur).
 *  Renderer must call Event::reject() method if navigation is not possible (e.g. when
 *  NAVIGATE_NEXT event is sent when current navigatable point is the last one on the page/document).
 *
 *  When the PagingMode is set to is PM_SCROLL_PAGES or PM_SCROLL, navigation occurs through the
 *  entire document. For example, NAVIGATE_FIRST would select the first navigatable point in the
 *  document. A navigation operation while in a scrolling mode does not change the portion of the
 *  document displyed. If that is desired, The navigation operation must be followed by a call to 
 *  Renderer::navigateToHighlight( HT_ACTIVE, 0 ). 
 *
 *  When the PagingMode is set to PM_HARD_PAGES, PM_HARD_PAGES_2UP, or PM_FLOW_PAGES navigation occurs
 *  on the current page. For example, NAVIGATE_FIRST would select the first navigatable point on the
 *  page.
 * 
 *  A relative navigation operation that occurs prior to any calls to NAVIGATE_FIRST or NAVIGATE_LAST
 *  selects the same navigatable point as would have been selected if NAVIGATE_FIRST had been called
 *  prior to the relative operation.
 *
 *  If a relative navigation operation reaches the limit of the page for paged paging modes or the limit
 *  of the document for scrolling paging modes, no navigatable point is selected, but the current
 *  position remains just outside the limit. Additional relative navigation operations in the same
 *  direction do not change the position. A relative navigation operation in the opposite direction
 *  will select the navigatable point closest to the limit.
 */
enum NavigateEventType
{
	NAVIGATE_FIRST,				/**< go to the first navigatable point */
	NAVIGATE_LAST,				/**< go to the last navigatable point */
	NAVIGATE_NEXT,				/**< go to the next navigatable point */
	NAVIGATE_PREVIOUS,			/**< go to the previous navigatable point */
	NAVIGATE_UP,				/**< go to the navigatable point above */
	NAVIGATE_DOWN,				/**< go to the navigatable point below */
	NAVIGATE_LEFT,				/**< go to the navigatable point on the left */
	NAVIGATE_RIGHT,				/**< go to the navigatable point on the right */
	NAVIGATE_OUT,				/**< go to the upper-level navigation context (exit submenu) */
	NAVIGATE_LEAVE,				/**< end navigation, after event is processed there is no current navigatable point */
	NAVIGATE_ACTIVATE			/**< activate action associated with the curent navigatable point (can be entering submenu) */
};

/**
 *  Event type for text events (when event kind is EK_TEXT)
 */
enum TextEventType
{
	TEXT_INPUT	/**< Some text is entered or pasted */
};

/**
 *  Text input event (direct, IME or paste operation).
 *  Event::getEventKind() call must return dpdoc::EK_TEXT.
 */
class TextEvent : public Event
{
public:

	virtual int getInterfaceID() { return IID_TextEvent; }
	/**
	 *  Get entered text as UTF-8 string.
	 */
	virtual dp::String getData() = 0;
};

/**
 *  Types of keyboard events.
 */
enum KeyboardEventType
{
	KEY_DOWN,	/**< corresponds to DOM Level 3 keydown */
	KEY_UP		/**< corresponds to DOM Level 3 keyup */
};

/**
 *  Key location to identify keys which can occur in multiple locations on the keyboard.
 *  Included for compatibility with DOM Level 3.
 */
enum KeyLocation
{
	KEY_LOCATION_STANDARD      = 0x00, /**< default key location */
	KEY_LOCATION_LEFT          = 0x01, /**< "left" key if there are more than one key, e.g. left Shift */
	KEY_LOCATION_RIGHT         = 0x02, /**< "right" key if there are more than one key, e.g. right Shift */
	KEY_LOCATION_NUMPAD        = 0x03  /**< numpad-located key */
};

/**
 *  Keyboard input event. Event::getEventKind() call must return dpdoc::EK_KEYBOARD.
 *  
 */
class KeyboardEvent : public Event
{
public:

	virtual int getInterfaceID() { return IID_KeyboardEvent; }
	/**
	 *  Name of the key that generated this event. See 
	 *  <a href="http://www.w3.org/TR/DOM-Level-3-Events/keyset.html">DOM Level 3</a>
	 *  for key names. Returned string is valid while event object is valid (i.e. for the duration of
	 *  the Renderer::handleEvent() call).
	 *  \return name of the key.
	 */
	virtual dp::String getKeyIdentifier() = 0;
	/**
	 *  A set of modifiers active when this event was generated.
	 *  \return a set of dpdoc::ModifierFlags values ORed together.
	 */
	virtual unsigned int getModifiers() = 0;
	/**
	 *  Key location to identify keys which can occur in multiple locations on the keyboard.
	 *  Included for compatibility with DOM Level 3.
	 *  \return one of the dpdoc::KeyLocation enum values.
	 */
	virtual int getKeyLocation() = 0;

};

}


#endif // DP_DOC_H
