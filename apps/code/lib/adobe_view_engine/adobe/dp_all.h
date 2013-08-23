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
 *  \file dp_all.h
 *
 *  Reader Mobile SDK public interface -- convenience header file to include all
 *  other public headers
 */
#ifndef _DP_ALL_H
#define _DP_ALL_H

/** 
 *  \mainpage Reader Mobile 9.1 SDK Embedding API
 *
 *  <b>ADOBE CONFIDENTIAL</b>
 * 
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
 *
 *  This documentation describes Reader Mobile APIs that can be used to embed EPUB and PDF content into
 *  other applications.
 *
 *  - \ref overview
 *  - \ref objectives
 *  - \ref principles
 *  - \ref commandFile
 *  .
 */
/**
 *  \page overview Reader Mobile API Overview
 *
 *  Reader Mobile SDK APIs can be divided in the following groups:
 *  - core types and utility classes (dp, dptimer namespaces)
 *  - embeddable document parser and renderer (dpdoc namespace)
 *  - I/O support (dpio namespace)
 *  - networking (dpnet namespace)
 *  - cryptography (dpcrypt namespace)
 *  - DRM support (dpdev, dpext, dpdrm namespaces)
 *  - library support (dplib namespace)
 *  .
 */
/**
 *  \page objectives Design Objectives
 *
 *  \section goals Goals
 *  
 *  - <b>Simplicity</b>. Interface should be expressable with simple C++ API, no need to learn any framework.
 *  - <b>High-level</b>. Concentrate on high-level interface, not detailed control.
 *  - <b>Platform-independent</b>. Simplest case (rendering to a bitmap) should not need any platform-specific code.
 *  - <b>Partial rendering</b>
 * . Don't have to wait until whole document is loaded.
 *  - <b>Host owns surface</b>
 * . Hosting app already has its own notion of offscreen and it should be reused.
 *  - <b>Host tells when to draw</b>
 * . Hosting app already has its own drawing logic.
 *  - <b>Partial redraw</b>
 * . Interactivity in the hosting app might require re-rendering only a small portion of screen.
 *  - <b>Navigation within the document</b>
 * . Fragment identifiers, page positions - etc.
 *  - <b>Transparency</b>. Drawing on top of host-provided background allows for both transparncy and blending modes.
 *  - <b>Hit testing</b>
 * . Transparent areas in the content might need to allow clicks to go to the content below.
 *  - <b>Transformations</b>. Content might need to be embedded scaled or rotated.
 *  - <b>Metadata</b>. Title, author, thumbnail, etc.
 *  - <b>TOC</b>. Many documents have table of contents.
 *  - <b>Interactivity</b>. Almost all kinds of content allows at least link embedding.
 *  - <b>Pagination modes</b>
 * . Paged, scrollable, mixed.
 *  - <b>Scale factor</b>
 * . Text (and possibly some art) can be made bigger or smaller (as in browsers).
 *
 *  \section nongoals Non-goals
 *
 *  - <b>Native UI embedding</b>
 * . There is no way to embed native platform UI controls
 *  - <b>Loadable embedding components</b>
 * . Splitting various viewers into DLLs and loading them dynamically is not
 *       a goal (but can be implemented on top of embedding layer).
 */
/**
 *  \page principles Design Principles
 *
 *  \section lifetime Object Lifetime
 *
 *  Hobbes API does not impose strict and general object lifetime rules, but a mimimal
 *  set of conventions has to be obeyed to avoid leaks.
 *  
 *  Hosting side is responsible for releasing instances of a dpdoc::Renderer class when they are no
 *  longer needed. dpdoc::Renderer::release() method should be used for that purpose. After
 *  call to dpdoc::Renderer::release(), dpdoc::Renderer methods can be no longer used. dpdoc::RendererClient
 *  that is provided to the dpdoc::Renderer must exist for the duration of the dpdoc::Renderer lifetime
 *  (i.e. until dpdoc::Renderer::release() returns).
 *
 *  Hosting side is also responsible for releasing instances of a dpdoc::Document class when they are no
 *  longer needed. dpdoc::Document::release() method should be used for that purpose. However, dpdoc::Document
 *  must exist until all dpdoc::Renderer objects that originated from it are released. dpdoc::DocumentClient
 *  that is provided to the dpdoc::Document must exist for the duration of the dpdoc::Document lifetime
 *  (i.e. until last dpdoc::Document::release()/dpdoc::Renderer::release() returns).
 *
 *  Hosting side must \e not release dpdoc::Document and dpdoc::Renderer objects from within host
 *  callbacks (e.g. dpdoc::RendererClient::navigateToURL). Release must always happen from the top level,
 *  when there are no renderer side function frames on stack.
 *
 *  Streams that are returned from dpdoc::DocumentClient::getResourceStream() are owned (directly
 *  or indirectly) by the dpdoc::Document and \e must be released with dpio::Stream::release()
 *  call. dpdoc::Document must release all the streams that it requested before returning from
 *  dpdoc::Document::release() call.
 *
 *  dpdoc::Location and dpdoc::TOCItem objects must be released by the host by calling their release methods.
 *  They must be released before the dpdoc::Document from which they originated is released.
 *
 *  dpdoc::Surface, dpdoc::Event and all Event subclasses must remain valid only for the duration of the calls
 *  to which they are passed as parameters. For instance, dpdoc::Renderer must not attempt
 *  to retain a pointer to the emebd::Surface to which it renders. In many cases it is convenient to
 *  allocate these objects on the stack (as local variables).
 *  
 *  Strings and, in many cases, byte arrays are represented by special objects (dp::String and dp::Data).
 *  These objects take care of the lifetime management automatically.
 *
 *  \section threading Threading Issues
 *
 *  All methods in this interface must be called from a single thread. If threading is involved, it
 *  should be isolated from the interface.
 *
 *  \section unicode String Encoding
 *
 *  All strings (dp::String) are UTF-8. Be careful - they cannot always directly passed to most OS functions
 *  (which on Windows in particular expect platform locale-encoded strings). Only ASCII characters are the
 *  same in UTF-8 and most platform encodings. On Linux and MacOS, UTF-8 has become de-facto standard for
 *  many operations (filesystem access, in particular), but you have to be careful and always check what
 *  character encoding a particular OS and third-party functions assume.
 *
 *  A UTF-16 String class (dp::UTF16String) is provided and can be used on Windows to do UTF-16 conversion
 *  for Win32 Unicode APIs.
 *
 *  \section loading Document Loading
 *
 *  Each document is considered to have an absolute URL from which it is loaded. Host can make up a URL
 *  if no meaningful URL exists, but that can affect loading of the document's external resources
 *  and hyperlinks. Document's URL must be provided to the dpdoc::Document with dpdoc::Document::setURL() method,
 *  at which point dpdoc::Document will start reading the document stream with 
 *  dpdoc::DocumentClient::getResourceStream() method using exactly the same URL as was provided in
 *  the dpdoc::Document::setURL() call. All document's external resources will be loaded using
 *  the same method (called with appropriate URL). Document can (not must) make relative URLs of their
 *  external resources absolute. See dpio::Stream class for discussion of stream reading process.
 *  Document is considered to be loaded when dpdoc::DocumentClient::reportLoadingState() is called with
 *  dpdoc::LS_COMPLETE code.
 *
 *  \section streams Streams
 *
 *  Both main document data stream and resource substreams (e.g images in HTML) are loaded through
 *  dpio::Stream interface (implemented by the host) and its helper dpio::StreamClient (implemented
 *  by the renderer). There are several modes of loading which are controlled by dpio::StreamCapabilities
 *  flags that can be ORed together.
 *
 *  Resource can be requested in both synchronous and asynchronous mode (dpio::SC_SYNCHRONOUS flag). In
 *  synchronous mode, while there may be an initial delay before resource is ready, once resource
 *  arrives all requests are handled synchronously. Specifically, dpio::StreamClient::bytesReady() is
 *  always called before dpio::Stream::requestBytes() returns and dpio::StreamClient::mimeTypeReady() is
 *  called before dpio::Stream::requestInfo() returns. In asynchronous mode, calls to StreamClient
 *  can come either before corresponding Stream method returns or after that.
 *
 *  Resource can be requested with random-access (dpio::SC_BYTE_RANGE) or in stream mode. In stream mode,
 *  Document can only request the whole resource at once (by calling dpio::Stream::requestBytes(0, (size_t)-1)).
 *  In random-access mode, requests can be made for arbitrary portions of the resource.
 *
 *  In asynhronous random-access mode, pending requests to the stream must never overlap. If a range of bytes
 *  is requested, no part of this range can be requested again until that part of the byte range is delivered.
 *  Requests can be serviced in arbitrary order (e.g the order of dpio::StreamClient::bytesReady() calls might be
 *  different from originating dpio::Stream::requestBytes() calls).
 *
 *  In all modes, requests for 4096 bytes or less must be serviced as a whole. Larger requests may be delivered
 *  in a sequence of dpio::StreamClient::bytesReady() calls, but chunk size cannot be less than 1024 bytes
 *  (unless it is the last chunk of the request) and bytes for each request should come in the sequential
 *  order (i.e. increasing chunk offset) without gaps. bytesReady() calls for one request can, though, be
 *  interleaved with bytesReady() calls for other requests.
 *
 *  dpio::Stream and dpio::StreamClient implementors must follow the following rules (and can expect the
 *  other party following the rules as well):
 *  - After a call is made to dpio::Stream::requestInfo(), dpio::StreamClient::mimeTypeReady() <b>will</b>
 *    be called. If content type of the stream is not known, "application/octet-stream" will be passed as
 *    mimeType.
 *  - After a call is made to dpio::Stream::requestInfo() in non-random-access mode
 *    dpio::StreamClient::totalLengthReady() may or may not be called. In random-access mode,
 *    dpio::StreamClient::totalLengthReady() must be called. In all cases it must be called before
 *    dpio::StreamClient::mimeTypeReady().
 *  - If dpio::Stream::requestInfo() is not called, <b>neither</b> dpio::StreamClient::mimeTypeReady() <b>nor</b>
 *    dpio::StreamClient::totalLengthReady() will be called.
 *  - Multiple active streams for the same resource are allowed. They must be different stream objects, since
 *    each will have its own stream receiver.
 *  - dpio::Stream <b>must</b> be valid until a call to dpio::Stream::release() is made.
 *  - dpio::StreamClient <b>must</b> be valid until a call to dpio::Stream::release() is made.
 *  - dpio::StreamClient methods <b>must not</b>
 *  be called after corresponding dpio::Stream's release
 *    has been called (e.g. dpio::StreamClient::bytesReady() must not be called from within the
 *    dpio::Stream::release).
 *  - all non-released streams must be released when dpdoc::Document and all of its dpdoc::Renderer objects are released.
 *  .
 *
 *
 *  \section systems Coordinate Systems
 *
 *  There are two important coordinate systems that are used in the APIs. The first one is document's
 *  logical coordinate system with the origin it the document top left corner, x axis going right
 *  and y axis going down (from the "document's point of view). Values that are related to the document
 *  itself without regard of how it is rendered (e.g. at what scale or resolution) are expressed in that
 *  coordinate system. Only floating-point numbers are used to represent document's logical coordinates.
 *
 *  The other coordinate system is pixel or device coordinate system. That coordinate system is aligned
 *  with the pixel grid. Origin is located at the (0,0) pixel \e corner (this is important for subpixel
 *  rendering).
 *
 *  \image html Transform.gif "Coordinate systems"
 * 
 *  These coordinate systems are related by standard 2D affine transformation expressed as dpdoc::Matrix
 *  (subscript \e p denotes pixel coordinate system and \e d - document coordinate system):
 *
 *  <pre><i>
 *      x<sub>
 * p</sub> = a x<sub>
 * d</sub> + c y<sub>
 * d</sub> + e
 *      y<sub>
 * p</sub> = b x<sub>
 * d</sub> + d y<sub>
 * d</sub> + f
 *  </i></pre>
 *
 *  While it is sufficent to have just one matrix to describe arbitrary transformation, it is more
 *  convenient to split the overall document-to-pixel transformation in two stages. First, logical
 *  document coordinate system is transformed into the renderer coordinate system. This transfomation
 *  is controlled by the <em>navigation matrix</em>
 * . Renderer can modify this matrix in response to
 *  navigation commands (e.g. scrolling the document). Host can control this matrix as well (e.g. in
 *  to implement scrollbars). Next, renderer coordinate system is transformed into the device coordinate
 *  system. This transformation is described by the <em>enviroment matrix</em>
 * . This matrix is always
 *  assigned by the host.
 *
 *  \section navigation Navigation Within the Document
 *
 *  dpdoc::Renderer can provide navigatation within the document in several ways. Scrolling (but also zooming and
 *  rotation to certain view) can be done by manipulating navigation matrix. Navigation to a arbitrary point within
 *  a document is usually doen with dpdoc::Renderer::navigateToLocation(). In addition, there might be some
 *  renderer-specific ways of navigation. Navigation can happen as a result of explicit navigation method call
 *  (such as dpdoc::Renderer::navigateToLocation() or dpdoc::Renderer::nextScreen()) or in response to an event. A
 *  particular point in the document can be captured as dpdoc::Location object. It can be also serialized into a
 *  character string (called <em>bookmark</em>) that can be kept beyond the emebed::Renderer lifetime. Bookmarks
 *  should only use URL-safe characters, so there is no need to URL-encode when using them as fragment identifiers.
 *
 *  See also:
 *  - \ref systems
 *  - dpdoc::Location
 *  - dpdoc::Renderer::navigateToLocation()
 *  - dpdoc::Renderer::setNavigationMatrix()
 *  - dpdoc::Renderer::getNavigationMatrix()
 *  - dpdoc::Renderer::nextScreen()
 *  - dpdoc::Renderer::previousScreen()
 *  - dpdoc::RendererClient::reportInternalNavigation()
 *
 *  \section ranges Document Ranges
 *
 *  Some methods (like search and highlighting) operate on a continuous subset of the document content which 
 *  is represented by the <em>document range</em>
 * . A range is simply two emebd::Location objects, one
 *  representing the beginning of the content and one for the end. The range "includes" its beginning,
 *  but does not "include" its end. So the range that is represented by the beginning of the first screen to
 *  to the beginning of the second screen includes all of the content on the first screen and nothing from the
 *  second.
 *
 *  \section rendering Rendering
 *
 *  When document is at least partially loaded (loading state is at least dpdoc::LS_INCOMPLETE) a
 *  host can create an dpdoc::Renderer object(s) to render it.
 *  Host controls both \e when the rendering occurs, \e where document
 *  it rendered (i.e., offscreen memory) and which portion of the document is rendered (e.g., host
 *  can request only a portion of the viewport to be rendered).
 *  It is also recommended (but not required) that host manages the background. Host should examine Renderer capabilities
 *  and if dpdoc::RC_BLEND capability is present (which is true for both PDF and EPUB), set dpdoc::PF_TRANSPARENT
 *  flag using dpdoc::Renderer::setPlayMode API. This way Renderer will draw on top of the host-supplied background
 *  without clearing. In most cases, this is the most efficient mode of operation.
 *  If dpdoc::RC_BLEND is not present or if dpdoc::PF_TRANSPARENT flags is not set, Renderer will
 *  clear the background to white before drawing. Note that only areas that Renderer checked out from the dpdoc::Surface
 *  will be cleared.
 *
 *  dpdoc::Renderer can request the host to repaint rectangular portions of the document screen. Host can
 *  ignore or delay such requests (e.g. when requested area is not visible), but it must take into
 *  account that previous rendering of that region is no longer valid. 
 *
 *  See:
 *  - dpdoc::Renderer::paint()
 *  - dpdoc::Renderer::getCapabilities()
 *  - dpdoc::RendererClient::requestRepaint()
 *  - dpdoc::DocumentClient::reportLoadingState()
 *  .
 *
 *  \section paging Paged Document Model
 *
 *  Humans typically look at the media of the finite dimensions: a computer screen or a piece of paper.
 *  But a document can also be viewed in its entirety on one large "chunk" of the media. There are
 *  two ways to resolve this basic conflict. The first one is to use \e scrolling: logically
 *  a document is layed out on an infinitely large media, but only a finite portion of it can be seen at
 *  a time. Navigation matrix can be used to implement this approach. The other way is to do "paging":
 *  use multiple "chunks" (or <em>screens</em>) of the media and flow document from one to the other,
 *  using as many screens as neccessary. Note that in general screens are different from pages that may
 *  or may not be present in the document itself.
 *
 *  \subsection paging_modes Paging Modes
 *
 *  There are five possible paging modes (see dpdoc::PagingMode):
 *  - scrolling (no screens)
 *  - scrolling with visible page breaks
 *  - hard paging, when screen breaks are the same as pages in the original document
 *  - 2-up hard paging, when two pages from the original document are displayed in a single screen 
 *  - soft paging, when screen breaks are determined at run time
 *  .
 *
 *  Soft paging is somewhat of an experimental concept.
 *
 *  See:
 *  - \ref page_numbers
 *  - dpdoc::Renderer::getPagingMode()
 *  - dpdoc::Renderer::setPagingMode()
 *  - dpdoc::PagingMode
 *  .
 *
 *  \subsection page_numbers Page Positions and Total Page Count
 *
 *  Pre-paginated documents (like PDF) have well-defined notions of total page count and page number. Documents
 *  that are not inherently pre-paginated may also define sections that represent pages. However, if
 *  a document is not pre-paginated (e.g. XHTML), pages are defined by an algorithm. The algorithm determines
 *  the number of pages in each chapter from the size of the unprocessed data for that chapter. When the chapter
 *  is actually layed out, the chapter is broken evenly into that number of pages.
 *
 *  A page position real-number that consists of a integer page number and a fraction describing a geomtric position
 *  on the page. Since the page number is defined by the document or by a algorithm, it is stable even if the lay out
 *  geometry is changed. The fractional part is dependent of the lay out geometry. To save a point in a document
 *  through changes in the lay out, a page position should be converted to a Location.
 *
 *  See:
 *  - dpdoc::Document::getPageCount()
 *  - dpdoc::Document::getLocationFromPagePosition()
 *  - dpdoc::Location::getPagePosition()
 *  .
 *
 *  \subsection page_names Page Names
 *
 *  Pre-paginated documents and other documents where pages are specified may have names assigned to the pages.
 *  Where page positions are number starting with 0 for the first page in the document, pages names are strings.
 *  In many cases, these srings contain numbers, but they could also contain roman numbers or other other
 *  characters. Even when the strings contain numbers, they may not be the same as the integer portion of the
 *  page position. For example, a book may have a table of contents that uses roman numerals for page names and
 *  starts with a page named "1" many pages into the book.
 *
 *  See:
 *  - dpdoc::Document::getPageName()
 *  - dpdoc::Document::getPagePosition()
 *  .
 *  \section background Background Processing
 *
 *  In many cases dpdoc::Document and associated dpdoc::Renderer need to perform some sort of background processing.
 *  If background processing is required, a Timer object is created and a timer scheduled to fire as soon as 
 *  possible or after an interval.
 *  
 *  When background processing is performed by a dpdoc::Document or dpdoc::Renderer,
 *  dpdoc::DocumentClient::canContinueProcessing() will be called periodically with
 *  dpdoc::PK_BACKGROUND code to check if background processing should
 *  be interrupted and control returned from the dpdoc::Document.
 *
 *  See:
 *  - dptimer::Timer
 *  - dpdoc::DocumentClient::canContinueProcessing()
 *  .
 *
 *  \section highlighting Highlighting
 *
 *  A continuous subset of the document's content can be highlighted. Currently only four types of
 *  highlighting are supported: selection, focus, annotation, and selected annotation. Selection
 *  represents text which was selected or was found. Focus can represent a link which can be
 *  activated. Multiple highlights can exist for every type except focus.
 *
 *  See:
 *	- dpdoc::Renderer::addHighlight()
 *	- dpdoc::Renderer::navigateToHighlight()
 *	- dpdoc::Renderer::getHighlight()
 *	- dpdoc::Renderer::setHighlightColor()
 *	- dpdoc::Renderer::getHighlightColor()
 *	- dpdoc::Renderer::removeHighlight()
 *	- dpdoc::Renderer::removeAllHighlights()
 *	- dpdoc::RendererClient::reportHighlightChange()
 *  .
 *
 *  \section annotations Annotation file format
 *
 *  For interoperability, all application must store annotations and bookmarks using common
 *  XML format and store them using URL obtained by dplib::ContentRecord::getAnnotationURL API.
 *  Here is a sample annotation file.
 *
 *  <pre>
 * &lt;annotationSet xmlns:xhtml="http://www.w3.org/1999/xhtml" 
 *    xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns="http://ns.adobe.com/digitaleditions/annotations"&gt;
 *   &lt;publication&gt;
 *     &lt;dc:identifier&gt;urn:uuid:6322dd08-ea15-11db-8314-0800200c9a66&lt;/dc:identifier&gt;
 *     &lt;dc:title&gt;Alice's Adventures in Wonderland&lt;/dc:title&gt;
 *     &lt;dc:creator&gt;Lewis Carroll&lt;/dc:creator&gt;
 *   &lt;/publication&gt;
 *   &lt;annotation&gt;
 *     &lt;dc:identifier&gt;urn:uuid:4E4A26CA-3997-3C9D-C230-CC0B14FD3BD2&lt;/dc:identifier&gt;
 *     &lt;dc:date&gt;2009-11-07T00:28:48-00:00&lt;/dc:date&gt;
 *     &lt;dc:creator&gt;urn:uuid:ab9ab128-f7cb-4a25-87ca-3fd800813182&lt;/dc:creator&gt;
 *     &lt;target&gt;
 *       &lt;fragment start="OEBPS/chapter01.xhtml\#point(/1/4/4/2:3)" end="OEBPS/chapter01.xhtml\#point(/1/4/4/2:92)"/&gt;
 *     &lt;/target&gt;
 *     &lt;content&gt;
 *       &lt;dc:date&gt;2009-11-07T00:28:56-00:00&lt;/dc:date&gt;
 *       &lt;text&gt;sample annotation&lt;/text&gt;
 *     &lt;/content&gt;
 *   &lt;/annotation&gt;
 * &lt;/annotationSet&gt;
 *  </pre>
 *
 *  Notes:
 *  - use of namespaces is mandatory
 *  - publication element contains metadata of the publication to which annotation is applied
 *  - annotation element contains information about annotation or bookmark
 *  - fragment element contains bookmarks for the beginning and end of the annotation
 *  - content element contains annotation itself and date it was modified
 *  .
 *
 *  \section errors Error Handling
 *
 * Errors and warnings are divided into two types - state and process. State errors and warnings
 * are non-transient. They remain for the duration of the life of the object they pertain
 * to or until some state change occurs. For example, a PDF file that contains syntax errors
 * has a state error. Process errors are transient errors. For example, requesting more pages be
 * printed than allowed by DRM is a process error.
 * 
 * Errors and warnings can apply to the application as a whole, a document, or a specific view
 * of a document. A document can have an error associated with it that prevents it from being
 * opened. These are referred to as fatal errors and occur due to problems such as errors
 * in downloading or DRM. If a fatal error is encountered in a document, this information is
 * saved in the manifest on disk and displayed in library mode. The fatal error in the manifest
 * is cleared if the document or DRM data is reloaded. If it is determined the problem no
 * longer exists during ADE startup or any other time, the error is cleared.
 * 
 * The following table defines the types of errors that occur in ADE.
 * 
 * <table>
 * <tr><th>Type</th><th>Code</th><th>Definition</th></tr>
 * <tr><td>message</td><td>M</td><td>Informative message</td>
 * </tr>
 * <tr><td>warning</td><td>W</td><td>Problem encountered that does not cause missing information</td>
 * </tr>
 * <tr><td>error</td><td>E</td><td>Problem encountered that may cause missing information</td>
 * </tr>
 * <tr><td>fatal error</td>
 * <td>F</td><td>Problem encountered that prevents a document from being opened</td>
 * </tr>
 * </table>
 *
 *  \subsection errorString Error String
 *
 * Every error is specified by an error string. An error string consists of a an error mnemonic
 * followed by a space and a space separated set of parameters. If the parameters contain
 * spaces or other special characters they must be percent sign encoded in the same way as a
 * URL. The error mnemonic consists of the following fields:
 * 
 * <table>
 * <tr><th>field</th><th>length</th><th>allowable characters</th>
 * <th>meaning</th></tr>
 * <tr><td>code</td><td>1</td><td>F, E, W, or M</td>
 * <td>F for fatal error, E for error, W for warning, M for message</td>
 * </tr>
 * <tr><td>separator</td><td>1</td><td>underscore</td><td>separator</td></tr>
 * <tr><td>details</td><td>1 or more</td>
 * <td>alphanumeric or underscore</td>
 * <td>Identifies the specific error, warning,or message</td>
 * </tr>              
 * </table>
 *
 * The error mnemonic is not a string that can be presented to end users. It is used instead of
 * a numeric error code to allow easy debugging. It must be mapped into a localized string
 * before use in the UI. In many cases, information from the parameters are merged into the
 * localized string as well.
 * 
 * The details field of the error mnemonic begins with a prefix that indicates the area of ADE
 * where the error occurred. This is followed by an underscore as a separator from the remainder
 * of the field. This reduces the likelihood of collisions between different error mnemonics.
 * Additional prefixes for sub-areas with in an area are used when desired.
 * Here are two examples of what error strings might look like:
 * 
 * - E_PKG_OCF_PARSE_ERROR 22 5
 * - F_DRM_FULFILL 19 5 32 6
 * .
 * 
 *  \subsection errorRep Methods and Classes that are used to report errors
 *
 *  - dp::ErrorList
 *  - dp::Callback::reportError
 *  - dpdoc::DocumentClient::reportDocumentError
 *  - dpdoc::RendererClient::reportRendererError
 *  - dpdoc::Document::getErrorList
 *  - dpdoc::DocumentClient::reportErrorListChange
 *  - dpdev::DeviceListener::reportError
 *  - dpdrm::DRMProcessorClient::reportWorkflowError
 *  - dpio::Stream::reportWriteError
 *  - dpio::StreamClient::reportError
 *  .
 *
 *  \subsection password Password-protected documents
 *
 *  RMSDK can open password-protected documents. When password-based document is encountered (after dpdoc::Document::setURL
 *  method is called), dpdoc::DocumentClient::requestDocumentPassword callback is invoked. Application responsibility is
 *  to obtain the password from the user and call dpdoc::Document::setDocumentPassword. Note that passwords must be entered
 *  by the end user at the document open time and must not be stored by the RMSDK licensee application.
 *
 *  When dpdoc::DocumentClient::requestDocumentPassword call is made, document loading process is suspended. Only a call to
 *  dpdoc::Document::setDocumentPassword can resume the loading. dpdoc::Document::setDocumentPassword call can, but does not
 *  have to happen before the return from dpdoc::DocumentClient::requestDocumentPassword. If incorrect password is supplied,
 *  another dpdoc::DocumentClient::requestDocumentPassword call will be made. If null password is given to
 *  dpdoc::DocumentClient::requestDocumentPassword, document loading will complete with failure (dpdoc::LS_ERROR).
 */
/**
 *  \page commandFile Command File Capabilities in book2png
 *
 * The book2png command processor has no looping or branching capabilities. The intent is to have each
 * command map to a single call over the embed interface to the extent possible. Each command is on a separate
 * line of the command file. The command file is in UTF-8 encoding. However, all characters outside of quoted
 * strings are in the ASCII subset of Unicode (range 0x0 to 0x7F). It is invoked with the
 * "-command-line <command file name>"  option on the line calling book2png. To read commands
 * from stdin, a "-" may be used as the command file name.
 *
 * \section commandLine Command Line
 * A command line consists of a command followed by 0 to 7 parameters. The command consists of
 * up to 32 ASCII characters at the start of the line. Whitespace is used as the separator between
 * the command and the first parameter and between each pair of parameters. Extra whitespace at the
 * end of the line is ignored, but no other characters are allowed on the line after the final parameter.
 * Whitespace at the start of the line is also ignored. Whitespace consists of any combination of spaces
 * (Unicode 0x20) and tabs (Unicode 0x09). A command line starting with a # (Unicode 0x23) is a comment
 * line and is ignored. Blank command lines are also ignored. 
 *
 * \section commandParameters Parameters
 * The parameters may be integers, real numbers, quoted strings, or locations. Integers, real numbers,
 * and quoted strings used for input parameters may be constants or calculated values. Locations and all
 * output parameters must specify a calculated value.
 *
 * Constant integers consist of a optional sign followed by up to 10 decimal digits. A integer must fit
 * in a 32 bit 2s complement number.
 *
 * Constant real numbers consist of an optional sign, up to twelve significant digits, and an optional
 * decimal point at any place in the number. Leading or trailing 0s may be used if the decimal point must
 * be outside the significant digits. A real number must be representable as a 64 bit IEEE floating point
 * number.
 *
 * Constant quoted strings consist of up to 127 characters within a pair of double-quotes (" - Unicode 0x22).
 * A backslash ( \\ - Unicode 0x5C) is used to as an escape character. The only valid escape sequences are a
 * backslash followed by a double-quote (\\") or a pair of backslashes (\\\\).
 *
 * Calculated values are where outputs from commands are placed and can be used as inputs to later commands.
 * An calculated value is represented by a single letter followed by a single digit. The letters used for
 * integer, real number, quoted string, location, matrix, or TOCItem calculated values are I, R, S, L, M, T
 * respectively. Only 10 calculated values of each type are available at a time. Calculated values can be reused.
 *
 * \section commandsAndOptions Interaction with book2png Options
 * The following options may be used in conjunction with -command-file on the line calling book2png.
 * They have the usual behavior.
 *
 * - -verbose
 * - -type
 * - -out-page
 * - -out-html
 * - -no-html
 * - -no-png
 * - -width
 * - -height
 * - -scale
 * - -margin-vert
 * - -margin-hor
 * - -font-size
 * - -dpi
 * - -query-meta
 * - -query-pages
 * - -query-ccount
 * - -query-bookmark
 * - -query-dimensions
 * - -query-toc
 * - -query-page
 * - -query-coff
 * - -reflow
 * - -version
 * - -monochrome
 * - -alpha
 * - -license
 * - -activation
 * - -write
 * - -time
 * - -time-detail
 * .
 *
 * The following  options on the line calling book2png are incompatible with -command-file and will
 * cause an error to occur. 
 *
 * - -go-to
 * - -go-to-end
 * - -find
 * - -page-start
 * - -page-count
 * - -coff-start
 * - -render pages
 * - -render thumb
 * - -render no
 * - -run
 * - -hit-test
 * - -link-info
 * - -commands
 * - -adobeid
 * - -password
 * - -fulfill
 * - -return-loan
 * - -out-pdf
 * - -fit
 * .
 *
 * \section commandCanContinue canContinueProcessing and abortCount
 * A number commands take abortCount parameter. This parameter can be used to abort the processing of that
 * command by returning false canContinueProcessing callback. This is done by counting the number of calls
 * to canContinueProcessing with any foreground processing kind codes (any code, except for PK_BACKGROUND).
 * Normally this parameter should be set to 0, in that case canContinueProcessing API will always returns
 * true. However if a positive number is given then after a prescribed number of calls this function starts
 * to return false, thereby requesting an operation to stop prematurely. After an operation is aborted the
 * document and renderer can be in inconsistent state, e.g. if navigation is aborted, renderer may have
 * navigated or not navigated to the requested location.
 *
 * In future releases of SDK you may see different number of calls to canContinueProcessing for exactly
 * the same script, therefore the counts passed to the commands may need to be adjusted.
 *
 * \section commandPerformance Performance measurements
 * Two command line options are available to output performance data. The -time-detail shows
 * the times for every call through the embed interface. The -time option will produce a time summary.
 * The time summary shows the setup time, the total time, the QETimer time, and the time from any other
 * time accumulators that were used. A time accumulator is created with the setTimeAccumulator command.
 *
 * The setTimeAccumulator command takes a single string parameter. Once a setTimeAccumulator command
 * is executed all time in hobbes is added into a time accumulators associated with the name in the
 * string parameter until another setTimeAccumulator command is issued. Another setTimeAccumulator
 * command may be executed to redirect the time into another named time accumulator or into no time
 * accumulator if an empty string is passed. The time in hobbes is always included in the total time
 * whether it is added into a time accumulator or not. Time may be put into a named time accumulator
 * for a few commands, put into another time accumulator for a few commands, and then additional time
 * may be added into the first accumulator again for a few more commands simply by calling
 * setTimeAccumulator with the same name again.
 *
 * At the end of the run, the times in all the time accumulators are printed out in addition to the total
 * time and the setup time. As mentioned before, the time accumulator named "QETimer" is special in that
 * it is always printed. The QETimer time is the same as the total time unless a setTimeAccumulator
 * command with "QETimer" as the parameter is executed. The setup time is time spent in hobbes prior
 * to the first command in the command file. The total time is the time spent in hobbes while processing
 * all the commands in the command file. The total time does not include the setup time. 
 *
 * \section commands Command Set
 *
 * \commanddef{addActiveHighlight, startLocation endLocation index}
 *
 * \commandparams
 * \commandparam{startLocation,input,location}
 * \commandparam{endLocation,input,location}
 * \commandparam{index,output,integer}
 *
 *  Executes the dpdoc::Renderer::addHighlight API passing dpdoc::HT_ACTIVE as the highlightType.
 *  The locations specified by the startLocation and endLocation parameters are passed
 *  in as the start and end respectively. Returns the highlight index in the place specified
 *  by the index parameter.
 *
 * \commanddef{addAnnotationHighlight, startLocation endLocation index}
 *
 * \commandparams
 * \commandparam{startLocation,input,location}
 * \commandparam{endLocation,input,location}
 * \commandparam{index,output,integer}
 * 
 * Executes the dpdoc::Renderer::addHighlight API passing dpdoc::HT_ANNOTATION as the highlightType. The locations specified by the startLocation and endLocation parameters are passed in as the start and end respectively. Returns the highlight index in the place specified by the index parameter.
 * 
 * \commanddef{addPass, operatorURL username password}
 *
 * \commandparams
 * \commandparam{operatorURL,input,string}
 * \commandparam{username,input,string}
 * \commandparam{password,output,string}
 * 
 * Computes a passhash with the given username and password using dpdrm::DRMProcessor::calculatePasshash and adds it to the operatorURL using dpdrm::DRMProccessor::addPasshash.  The result of addPasshash will be displayed on stdout.
 *
 * \commanddef{addSelectHighlight, startLocation endLocation index}
 *
 * \commandparams
 * \commandparam{startLocation,input,location}
 * \commandparam{endLocation,input,location}
 * \commandparam{index,output,integer}
 *  
 * Executes the dpdoc::Renderer::addHighlight API passing dpdoc::HT_SELECTION as the highlightType. The locations specified by the startLocation and endLocation parameters are passed in as the start and end respectively. Returns the highlight index in the place specified by the index parameter.
 * 
 * \commanddef{calcFitMatrix, docWidth docHeight desWidth desHeight ctm}
 *
 * \commandparams
 * \commandparam{docWidth,input,real}
 * \commandparam{docHeight,input,real}
 * \commandparam{desWidth,input,real}
 * \commandparam{desHeight,input,real}
 * \commandparam{ctm,matrix,output}
 *  
 * Utility command to calculate transformation matrix ctm that can be used to draw the document with docWidth docHeight dimensions into a surface that has dimensions desWidth and desHeight
 * 
 * \commanddef{calcZoomMatrix, docXMin docYMin docXMax docYMax desWidth desHeight ctm}
 *
 * \commandparams
 * \commandparam{docXMin,input,real}
 * \commandparam{docYMin,input,real}
 * \commandparam{docXMax,input,real}
 * \commandparam{docYMax,input,real}
 * \commandparam{desWidth,input,real}
 * \commandparam{desHeight,input,real}
 * \commandparam{ctm,matrix,output}
 *  
 * Utility command to calculate transformation matrix ctm that can be used to draw a portion of
 * a page defined by xMin, yMin, xMax, and yMax into a surface that has dimensions desWidth and
 * desHeight
 * 
 * \commanddef{ceil, realNumber intNumber}
 *
 * \commandparams
 * \commandparam{realNumber,input,real}
 * \commandparam{intNumber,output,integer}
 * 
 * \commanddef{clearErrorList}
 *
 * \commandparams
 *  
 * Executes the dp::ErrorList::clear API on the current ErrorList. This command may only be used if getErrorList was the last command to access the Document or Renderer.
 *  
 * Untility command to convert real to integer rounding up
 * 
 * \commanddef{cnvtBookmkToLoc, bookmark location}
 *
 * \commandparams
 * \commandparam{bookmark,input,string}
 * \commandparam{location,output,location}
 *  
 * Executes the dpdoc::Document::getLocationFromBookmark API passing in the value specified by
 * the bookmark parameter and placing the result in the calculated value specified by the location parameter.
 * 
 * \commanddef{cnvtLocToBookmk, location bookmark}
 *
 * \commandparams
 * \commandparam{location,input,location}
 * \commandparam{bookmark,output,string}
 *  
 * Executes the dpdoc::Location::getBookmark API on the location specified by the location
 * parameter and places the result in the calculated value specified by the bookmark parameter.
 * 
 * \commanddef{compareLocations, first second}
 *
 * \commandparams
 * \commandparam{first,input,integer}
 * \commandparam{second,output,string}
 *  
 * Executes the Location::compare API on the Location specified by the first parameter passing
 * in the second parameter as the parameter to Location::compare. Outputs a message to stdout
 * indicating how the locations compare and if the result is precise.
 * 
 * \commanddef{contentIteratorGetCurrentPosition, result}
 *
 * \commandparams
 * \commandparam{result,output,Location}
 *  
 * Executes the ContentIterator::getCurrentPosition API on the currently saved content iterator.
 * The Location returned is put in the place specified by the result parameter.
 * 
 * \commanddef{contentIteratorNext, flags abortCount result}
 *
 * \commandparams
 * \commandparam{flags,input,integer}
 * \commandparam{abortCount,input,integer}
 * \commandparam{result,output,string}
 *  
 * Executes the ContentIterator::next API on the currently saved content iterator. The value from
 * the flags parameter are passed through the API as the flags. See notes for abortCount parameter
 * description.
 * 
 * \commanddef{contentIteratorPrevious, flags abortCount result}
 *
 * \commandparams
 * \commandparam{flags,input,integer}
 * \commandparam{abortCount,input,integer}
 * \commandparam{result,output,string}
 *  
 * Executes the ContentIterator::previous API on the currently saved content iterator. The value from
 * the flags parameter are passed through the API as the flags. See notes for abortCount parameter
 * description.
 * 
 * \commanddef{contentRecordAddTag}
 *
 * \commandparams
 * none
 *
 * Calls dplib::ContentRecord::addTag API on the current content record using current
 * content tag as a parameter.
 *
 * \commanddef{contentRecordAnnotationChangeNotify}
 *
 * \commandparams
 * none
 *
 * Calls dplib::ContentRecord::annotationChangeNotify API on the current content record.
 *
 * \commanddef{contentRecordGetAnnotationURL, url}
 *
 * \commandparams
 * \commandparam{url,output,string}
 *
 * Calls dplib::ContentRecord::getAnnotationURL API on the current content record and
 * returns result in \a url.
 *
 * \commanddef{contentRecordGetContentURL, url}
 *
 * \commandparams
 * \commandparam{url,output,string}
 *
 * Calls dplib::ContentRecord::getContentURL API on the current content record and
 * returns result in \a url.
 *
 * \commanddef{contentRecordGetCreationTime, time}
 *
 * \commandparams
 * \commandparam{time,output,string}
 *
 * Calls dplib::ContentRecord::getCreationTime API on the current content record and
 * returns result in \a time.
 *
 * \commanddef{contentRecordGetLastReadBookmark, bookmark}
 *
 * \commandparams
 * \commandparam{bookmark,output,string}
 *
 * Calls dplib::ContentRecord::getLastReadBookmark API on the current content record and
 * returns result in \a bookmark.
 *
 * \commanddef{contentRecordGetLastReadTime, time}
 *
 * \commandparams
 * \commandparam{time,output,string}
 *
 * Calls dplib::ContentRecord::getLastReadTime API on the current content record and
 * returns result in \a time.
 *
 * \commanddef{contentRecordGetLibrary}
 *
 * \commandparams
 * none
 *
 * Executes dplib::ContentRecord::getLibrary API on the current content record and saves
 * the result as current library
 *
 * \commanddef{contentRecordGetMetadata, name value}
 *
 * \commandparams
 * \commandparam{name,input,string}
 * \commandparam{value,output,string}
 *
 * Executes dplib::ContentRecord::getMetadata API on the current content record, passing
 * \a name as a parameter and stores the result in \a value.
 *
 * \commanddef{contentRecordGetPrivateData, ns name value}
 *
 * \commandparams
 * \commandparam{ns,input,string}
 * \commandparam{name,input,string}
 * \commandparam{value,output,string}
 *
 * Executes dplib::ContentRecord::getPrivateData API on the current content record, passing
 * \a ns and \a name as parameters and stores the result in \a value.
 *
 * \commanddef{contentRecordGetTags}
 *
 * \commandparams
 * none
 *
 * Executes dplib::ContentRecord::getTags API on the current content record and stores
 * the result in the current tag list.
 *
 * \commanddef{contentRecordGetThumbnailURL, url}
 *
 * \commandparams
 * \commandparam{url,output,string}
 *
 * Calls dplib::ContentRecord::getThumbnailURL API on the current content record and
 * returns result in \a url.
 *
 * \commanddef{contentRecordIsTaggedBy, flag}
 *
 * \commandparams
 * \commandparam{flag,output,integer}
 *
 * Executes dplib::ContentRecord::isTaggedBy API on the current content record and stores
 * 1 if result true and 0 otherwise in \a flag.
 *
 * \commanddef{contentRecordRemoveTag}
 *
 * \commandparams
 * none
 *
 * Executes dplib::ContentRecord::getPrivateData API on the current content record, passing
 * current tag as a parameter.
 *
 * \commanddef{contentRecordSetLastReadBookmark, bookmark}
 *
 * \commandparams
 * \commandparam{bookmark,input,string}
 *
 * Executes dplib::ContentRecord::setLastReadBookmark API on the current content record, passing
 * \a bookmark as a parameter.
 *
 * \commanddef{contentRecordSetMetadata, name value}
 *
 * \commandparams
 * \commandparam{name,input,string}
 * \commandparam{value,input,string}
 *
 * Executes dplib::ContentRecord::setMetadata API on the current content record, passing
 * \a name and \a value as parameters.
 *
 * \commanddef{contentRecordSetPrivateData, ns name value}
 *
 * \commandparams
 * \commandparam{ns,input,string}
 * \commandparam{name,input,string}
 * \commandparam{value,input,string}
 *
 * Executes dplib::ContentRecord::setPrivateData API on the current content record, passing
 * \a ns, \a name and \a value as parameters.
 *
 * \commanddef{contentRecordsGetContentRecord, index}
 *
 * \commandparams
 * \commandparam{name,input,integer}
 *
 * Executes dp::ref::operator[] API on the current content record list, passing
 * \a index as a parameter.
 *
 * \commanddef{contentRecordsLength, len}
 *
 * \commandparams
 * \commandparam{len,input,output}
 *
 * Executes dp::ref::length API on the current content record list, storing the result
 * in \a len.
 *
 * \commanddef{contentRecordThumbnailChangeNotify}
 *
 * \commandparams
 * none
 *
 * Executes dplib::ContentRecord::thumbnailChange API on the current content record.
 *
 * \commanddef{contentTagGetLibrary}
 *
 * \commandparams
 * none
 *
 * Executes dplib::ContentRecord::getLibrary API on the current tag and saves
 * the result as current library
 *
 * \commanddef{contentTagGetMetadata, name value}
 *
 * \commandparams
 * \commandparam{name,input,string}
 * \commandparam{value,output,string}
 *
 * Executes dplib::ContentTag::getMetadata API on the current tag, passing
 * \a name as a parameter and stores the result in \a value.
 *
 * \commanddef{contentRecordGetPrivateData, ns name value}
 *
 * \commandparams
 * \commandparam{ns,input,string}
 * \commandparam{name,input,string}
 * \commandparam{value,output,string}
 *
 * Executes dplib::ContentTag::getPrivateData API on the current tag, passing
 * \a ns and \a name as parameters and stores the result in \a value.
 *
 * \commanddef{contentTagGetParent}
 *
 * \commandparams
 * none
 *
 * Executes dplib::ContentTag::getParent API on the current tag and stores the
 * result as the current tag.
 *
 * \commanddef{contentTagGetTagID, tagid}
 *
 * \commandparams
 * \commandparam{tagid,output,string}
 *
 * Executes dplib::ContentTag::getTagID API on the current tag and stores the
 * result in \a tagid.
 *
 * \commanddef{contentTagSetMetadata, name value}
 *
 * \commandparams
 * \commandparam{name,input,string}
 * \commandparam{value,input,string}
 *
 * Executes dplib::ContentTag::setMetadata API on the current tag, passing
 * \a name and \a value as parameters.
 *
 * \commanddef{contentTagSetPrivateData, ns name value}
 *
 * \commandparams
 * \commandparam{ns,input,string}
 * \commandparam{name,input,string}
 * \commandparam{value,input,string}
 *
 * Executes dplib::ContentTag::setPrivateData API on the current tag, passing
 * \a ns, \a name and \a value as parameters.
 *
 * \commanddef{contentTagsGetContentTag, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *
 * Executes dp::ref::operator[] API on the current tag list, passing
 * \a index as a parameter.
 *
 * \commanddef{contentTagsLength, len}
 *
 * \commandparams
 * \commandparam{len,output,integer}
 *
 * Executes dp::ref::length API on the current tag list, storing the result
 * in \a len.
 *
 * \commanddef{copyInteger, integerIn integerOut}
 *
 * \commandparams
 * \commandparam{integerIn,input,integer}
 * \commandparam{integerOut,output,integer}
 *
 * Copy the integer value from integerIn to integerOut. integerIn may be a constant.
 *
 * \commanddef{copyReal, realIn realOut}
 *
 * \commandparams
 * \commandparam{realIn,input,real}
 * \commandparam{realOut,output,real}
 *
 * Copy the real value from realIn to realOut. realIn may be a constant.
 *
 * \commanddef{copyString, stringIn stringOut}
 *
 * \commandparams
 * \commandparam{stringIn,input,string}
 * \commandparam{stringOut,output,string}
 *
 * Copy the string from stringIn into stringOut. stringIn may be a constant.
 *
 * \commanddef{createDevice, provider index}
 *
 * \commandparams
 * \commandparam{provider,input,string}
 * \commandparam{index,input,integer}
 *  
 * Finds the provider with the identifier given by \a provider by looping through all device providers
 * using dpdev::DeviceProvider::getProvider method and comparing given provider identifier with the result
 * of dpdev::DeviceProvider::getIdentifier method. When provider is found gets the device with the index
 * given by \a index using dpdev::DeviceProvider::getDevice method and stores that device as the current
 * device.
 * 
 * \commanddef{createDRMProcessor, provider index}
 *
 * \commandparams
 * \commandparam{provider,input,string}
 * \commandparam{index,input,integer}
 *  
 * Works in the same way as createDevice to find the device specified by \a provider and \a index, 
 * creates a DRMProcessor for this device and stores that processor as the current processor
 * 
 * \commanddef{createDRMWorkflowFlags, flagString workflows}
 *
 * \commandparams
 * \commandparam{flags,output,integer}
 * \commandparam{workfows,input,string}
 *  
 * Creates the flags needed for dpdrm::DRMProcessor::initWorkflows(), 
 * dpdrm::DRMProcessor::initAdobeIDWorkflow(), dpdrm::DRMProcessor::initJoinAccountsWorkflow()
 * and dpdrm::DRMProcessor::startWorkflows() functions. This does not call any embed interface APIs.
 * The wokflows parameter contains space separated items from the enum dpdrm::DRMWorkflows 
 * (e.g. "DW_AUTH_SIGN_IN DW_ACTIVATE"). The items can be in any order.
 * 
 * \commanddef{createLibrary}
 *
 * \commandparams
 * none
 *
 * Executes dplib::Library::getPartitionLibrary method using current partition as a parameter and
 * stores the result as the current library.
 *
 * \commanddef{createMatrix, a b c d e f matrix}
 *
 * \commandparams
 * \commandparam{a,input,real}
 * \commandparam{b,input,real}
 * \commandparam{c,input,real}
 * \commandparam{d,input,real}
 * \commandparam{e,input,real}
 * \commandparam{f,input,real}
 * \commandparam{matrix,output,matrix}
 *  
 * Utility function to create a transformation matrix 
 * 
 * \commanddef{createPartition, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *
 * Executes dpdev::Device::getPartition method using current device and index given by
 * the \a index parameter. Stores the result as the current partition.
 *
 * \commanddef{createSearchFlags, flagString searchFlags}
 *
 * \commandparams
 * \commandparam{flagString,input,string}
 * \commandparam{searchFlags,output,integrer}
 *  
 * Creates the searchFlags needed for findText. This does not call any embed interface APIs.
 * The flagString parameter contains space separated items specifying the flags to be set.
 * The items can be in any order, but repeating an entry or specifying two different values for
 * an item will result in an error. One item specifies case handling and can be either "matchCase"
 * or "ignoreCase". If neither is specified, case is ignored. Another item specifies direction and
 * can be either "forward" or "backward". If neither is specified, the direction is forward.
 * Another item specifies whether to look for whole words only and can be either "wholeWords" or
 * "partialWordsOK". If neither is specified, partial words will be found. Another item 
 * specifies whether to wrap to the beginning if the end of the document is reached and can be
 * either "wrap" or "noWrap". If neither is specified, wrapping will not occur. Another item
 * specifies whether accents on characters should be ignored and can be either "matchAccents" or
 * "ignoreAccents". If neither is specified, only characters with matching accents will be found.
 * The result is put in searchFlags.
 * 
 * \commanddef{deviceAddListener}
 *
 * \commandparams
 * none
 *
 * \commanddef{deviceGetActivationRecord, str}
 *
 * \commandparams
 * \commandparam{str,output,string}
 *
 * \commanddef{deviceGetDeviceKey, key}
 *
 * \commandparams
 * \commandparam{key,output,string}
 *
 * \commanddef{deviceGetDeviceName, name}
 *
 * \commandparams
 * \commandparam{name,output,string}
 *
 * \commanddef{deviceGetDeviceType, type}
 *
 * \commandparams
 * \commandparam{type,output,string}
 *
 * \commanddef{deviceGetFingerprint, fingerprint}
 *
 * \commandparams
 * \commandparam{fingerprint,output,string}
 *
 * \commanddef{deviceGetIndex, index}
 *
 * \commandparams
 * \commandparam{index,output,integer}
 *
 * \commanddef{deviceGetProviderIdentifier, id}
 *
 * \commandparams
 * \commandparam{id,output,string}
 *
 * \commanddef{deviceGetProviderIndex, index}
 *
 * \commandparams
 * \commandparam{index,output,integer}
 *
 * \commanddef{deviceGetVersionInfo, name info}
 *
 * \commandparams
 * \commandparam{name,input,string}
 * \commandparam{str,output,string}
 *
 * \commanddef{deviceIsTrusted, flag}
 *
 * \commandparams
 * \commandparam{flag,output,integer}
 *
 * \commanddef{deviceSetActivationRecord, str}
 *
 * \commandparams
 * \commandparam{str,input,string}
 *
 * \commanddef{disableCallbackReporting}
 *
 * \commandparams
 * none
 *
 * Turns off reporting of calls to dpdoc::RendererClient::requestRepaint, 
 * dpdoc::RendererClient::reportMouseLocationInfo, dpdoc::RendererClient::reportInternalNavigation,
 * dpdoc::RendererClient::reportDocumentSizeChange, dpdoc::RendererClient::reportHighlightChange,
 * and dpdoc::RendererClient::finishedPlaying. This does not execute any SDK APIs
 * 
 * \commanddef{disableExternalLinks}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::allowExternalLinks API with false passed in the parameter .
 * 
 * \commanddef{disablePageNumbers}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::showPageNumbers API with false passed in the parameter .
 * 
 * \commanddef{DRMProcessor_getActivation, activationNumber userID deviceID expiration adobeID hasCredentials}
 *
 * \commandparams
 * \commandparam{activationNumber,input,integer}
 * \commandparam{userID,output,string}
 * \commandparam{deviceID,output,string}
 * \commandparam{expiration,output,string}
 * \commandparam{adobeID,output,string}
 * \commandparam{hasCredentials,output,integer}
 *
 * Calls DRMProcessor::getActivations(), gets the activation specified in the first param, calls all the methods
 * of dpdrm::Activation and stores results in output params from second to sixth in the order specified above.
 * 
 * \commanddef{DRMProcessor_getActivationsCount, count}
 *
 * \commandparams
 * \commandparam{count,output,integer}
 *
 * Calls DRMProcessor::getActivations(), and stores the count of activations received.
 * 
 * \commanddef{DRMProcessor_getFulfillmentID, fulfillmentID}
 *
 * \commandparams
 * \commandparam{fulfillmentID,output,string}
 *
 * Calls DRMProcessor::getFulfillmentID()
 * 
 * \commanddef{DRMProcessor_getFulfillmentItems}
 *
 * \commandparams
 * none
 *
 * Calls DRMProcessor::getFulfillmentItems() and stores result as current FulgillmentItems list.
 * 
 * \commanddef{DRMProcessor_initSignInWorkflow, initilizedWorkflows requestedWorkflows authProvider username password}
 *
 * \commandparams
 * \commandparam{initilizedWorkflows,output,integer}
 * \commandparam{requestedWorkflows,input,integer}
 * \commandparam{authProvider,input,string}
 * \commandparam{username,input,string}
 * \commandparam{password,input,string}
 *
 * Calls DRMProcessor::initSignInWorkflow()
 * 
 * \commanddef{DRMProcessor_initTokenSignInWorkflow, initilizedWorkflows requestedWorkflows authProvider username authData}
 *
 * \commandparams
 * \commandparam{initilizedWorkflows,output,integer}
 * \commandparam{requestedWorkflows,input,integer}
 * \commandparam{authProvider,input,string}
 * \commandparam{username,input,string}
 * \commandparam{authData,input,string}
 *
 * Calls DRMProcessor::initSignInWorkflow(), after base64 decoding authData into binary.
 *
 * \commanddef{DRMProcessor_initJoinAccountsWorkflow, initilizedWorkflows user operatorURL title}
 *
 * \commandparams
 * \commandparam{initilizedWorkflows,output,integer}
 * \commandparam{user,input,string}
 * \commandparam{operatorURL,input,string}
 * \commandparam{title,input,string}
 *
 * Calls DRMProcessor::initJoinAccountsWorkflow()
 * 
 * \commanddef{DRMProcessor_initLoanReturnWorkflow, initilizedWorkflows loanID}
 *
 * \commandparams
 * \commandparam{initilizedWorkflows,output,integer}
 * \commandparam{loanID,input,string}
 *
 * Calls DRMProcessor::initLoanReturnWorkflow()
 * 
 * \commanddef{DRMProcessor_initUpdateLoansWorkflow, initilizedWorkflows operatorURL resourceID}
 *
 * \commandparams
 * \commandparam{initilizedWorkflows,output,integer}
 * \commandparam{operatorURL,input,string}
 * \commandparam{resourceID,input,string}
 *
 * Calls DRMProcessor::initUpdateLoansWorkflow()
 * 
 * \commanddef{DRMProcessor_initWorkflows, initilizedWorkflows requestedWorkflows acsm}
 *
 * \commandparams
 * \commandparam{initilizedWorkflows,output,integer}
 * \commandparam{requestedWorkflows,input,integer}
 * \commandparam{acsm,input,string}
 *
 * Calls DRMProcessor::initWorkflows()
 * 
 * \commanddef{DRMProcessor_isReturnable, returnable}
 *
 * \commandparams
 * \commandparam{returnable,output,integer}
 *
 * Calls DRMProcessor::isReturnable() and returns 1, when the most recent fulfillment was a loan, and 0 otherwise.
 * 
 * \commanddef{DRMProcessor_reset}
 *
 * \commandparams
 * none
 *
 * Calls DRMProcessor::reset()
 * 
 * \commanddef{DRMProcessor_setPartition}
 *
 * \commandparams
 * none
 *
 * Calls DRMProcessor::setPartition() passing a current partition as an argument. 
 * See \a createPartition and \a libraryGetPartition for more info on current partition.
 * 
 * \commanddef{DRMProcessor_setUser, userID}
 *
 * \commandparams
 * \commandparam{userID,input,string}
 *
 * Calls DRMProcessor::setUser()
 * 
 * \commanddef{DRMProcessor_startWorkflows, requestedWorkflows}
 *
 * \commandparams
 * \commandparam{requestedWorkflows,input,integer}
 *
 * Calls DRMProcessor::startWorkflows()
 * 
 * \commanddef{DRMProcessor_transferCredentialsFrom, userID all}
 *
 * \commandparams
 * \commandparam{userID,input,string}
 * \commandparam{all,input,integer}
 *
 * Calls DRMProcessor::transferCredentialsFrom() for current processor and device and
 * specified userID. Set \a all parameter to zero, if only the basic credentials required 
 * for opening the document should be saved permanently.
 * See createDevice and createDRMProcessor.
 * 
 * \commanddef{DRMProcessor_transferLoanTokensFrom}
 *
 * \commandparams
 * none
 *
 * Calls DRMProcessor::transferLoanTokensFrom() for current processor and device. 
 * See createDevice and createDRMProcessor.
 * 
 * \commanddef{enableCallbackReporting}
 *
 * \commandparams
 * none
 *
 * Turns on reporting of calls to dpdoc::RendererClient::requestRepaint, RendererClient::reportMouse-LocationInfo, RendererClient::reportInternalNavigation, RendererClient::reportDocumentSize-Change, RendererClient::reportHighlightChange, and RendererClient::finishedPlaying. Reports are made with a message written to stdout. This does not execute any embed interface APIs.
 * 
 * \commanddef{enableExternalLinks}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::allowExternalLinks API with true passed in the parameter .
 * 
 * \commanddef{enablePageNumbers}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::showPageNumbers API with true passed in the parameter .
 * 
 * \commanddef{findText, stringToFind searchFlags startSearch endSearch abortCount startStringFound endStringFound}
 *
 * \commandparams
 * \commandparam{stringToFind,input,string}
 * \commandparam{searchFlags,input,integer}
 * \commandparam{startSearch,input,string}
 * \commandparam{endSearch,input,String}
 * \commandparam{abortCount,input,integer}
 * \commandparam{startStringFound,output,location}
 * \commandparam{endStringFound,output,location}
 *  
 * Executes the dpdoc::Document::findText API passing stringToFind, searchFlags as flags, startSearch as start, and endSearch as end. The searchFlags can be created using the createSearchFlags command. The start and end of the string found are placed in startStringFound and endStringFound respectively. If the stringToFind was not found, "String [stringToFind] not found" is written to stdout. Abort count is described in notes.
 * 
 * \commanddef{floor, realNumber intNumber}
 *
 * \commandparams
 * \commandparam{realNumber,input,real}
 * \commandparam{intNumber,output,integer}
 *  
 * Untility command to convert real to integer rounding down.
 * 
 * \commanddef{focusIn}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::handleEvent API with an event of dpdoc::EK_FOCUS kind and dpdoc::FOCUS_IN type.
 * 
 * \commanddef{focusOut}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::handleEvent API with an event of dpdoc::EK_FOCUS kind and dpdoc::FOCUS_OUT type.
 * 
 * \commanddef{fulfillmentItemGetDownloadMethod, downloadMethod}
 *
 * \commandparams
 * \commandparam{downloadMethod,output,string}
 *
 * Calls FulfillmentItem::GetDownloadMethod of current fulfillment item.
 * 
 * \commanddef{fulfillmentItemGetDownloadURL, downloadURL}
 *
 * \commandparams
 * \commandparam{downloadURL,output,string}
 *
 * Calls FulfillmentItem::GetDownloadURL of current fulfillment item.
 * 
 * \commanddef{fulfillmentItemGetMetadata, value name}
 *
 * \commandparams
 * \commandparam{value,output,string}
 * \commandparam{name,input,string}
 *
 * Calls FulfillmentItem::GetMetadata of current fulfillment item for specified name.
 * 
 * \commanddef{fulfillmentItemGetPostData, postData}
 *
 * \commandparams
 * \commandparam{postData,output,string}
 *
 * Calls FulfillmentItem::GetPostData of current fulfillment item.
 * 
 * \commanddef{fulfillmentItemGetRights}
 *
 * \commandparams
 * none
 *
 * Calls FulfillmentItem::GetRights of current fulfillment item and saves the result as
 * current rights data.
 * 
 * \commanddef{fulfillmentItemsGetFulfillmentItem, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *
 * Selects index-th element of current FulfillmentItems list as current FulfillmentItem data.
 * 
 * \commanddef{fulfillmentItemsLength, length}
 *
 * \commandparams
 * \commandparam{length,output,integer}
 *
 * Gets the length of current FulfillmentItems list
 * 
 * \commanddef{getActiveHighlight, index start end}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{start,output,location}
 * \commandparam{end,output,location}
 *  
 * Executes the dpdoc::Renderer::getHighlight API passing dpdoc::HT_ACTIVE as the highlight type and the value from the index parameter as the index. Places the start and end values returned in the places specified by the start and end parameters respectively. Prints a message to stdout if false is returned.
 * 
 * \commanddef{getActiveHighlightColor, index color}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{color,output,integer}
 *  
 * Executes the dpdoc::Renderer::getHighlightColor API passing dpdoc::HT_ACTIVE as the highlight type, the value from the index parameter as the index, and places the result in the place specified by color parameter.
 * 
 * \commanddef{getActiveHighlightCount, count}
 *
 * \commandparams
 * \commandparam{count,output,integer}
 *  
 * Executes the dpdoc::Renderer::getHighlightCount API passing dpdoc::HT_ACTIVE as the highlight type and places the result in the place specified by count parameter.
 * 
 * \commanddef{getAnnotationHighlight, index start end}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{start,output,location}
 * \commandparam{end,output,location}
 *  
 * Executes the dpdoc::Renderer::getHighlight API passing dpdoc::HT_ANNOTATION as the highlight type and the value from the index parameter as the index. Places the start and end values returned in the places specified by the start and end parameters respectively. Prints a message to stdout if false is returned.
 * 
 * \commanddef{getAnnotationHighlightColor, index color}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{color,output,integer}
 *  
 * Executes the dpdoc::Renderer::getHighlightColor API passing dpdoc::HT_ANNOTATION as the highlight type, the value from the index parameter as the index, and places the result in the place specified by color parameter.
 * 
 * \commanddef{getAnnotationHighlightCount, count}
 *
 * \commandparams
 * \commandparam{count,output,integer}
 *  
 * Executes the dpdoc::Renderer::getHighlightCount API passing dpdoc::HT_ANNOTATION as the highlight type and places the result in the place specified by count parameter.
 * 
 * \commanddef{getContentIteratorForText, start}
 *
 * \commandparams
 * \commandparam{start,input,Location}
 *  
 * Executes the Document::getContentIterator API with the ContentVariety set to CV_TEXT, passes
 * in the value from the start parameter as the position, and saves the result. Only a single content
 * iterator is saved at any time. An additional call to this command will overwrite the old content
 * iterator. The content iterator becomes invalid on a call that changes the current document.
 *
 * \commanddef{getDefaultFontSize, size}
 *
 * \commandparams
 * \commandparam{size,output,real}
 *  
 * Executes the dpdoc::Renderer::getDefaultFontSize API and places the result in the place specified by the size parameter.
 * 
 * \commanddef{getDocumentBeginning, beginning}
 *
 * \commandparams
 * \commandparam{beginning,output,location}
 *  
 * Executes the dpdoc::Document::getBeginning API and places the result in the calculated value specified by the beginning parameter.
 * 
 * \commanddef{getDocumentEnd, end}
 *
 * \commandparams
 * \commandparam{end,output,location}
 *  
 * Executes the dpdoc::Document::getEnd API and places the result in the calculated value specified by the end parameter.
 * 
 * \commanddef{getDocumentInterfaceVersion, version}
 *
 * \commandparams
 * \commandparam{version,output,integer}
 *  
 * Executes the dpdoc::Document::getInterfaceVersion API and places the result in the calculated value specified by the version parameter.
 * 
 * \commanddef{getErrorList}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Document::getErrorList API and saves the result. Only a single error list is saved at any time. An additional call to this command will overwrite the old error list. The error list  becomes invalid on a call to any command that accesses the Document or Renderer.
 * 
 * \commanddef{getErrorListLength, length }
 *
 * \commandparams
 * \commandparam{length,output,integer}
 *   
 * Executes the dp::StringList::length API on the current ErrorList placing the result in the calculated value specified by the length parameter.  This command may only be used if getErrorList was the last command to access the Document or Renderer.
 * 
 * \commanddef{getErrorString, index errorString}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{errorString,output,string}
 *  
 * Executes the dp::StringList::item API on the current ErrorList passing in the index specified in the first parameter.  The result is copied into the calculated value specified by the errorString parameter.  The result is then released by calling the ErrorList::releaseErrorString API.This command may only be used if getErrorList was the last command to access the Document or Renderer.
 * 
 * \commanddef{getLinkCount, count}
 *
 * \commandparams
 * \commandparam{count,output,integer}
 *  
 * Executes the dpdoc::Renderer::getLinkCount API and places the result in the place specified by the count parameter.
 * 
 * \commanddef{getLinkLocations, index startLocation endLocation}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{startLocation,output,location}
 * \commandparam{endLocation,output,location}
 *  
 * Executes the Renderer::getLinkInfo API passing in the value specified by the index parameter as the linkIndex. The values returned in the srcStart and srcEnd are placed in the calculated values specified by the startLocation parameter and the endLocation parameter respectively.
 * 
 * \commanddef{getLinkTargetLocation, index location}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{location,output,location}
 *  
 * Executes the Renderer::getLinkInfo API passing in the value specified by the index parameter as the linkIndex and placing the value returned as the target in the calculated value specified by the location parameter.
 * 
 * \commanddef{getLocation, currentLocation}
 *
 * \commandparams
 * \commandparam{currentLocation,output,location}
 *  
 * Executes the dpdoc::Renderer::getCurrentLocation API and places the result in the calculated value specified by the currentLocation parameter.
 * 
 * \commanddef{getLocationFromPagePosition, pagePosition location}
 *
 * \commandparams
 * \commandparam{pagePosition,input,real}
 * \commandparam{location,output,location}
 *  
 * Executes the dpdoc::Document::getLocationFromPagePosition API passing the value from the pagePosition parameter as pos and placing the result in the calculated value specified by the location parameter.
 * 
 * \commanddef{getMarkedArea, xMin yMin xMax yMax}
 *
 * \commandparams
 * \commandparam{xMin,output,real}
 * \commandparam{yMin,output,real}
 * \commandparam{xMax,output,real}
 * \commandparam{yMax,output,real}
 *  
 * Executes dpdoc::Renderer::getMarkedArea API and returns xMin, yMin, xMax, and yMax
 * 
 * \commanddef{getMetadata, name index metadata attributes}
 *
 * \commandparams
 * \commandparam{name,input,string}
 * \commandparam{index,input,integer}
 * \commandparam{metadata,output,string}
 * \commandparam{attributes,output,string}
 *  
 * Executes the dpdoc::Document::getMetadata API passing in the name and index specified in the first two parameters.  The result is copied into the calculated value specified by the metadata parameter and the string returned through the attrs is copied into the calculated value specified by the attributes parameter.  The result and the attributes are then released by calls to the dpdoc::Document::releaseMetadata API.
 * 
 * \commanddef{getMetadataNoAttrs, name index metadata}
 *
 * \commandparams
 * \commandparam{name,input,string}
 * \commandparam{index,input,integer}
 * \commandparam{metadata,output,string}
 *  
 * Executes the dpdoc::Document::getMetadata API passing in the name and index specified in the first two parameters and a NULL in attrs.  The result is copied into the calculated value specified by the metadata parameter.  The result is then released by calling the dpdoc::Document::releaseMetadata API.
 * 
 * \commanddef{getNavigationMatrix, ctm}
 *
 * \commandparams
 * \commandparam{ctm,output,matrix}
 *  
 * Executes Renderer::getNavigationMatrix API and returns the result
 * 
 * \commanddef{getNaturalSize, width height}
 *
 * \commandparams
 * \commandparam{width,output,real}
 * \commandparam{height,output,real}
 *  
 * Executes Renderer::getNaturalSize API and returns width and height
 * 
 * \commanddef{getPageCount, count}
 *
 * \commandparams
 * \commandparam{count,output,real}
 *  
 * Executes dpdoc::Document::getPageCountAPI and returns the count.
 * 
 * \commanddef{getPageEnd, pagePosition end}
 *
 * \commandparams
 * \commandparam{pagePosition,input,real}
 * \commandparam{end,output,location}
 *  
 * Executes the dpdoc::Document::getPageEnd API passing the value from the pagePosition parameter as pos and places the result in the calculated value specified by the end parameter.
 * 
 * \commanddef{getPageName, pagePosition name}
 *
 * \commandparams
 * \commandparam{pagePosition,input,real}
 * \commandparam{name,output,string}
 *  
 * Executes the dpdoc::Document::getPageName API passing the value from the pagePosition parameter as pos.  It places the result in the calculated value specified by the name parameter and then calls dpdoc::Document::releasePageName.
 * 
 * \commanddef{getPageNumbersForScreen, start end}
 *
 * \commandparams
 * \commandparam{start,output,integer}
 * \commandparam{end,output,integer}
 *  
 * Executes the Renderer::getPageNumbersForScreen API placing the value returned in start and end in the calculated values specified by the start and end parameters respectively.
 * 
 * \commanddef{getPagePosition, name pagePosition}
 *
 * \commandparams
 * \commandparam{name,input,string}
 * \commandparam{pagePosition,output,real}
 *  
 * Executes the dpdoc::Document::getPagePosition API passing the value from the name parameter as pos.  It places the result in the calculated value specified by the pagePosition parameter.
 * 
 * \commanddef{getPagePositionFromLocation, location pagePosition }
 *
 * \commandparams
 * \commandparam{location,input,location}
 * \commandparam{pagePosition,output,real}
 *   
 * Executes the dpdoc::Document::getPagePositionFromLocation API passing the value from the location parameter as pos and placing the result in the calculated value specified by the pagePosition parameter.
 * 
 * \commanddef{getPagingMode, mode}
 *
 * \commandparams
 * \commandparam{mode,output,integer}
 *  
 * Executes Renderer::getPagingMode API and places the paging mode in the spot indicated by the mode parameter. The value of mode will be one of the values specified by dpdoc::PagingMode.
 * 
 * \commanddef{getPlayMode, mode}
 *
 * \commandparams
 * \commandparam{mode,output,integer}
 *  
 * Executes the Renderer::getPlayMode API and places the result in the place specified by the mode parameter.
 * 
 * \commanddef{getRangeInfo, start end}
 *
 * \commandparams
 * \commandparam{start,input,Location}
 * \commandparam{end,input,Location}
 *  
 * Executes the dpdoc::Renderer::getRangeInfo API and saves the result. Only a single range info is saved at any time. An additional call to this command will overwrite the old range info. The range info  becomes invalid on a call to any command that access the Document or Renderer.
 * 
 * \commanddef{getRangeInfoBox, index xMin yMin xMax yMax}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{xMin,output,double}
 * \commandparam{yMin,output,double}
 * \commandparam{xMax,output,double}
 * \commandparam{yMax,output,double}
 *  
 * Executes the dpdoc::RangeInfo::getBox API on the currently saved range info. The index specifies the box in case more than one box is needed to cover the range. No commands that access the Renderer may be called between the getRangeInfo command that created the range and this command.
 * 
 * \commanddef{getRangeInfoBoxCount, count}
 *
 * \commandparams
 * \commandparam{count,output,integer}
 *  
 * Executes the dpcdoc::RangeInfo::getBoxCount API on the currently saved range info and places the result in the place specified by the count parameter. No commands that access the Renderer may be called between the getRangeInfo command that created the range and this command.
 * 
 * \commanddef{getRendererCapabilities, capabilities}
 *
 * \commandparams
 * \commandparam{capabilities,output,integer}
 *  
 * Executes the dpdoc::Renderer::getCapabilities API and places the result in the calculated value specified by the capabilities parameter.
 * 
 * \commanddef{getRendererInterfaceVersion, version}
 *
 * \commandparams
 * \commandparam{version,output,integer}
 *  
 * Executes the dpdoc::Renderer::getInterfaceVersion API and places the result in the calculated value specified by the version parameter.
 * 
 * \commanddef{getRights}
 *
 * \commandparams
 * none
 * 
 * Executes the dpdoc::Document::getRights API and stores the result as current rights. Only one dpdrm::Rights
 * object can be stored at a time.
 *
 * \commanddef{getScreenEnd, endOfScreen}
 *
 * \commandparams
 * \commandparam{endOfScreen,output,location}
 *  
 * Executes the dpdoc::Renderer::getScreenEnd API and places the result in the calculated value specified by the endOfScreen parameter.
 * 
 * \commanddef{getScreenStart, beginningOfScreen}
 *
 * \commandparams
 * \commandparam{beginningOfScreen,output,location}
 *  
 * Executes the dpdoc::Renderer::getScreenBeginning API and places the result in the calculated value specified by the beginningOfScreen parameter.
 * 
 * \commanddef{getSelectHighlight, index start end}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{start,output,location}
 * \commandparam{end,output,location}
 *  
 * Executes the dpdoc::Renderer::getHighlight API passing HT_SELECTION as the highlight type and the value from the index parameter as the index. Places the start and end values returned in the places specified by the start and end parameters respectively. Prints a message to stdout if false is returned.
 * 
 * \commanddef{getSelectHighlightColor, index color}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{color,output,integer}
 *  
 * Executes the dpdoc::Renderer::getHighlightColor API passing HT_SELECTION as the highlight type, the value from the index parameter as the index, and places the result in the place specified by color parameter.
 * 
 * \commanddef{getSelectHighlightCount, count}
 *
 * \commandparams
 * \commandparam{count,output,integer}
 *  
 * Executes the dpdoc::Renderer::getHighlightCount API passing HT_SELECTION as the highlight type and places the result in the place specified by count parameter.
 * 
 * \commanddef{getText, start end text}
 *
 * \commandparams
 * \commandparam{start,input,location}
 * \commandparam{end,input,location}
 * \commandparam{text,output,string}
 *  
 * Executes the dpdoc::Document::getText API passing in the start and end specified in as the two parameters.  The result is copied into the calculated value specified by the text parameter.  The result is then released by calling the dpdoc::Document::releaseText API.
 * 
 * \commanddef{getTOCChild, item index child}
 *
 * \commandparams
 * \commandparam{item,input,TOCItem}
 * \commandparam{index,input,integer}
 * \commandparam{child,output,TOCItem}
 *  
 * Executes the TOCItem: getChild API on the TOCItem specified with the item parameter passing in the value from the index parameter as the index. Places the result in the calculated value specified by the child parameter.
 * 
 * \commanddef{getTOCChildCount, item count}
 *
 * \commandparams
 * \commandparam{item,input,TOCItem}
 * \commandparam{count,output,integer}
 *  
 * Executes the TOCItem: getChildCount API on the TOCItem specified with the item parameter and places the result in the calculated value specified by the count parameter.
 * 
 * \commanddef{getTOCLocation, item location}
 *
 * \commandparams
 * \commandparam{item,input,TOCItem}
 * \commandparam{location,output,location}
 *  
 * Executes the TOCItem: getLocation API on the TOCItem specified with the item parameter and places the result in the calculated value specified by the location parameter.
 * 
 * \commanddef{getTOCRoot, root}
 *
 * \commandparams
 * \commandparam{root,output,TOCItem}
 *  
 * Executes the dpdoc::Document::getTOCRoot API and places the result in the calculated value specified by the root parameter.
 * 
 * \commanddef{getTOCTitle, item title}
 *
 * \commandparams
 * \commandparam{item,input,TOCItem}
 * \commandparam{title,output,string}
 *  
 * Executes the TOCItem: getTitle API on the TOCItem specified with the item parameter and places the result in the calculated value specified by the title parameter.
 * 
 * \commanddef{getViewportSize, width height}
 *
 * \commandparams
 * \commandparam{width,output,real}
 * \commandparam{height,output,real}
 *  
 * Gets the current viewport size as specified in the most recent setViewportSize command, the command-line parameters for book2png, or the default viewport size. This does not execute any embed interface APIs.
 * 
 * \commanddef{hasFatalErrors, comment }
 *
 * \commandparams
 * \commandparam{comment,input,string}
 *   
 * Prints a line to stdout consisting of the string from comment parameter followed "Document has fatal errors" if there are fatal errors in the error list as determined by executing the ErrorList::hasFatalErrors API on the current ErrorList. This command may only be used if getErrorList was the last command to access the Document or Renderer.
 * 
 * \commanddef{hasErrors, comment}
 *
 * \commandparams
 * \commandparam{comment,input,string}
 *  
 * Prints a line to stdout consisting of the string from comment parameter followed "Document has errors" if there are errors in the error list as determined by executing the ErrorList::hasErrors API on the current ErrorList. This command may only be used if getErrorList was the last command to access the Document or Renderer.
 * 
 * \commanddef{hasWarnings, comment}
 *
 * \commandparams
 * \commandparam{comment,input,string}
 *  
 * Prints a line to stdout consisting of the string from comment parameter followed  "Document has warnings" if there are warnings in the error list as determined by executing the ErrorList::hasWarnings API on the current ErrorList. This command may only be used if getErrorList was the last command to access the Document or Renderer.
 * 
 * \commanddef{help, str}
 *
 * \commandparams
 * \commandparam{str,input,string}
 *
 * Provides rudimentary help. Prints a list of commands with names that contain \a str and their parameters.
 * 
 * \commanddef{hibernate}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::hibernate API with the given parameter and prints a line to stdout indicating if true or false was returned.
 * 
 * \commanddef{hitTest, x y location}
 *
 * \commandparams
 * \commandparam{x,input,real}
 * \commandparam{y,input,real}
 * \commandparam{location,output,location}
 *  
 * Executes the dpdoc::Renderer::hisTest API with the flags set to HF_SELECT. If true is returned, the location passed back is put in the spot specified by the location parameter. If false is returned, a message indicating no hit prints on stdout and the spot specified by the location parameter is set to null.
 * 
 * \commanddef{hitTestWithForce, x y location}
 *
 * \commandparams
 * \commandparam{x,input,real}
 * \commandparam{y,input,real}
 * \commandparam{location,output,location}
 *  
 * Executes the dpdoc::Renderer::hisTest API with the flags set to HF_SELECT or with HF_FORCE. If true is returned, the location passed back is put in the spot specified by the location parameter. If false is returned, a message indicating no hit prints on stdout and the spot specified by the location parameter is set to null.
 * 
 * \commanddef{isAtBeginning}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::isAtBeginning API and prints a line to stdout indicating if true or false was returned.
 * 
 * \commanddef{isAtEnd}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::isAtEnd API and prints a line to stdout indicating if true or false was returned.
 * 
 * \commanddef{isFocusable, layout}
 *
 * \commandparams
 * \commandparam{layout,input,integer}
 *  
 * Executes the dpdoc::Renderer::isFocusable API and prints a line to stdout indicating if true or false was returned.
 * 
 * \commanddef{keyDown, keyIdentifier modifiers keyLocation }
 *
 * \commandparams
 * \commandparam{keyIdentifier,input,string}
 * \commandparam{modifiers,input,integer}
 * \commandparam{keyLocation,input,integer}
 *   
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_KEYBOARD kind and KEY_DOWN type. The event contains the information from the keyIdentifier, modifiers, and keyLocation parameters. 
 * 
 * \commanddef{keyUp, keyIdentifier modifiers keyLocation }
 *
 * \commandparams
 * \commandparam{keyIdentifier,input,string}
 * \commandparam{modifiers,input,integer}
 * \commandparam{keyLocation,input,integer}
 *   
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_KEYBOARD kind and KEY_UP type. The event contains the information from the keyIdentifier, modifiers, and keyLocation parameters. 
 * 
 * \commanddef{libraryAddListener}
 *
 * \commandparams
 * none
 *
 * Executes the dplib::Library::addListener method on the current library attaching a simple
 * implementation of dplib::LibraryListener that just reports every call to it to stdout.
 *
 * \commanddef{libraryCloneContentRecord, url}
 *
 * \commandparams
 * \commandparam{url,input,string}
 *
 * Executes the dplib::Library::cloneContentRecord method on the current library and the current
 * content record and stores the result as the current content record.
 *
 * \commanddef{libraryCreateContentRecord, url}
 *
 * \commandparams
 * \commandparam{url,input,string}
 *
 * Executes the dplib::Library::createContentRecord method on the current library and stores the result as
 * the current content record.
 *
 * \commanddef{libraryGetContentRecordByURL, url}
 *
 * \commandparams
 * \commandparam{url,input,string}
 *
 * Executes the dplib::Library::getContentRecordByURL method on the current library and stores the result as
 * the current content record.
 *
 * \commanddef{libraryGetContentRecords}
 *
 * \commandparams
 * none
 *
 * Executes the dplib::Library::getContentRecords method on the current library using current tag as a
 * parameter and stores the result as the current list of content records.
 *
 * \commanddef{libraryGetPartition}
 *
 * \commandparams
 * none
 *
 * Executes the dplib::Library::getPartition method on the current library  
 * and stores the result as the current partition.
 *
 * \commanddef{libraryGetTagByID, tagid}
 *
 * \commandparams
 * \commandparam{tagid,input,string}
 *
 * Executes the dplib::Library::getTagByID method on the current library and stores the result as
 * the current tag.
 *
 * \commanddef{libraryGetTags}
 *
 * \commandparams
 * none
 *
 * Executes the dplib::Library::getTags method on the current library using current tag as a
 * parameter and stores the result as the current list of tags.
 *
 * \commanddef{libraryIsLoaded, loaded}
 *
 * \commandparams
 * \commandparam{loaded,output,integer}
 *
 * Executes the dplib::Library::isLoaded method on the current library sets output parameter to
 * 1 if true is returned and 0 otherwise.
 *
 * \commanddef{libraryRemoveContentRecord}
 *
 * \commandparams
 * none
 *
 * Executes the dplib::Library::removeContentRecord method on the current library using current content record as a
 * parameter.
 *
 * \commanddef{libraryRemoveTag}
 *
 * \commandparams
 * none
 *
 * Executes the dplib::Library::removeTag method on the current library using current content tag as a
 * parameter.
 *
 * \commanddef{licenseConsume, count}
 *
 * \commandparams
 * \commandparam{count,input,integer}
 *
 * Executes dpdrm::License::consume method for the current license.
 *
 * \commanddef{licenseGetCurrentCount, permType count}
 *
 * \commandparams
 * \commandparam{permType,input,string}
 * \commandparam{count,output,integer}
 *
 * Executes dpdrm::License::getCurrentCount method for the current license and returns the result.
 *
 * \commanddef{licenseGetDistributorID, distributorID}
 *
 * \commandparams
 * \commandparam{distributorID,output,string}
 *
 * Executes dpdrm::License::getDistributorID method for the current license and returns the result.
 *
 * \commanddef{licenseGetFlavor, flavor}
 *
 * \commandparams
 * \commandparam{flavor,output,string}
 *
 * Executes dpdrm::License::getFlavor method for the current license and returns the result.
 *
 * \commanddef{licenseGetFulfillmentID, fulfillmentID}
 *
 * \commandparams
 * \commandparam{fulfillmentID,output,string}
 *
 * Executes dpdrm::License::getFulfillmentID method for the current license and returns the result.
 *
 * \commanddef{licenseGetLicensee, licensee}
 *
 * \commandparams
 * \commandparam{licensee,output,string}
 *
 * Executes dpdrm::License::getLicensee method for the current license and returns the result.
 *
 * \commanddef{licenseGetLicenseURL, licenseURL}
 *
 * \commandparams
 * \commandparam{licenseURL,output,string}
 *
 * Executes dpdrm::License::getLicenseURL method for the current license and returns the result.
 *
 * \commanddef{licenseGetOperatorURL, operatorURL}
 *
 * \commandparams
 * \commandparam{operatorURL,output,string}
 *
 * Executes dpdrm::License::getOperatorURL method for the current license and returns the result.
 *
 * \commanddef{licenseGetPermissions, permissionType}
 *
 * \commandparams
 * \commandparam{permissionType,output,string} - should be one of "display", "print", "excerpt", and "play".
 *
 * Executes dpdrm::License::getPermissions method for the current license and
 * stores the result as the current permission list.
 *
 * \commanddef{licenseGetResourceID, resourceID}
 *
 * \commandparams
 * \commandparam{resourceID,output,string}
 *
 * Executes dpdrm::License::getResourceID method for the current license and returns the result.
 *
 * \commanddef{licenseGetUserID, userID}
 *
 * \commandparams
 * \commandparam{userID,output,string}
 *
 * Executes dpdrm::License::getUserID method for the current license and returns the result.
 *
 * \commanddef{licenseGetVoucherID, voucher}
 *
 * \commandparams
 * \commandparam{voucher,output,string}
 *
 * Executes dpdrm::License::getVoucherID method for the current license and returns the result.
 *
 * \commanddef{licensesLength, length}
 *
 * \commandparams
 * \commandparam{length,output,integer}
 *
 * Executes dp::list::length method for the current list of licenses.
 *
 * \commanddef{licensesGetLicense, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *
 * Executes dp::list::operator[] method for the current list of licenses. Stores returned
 * license as the current license.
 *
 * \commanddef{mouseClick, button modifiers x y}
 *
 * \commandparams
 * \commandparam{button,input,integer}
 * \commandparam{modifiers,input,integer}
 * \commandparam{x,input,integer}
 * \commandparam{y,input,integer}
 *  
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_MOUSE kind and MOUSE_CLICK type. The event contains the information from the button, modifiers, x, and y parameters. 
 * 
 * \commanddef{mouseDown, button modifiers x y}
 *
 * \commandparams
 * \commandparam{button,input,integer}
 * \commandparam{modifiers,input,integer}
 * \commandparam{x,input,integer}
 * \commandparam{y,input,integer}
 *  
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_MOUSE kind and MOUSE_DOWN type. The event contains the information from the button, modifiers, x, and y parameters. 
 * 
 * \commanddef{mouseEnter, button modifiers x y}
 *
 * \commandparams
 * \commandparam{button,input,integer}
 * \commandparam{modifiers,input,integer}
 * \commandparam{x,input,integer}
 * \commandparam{y,input,integer}
 *  
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_MOUSE kind and MOUSE_ENTER type. The event contains the information from the button, modifiers, x, and y parameters. 
 * 
 * \commanddef{mouseExit, button modifiers x y}
 *
 * \commandparams
 * \commandparam{button,input,integer}
 * \commandparam{modifiers,input,integer}
 * \commandparam{x,input,integer}
 * \commandparam{y,input,integer}
 *  
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_MOUSE kind and MOUSE_EXIT type. The event contains the information from the button, modifiers, x, and y parameters. 
 * 
 * \commanddef{mouseMove, button modifiers x y}
 *
 * \commandparams
 * \commandparam{button,input,integer}
 * \commandparam{modifiers,input,integer}
 * \commandparam{x,input,integer}
 * \commandparam{y,input,integer}
 *  
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_MOUSE kind and MOUSE_MOVE type. The event contains the information from the button, modifiers, x, and y parameters. 
 * 
 * \commanddef{mouseUp, button modifiers x y}
 *
 * \commandparams
 * \commandparam{button,input,integer}
 * \commandparam{modifiers,input,integer}
 * \commandparam{x,input,integer}
 * \commandparam{y,input,integer}
 *  
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_MOUSE kind and MOUSE_UP type. The event contains the information from the button, modifiers, x, and y parameters.
 * 
 * \commanddef{navigateActivate}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_NAVIGATE kind and NAVIGATE_ACTIVATE type.
 * 
 * \commanddef{navigateFirst, abortCount}
 *
 * \commandparams
 * \commandparam{abortCount,input,integer}
 *  
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_NAVIGATE kind and NAVIGATE_FIRST type. After the abortCount number of calls to canContinueProcessing ( PK_FOREGROUND ) are made, canContinueProcessing returns false. At other times, it returns true. If  abortCount is 0, canContinueProcessing always returns true.
 * 
 * \commanddef{navigateLast, abortCount}
 *
 * \commandparams
 * \commandparam{abortCount,input,integer}
 *  
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_NAVIGATE kind and NAVIGATE_LAST type. After the abortCount number of calls to canContinueProcessing ( PK_FOREGROUND ) are made, canContinueProcessing returns false. At other times, it returns true. If  abortCount is 0, canContinueProcessing always returns true.
 * 
 * \commanddef{navigateLeave}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_NAVIGATE kind and NAVIGATE_LEAVE type.
 * 
 * \commanddef{navigateNext, abortCount}
 *
 * \commandparams
 * \commandparam{abortCount,input,integer}
 *  
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_NAVIGATE kind and NAVIGATE_NEXT type. After the abortCount number of calls to canContinueProcessing ( PK_FOREGROUND ) are made, canContinueProcessing returns false. At other times, it returns true. If  abortCount is 0, canContinueProcessing always returns true.
 * 
 * \commanddef{navigatePrevious, abortCount}
 *
 * \commandparams
 * \commandparam{abortCount,input,integer}
 *  
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_NAVIGATE kind and NAVIGATE_PREVIOUS type. After the abortCount number of calls to canContinueProcessing ( PK_FOREGROUND ) are made, canContinueProcessing returns false. At other times, it returns true. If  abortCount is 0, canContinueProcessing always returns true.
 * 
 * \commanddef{navigateToActiveHighlight, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *  
 * Executes the dpdoc::Renderer::navigateToHighlight API passing HT_ACTIVE as the highlight type and the value from the index parameter as the index.
 * 
 * \commanddef{navigateToAnnotationHighlight, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *  
 * Executes the dpdoc::Renderer::navigateToHighlight API passing HT_ANNOTATION as the highlight type and the value from the index parameter as the index.
 * 
 * \commanddef{navigateToLocation, location  abortCount}
 *
 * \commandparams
 * \commandparam{location,input,location}
 *  \commandparam{abortCount,input,integer}
 *  
 * Executes the dpdoc::Renderer::navigateToLocation API passing the value from the location parameter as the loc.
 * 
 * \commanddef{navigateToSelectHighlight, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *  
 * Executes the dpdoc::Renderer::navigateToHighlight API passing HT_SELECTION as the highlight type and the value from the index parameter as the index.
 * 
 * \commanddef{next}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::nextScreen API.
 * 
 * \commanddef{parseLicense, licxml}
 *
 * \commandparams
 * \commandparam{licxml,input,string}
 *
 * Executes the dpdrm::DRMProvider::parseLicense method using \a licxml parameter
 * and stores the result as the current rights object.
 * 
 * \commanddef{partitionFindForURL, url}
 *
 * \commandparams
 * \commandparam{url,input,string}
 *
 * Executes dpio::Partition::findPartitionForURL method with \a url parameter and stores
 * the result as current partition object.
 * 
 * \commanddef{partitionGetDevice}
 *
 * \commandparams
 * none
 *
 * Executes dpio::Partition::getDevice method on the current partition object and stores
 * the result as current device object.
 * 
 * \commanddef{partitionGetIndex, index}
 *
 * \commandparams
 * \commandparam{index,output,integer}
 *
 * Executes dpio::Partition::getIndex method on the current partition object and stores
 * the result in \a index parameter.
 * 
 * \commanddef{partitionGetDocumentFolderURL, url}
 *
 * \commandparams
 * \commandparam{url,output,string}
 *
 * Executes dpio::Partition::getDocumentFolderURL method on the current partition object and stores
 * the result in \a url parameter.
 * 
 * \commanddef{partitionGetPartitionName, name}
 *
 * \commandparams
 * \commandparam{name,output,string}
 *
 * Executes dpio::Partition::getPartitionName method on the current partition object and stores
 * the result in \a name parameter.
 * 
 * \commanddef{partitionGetPartitionType, type}
 *
 * \commandparams
 * \commandparam{type,output,string}
 *
 * Executes dpio::Partition::getPartitionType method on the current partition object and stores
 * the result in \a type parameter.
 * 
 * \commanddef{partitionGetRootURL, url}
 *
 * \commandparams
 * \commandparam{url,output,string}
 *
 * Executes dpio::Partition::getRootURL method on the current partition object and stores
 * the result in \a url parameter.
 * 
 * \commanddef{partitionGetTemporaryFolderURL, url}
 *
 * \commandparams
 * \commandparam{url,output,string}
 *
 * Executes dpio::Partition::getTemporaryFolderURL method on the current partition object and stores
 * the result in \a url parameter.
 * 
 * \commanddef{performAction, action result}
 *
 * \commandparams
 * \commandparam{action,input,string}
 * \commandparam{result,output,integer}
 *
 * Executes dpdoc::Renderer::performAction method on \a action parameter and returns the result in \a result parameter.
 * 
 * \commanddef{permissionIsConsumable, flag}
 *
 * \commandparams
 * \commandparam{flag,output,string}
 *
 * Executes dpdrm::Permission::isConsumable method on the current permission object. Stores
 * "true" in the \a flag parameter is \b true was returned and "false" otherwise. 
 * 
 * \commanddef{permissionGetDeviceID, devid}
 *
 * \commandparams
 * \commandparam{devid,output,string}
 *
 * Executes dpdrm::Permission::getDeviceID method on the current permission object and stores
 * the result in \a devid parameter.
 * 
 * \commanddef{permissionGetDeviceType, devtype}
 *
 * \commandparams
 * \commandparam{devtype,output,string}
 *
 * Executes dpdrm::Permission::getDeviceType method on the current permission object and stores
 * the result in \a devtype parameter.
 * 
 * \commanddef{permissionGetExpiration, date}
 *
 * \commandparams
 * \commandparam{date,output,string}
 *
 * Executes dpdrm::Permission::getExpiration method on the current permission object and stores
 * the result in \a date parameter converting it to W3C DTF.
 * 
 * \commanddef{permissionGetIncrementInterval, interval}
 *
 * \commandparams
 * \commandparam{interval,output,real}
 *
 * Executes dpdrm::Permission::getIncrementInterval method on the current permission object and stores
 * the result in \a interval parameter.
 * 
 * \commanddef{permissionGetInitialCount, count}
 *
 * \commandparams
 * \commandparam{count,output,integer}
 *
 * Executes dpdrm::Permission::getInitialCount method on the current permission object and stores
 * the result in \a count parameter.
 * 
 * \commanddef{permissionGetLoanID, loanid}
 *
 * \commandparams
 * \commandparam{loanid,output,string}
 *
 * Executes dpdrm::Permission::getLoanID method on the current permission object and stores
 * the result in \a loanid parameter.
 * 
 * \commanddef{permissionGetMaxCount, count}
 *
 * \commandparams
 * \commandparam{count,output,integer}
 *
 * Executes dpdrm::Permission::getMaxCount method on the current permission object and stores
 * the result in \a count parameter.
 * 
 * \commanddef{permissionGetMaxResolution, res}
 *
 * \commandparams
 * \commandparam{res,output,real}
 *
 * Executes dpdrm::Permission::getMaxResolution method on the current permission object and stores
 * the result in \a res parameter.
 * 
 * \commanddef{permissionGetParts, parts}
 *
 * \commandparams
 * \commandparam{parts,output,string}
 *
 * Executes dpdrm::Permission::getParts method on the current permission object and stores
 * the result in \a parts parameter.
 * 
 * \commanddef{permissionGetPermissionType, type}
 *
 * \commandparams
 * \commandparam{type,output,string}
 *
 * Executes dpdrm::Permission::getPermissionType method on the current permission object and stores
 * the result in \a type parameter.
 * 
 * \commanddef{permissionsGetPermission, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *
 * Executes dp::list::operator[] method for the current list of permissions with \a index as a parameter. Stores returned
 * license as the current permission object.
 * 
 * \commanddef{permissionsLength, length}
 *
 * \commandparams
 * \commandparam{length,output,integer}
 *
 * Executes dp::list::length method for the current list of permissions. Stores returned
 * license in \a length parameter.
 * 
 * \commanddef{previous}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::previousScreen API.
 * 
 * \commanddef{printInteger, comment number}
 *
 * \commandparams
 * \commandparam{comment,input,string}
 * \commandparam{number,input,integer}
 *  
 * Prints a line to stdout consisting of the string from comment parameter followed by the value of the integer in the int parameter. This does not call any embed interface APIs.
 * 
 * \commanddef{printMatrix, comment matrix}
 *
 * \commandparams
 * \commandparam{comment,input,string}
 * \commandparam{matrix,input,matrix}
 *  
 * Prints a line to stdout consisting of the string from comment parameter followed by 6 real values of the matrix parameter components. This does not call any embed interface APIs.
 * 
 * \commanddef{printReal, comment number}
 *
 * \commandparams
 * \commandparam{comment,input,string}
 * \commandparam{number,input,real}
 *  
 * Prints a line to stdout consisting of the string from comment parameter followed by the value of the real number in the number parameter. This does not call any embed interface APIs.
 * 
 * \commanddef{printString, comment string}
 *
 * \commandparams
 * \commandparam{comment,input,string}
 * \commandparam{string,input,string}
 *  
 * Prints a line to stdout consisting of the string from comment parameter followed by the the string from string parameter. This does not call any embed interface APIs.
 * 
 * \commanddef{process, abortCount}
 *
 * \commandparams
 * \commandparam{abortCount,input,integer}
 *  
 * Executes the dpdoc::Document::process API. See notes for abortCount parameter description.
 * 
 * \commanddef{queuePasshashInput}
 *
 * \commandparams
 * \commandparam{user,input,string}
 * \commandparam{password,input,string}
 * \commandparam{timeout,input,integer}
 * 
 * Adds next (user,password) pair to be used as user response on passhash request. The timeout parameter specifies the wait time (in milliseconds) before passhash is delivered. If this parameter equals to zero, value is delivered synchronously, otherwise it will be delivered asynchronously. Note that if the passhash is wrong, user will be asked again until the right passhash is entered. To cancel the loop simply provide the input with empty user parameter. Also, process will be cancelled, when no more pairs in the queue.
 * 
 * \commanddef{quit}
 *
 * \commandparams
 * none
 *
 * Terminates command file processing.
 * 
 * \commanddef{rangeEndsAfter}
 *
 * \commandparams
 * none
 *
 * Executes the RangeInfo::endsAfterThisScreen API on the currently saved range info and prints a line to stdout indicating if true or false was returned. No commands that access the Renderer may be called between the getRangeInfo command that created the range and this command.
 * 
 * \commanddef{rangeEndsBefore}
 *
 * \commandparams
 * Executes the RangeInfo::endsBeforeThisScreen API on the currently saved range info and prints a line to stdout indicating if true or false was returned. No commands that access the Renderer may be called between the getRangeInfo command that created the range and this command.
 * 
 * \commanddef{rangeStartsAfter}
 *
 * \commandparams
 * none
 *
 * Executes the RangeInfo::startsAfterThisScreen API on the currently saved range info and prints a line to stdout indicating if true or false was returned. No commands that access the Renderer may be called between the getRangeInfo command that created the range and this command.
 * 
 * \commanddef{rangeStartsBefore}
 *
 * \commandparams
 * none
 *
 * Executes the RangeInfo::startsBeforeThisScreen API on the currently saved range info and prints a line to stdout indicating if true or false was returned. No commands that access the Renderer may be called between the getRangeInfo command that created the range and this command.
 * 
 * \commanddef{readFile, content filename}
 *
 * \commandparams
 * \commandparam{content,output,string}
 * \commandparam{filename,input,string}
 *  
 * Reads the text file into a string. File size is limited to 40,000 characters.
 * 
 * \commanddef{removeActiveHighlight, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *  
 * Executes the dpdoc::Renderer::removeHighligh API passing HT_ACTIVE as the highlight type and the value from the index parameter as the index.
 * 
 * \commanddef{removeAnnotationHighlight, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *  
 * Executes the dpdoc::Renderer::removeHighligh API passing HT_ANNOTATION as the highlight type and the value from the index parameter as the index.
 * 
 * \commanddef{removeAllActiveHighlights}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::removeHighlights API passing HT_ACTIVE as the highlight type.
 * 
 * \commanddef{removeAllAnnotationHighlights}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::removeHighlights API passing HT_ANNOTATION as the highlight type.
 * 
 * \commanddef{removeAllSelectHighlights}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::removeHighlights API passing HT_SELECTION as the highlight type.
 * 
 * \commanddef{removeSelectHighlight, index}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 *  
 * Executes the dpdoc::Renderer::removeHighligh API passing HT_SELECTION as the highlight type and the value from the index parameter as the index.
 * 
 * \commanddef{render, abortCount}
 *
 * \commandparams
 * \commandparam{abortCount,input,integer}
 *  
 * Executes the dpdoc::Renderer::paint API. It uses bitmap format determined by -monochrome and -alpha book2png command-line options and dimensions determined by -width, -height and -scale options. See notes for abortCount parameter description.
 * 
 * \commanddef{renderRect, format xMin yMin xMax yMax abortCount}
 *
 * \commandparams
 * \commandparam{format,input,string}
 * \commandparam{xMin,input,integer}
 * \commandparam{yMin,input,integer}
 * \commandparam{xMax,input,integer}
 * \commandparam{yMax,input,integer}
 * \commandparam{abortCount,input,integer}
 *  
 * Executes the dpdoc::Renderer::paint API. It is similar to the render command, but provides more parameters to control bitmap format and the region to be painted.
 * 
 * \commanddef{rightsSerialize}
 *
 * \commandparams
 * none
 *
 * Executes dpdrm::Rights::serialize method on the current rights and prints result to stdout.
 * Rights can be extracted from the document using command \commandref{getRights}
 *
 * \commanddef{rightsGetLicenses}
 *
 * \commandparams
 * none
 *
 * Executes dpdrm::Rights::getLicenses method on the current rights and stores result in the current 
 * license list. Rights can be extracted from the document using command \commandref{getRights}.
 * License list can be examined using commands \commandref{licensesLength} and
 * \commandref{licensesGetLicense}
 *
 * \commanddef{rightsGetValidLicenses}
 *
 * \commandparams
 * none
 *
 * Executes dpdrm::Rights::getValidLicenses method on the current rights and stores result in the current 
 * license list. Rights can be extracted from the document using command \commandref{getRights}.
 * License list can be examined using commands \commandref{licensesLength} and
 * \commandref{licensesGetLicense}
 *
 * \commanddef{setEnvironmentMatrix, ctm}
 *
 * \commandparams
 * \commandparam{ctm,input,matrix}
 *  
 * Executes the dpdoc::Renderer::setEnvironmentMatrix API with the given parameter
 * 
 * \commanddef{setActiveHighlightColor, index color}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{color,input,integer}
 *  
 * Executes the dpdoc::Renderer::setHighlightColor API passing HT_ACTIVE as the highlight type, the value from the index parameter as the index, and the value from the color parameter as the color.
 * 
 * \commanddef{setAnnotationHighlightColor, index color}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{color,input,integer}
 *  
 * Executes the dpdoc::Renderer::setHighlightColor API passing HT_ANNOTATION as the highlight type the value from the index parameter as the index, and the value from the color parameter as the color.
 * 
 * \commanddef{setCSSMediaType, media}
 *
 * \commandparams
 * \commandparam{media,input,string}
 *  
 * Executes the dpdoc::Renderer::setCSSMediaType API passing the value from the media parameter.
 * 
 * \commanddef{setCurrentDocument, name}
 *
 * \commandparams
 * \commandparam{name,input,string}
 *  
 * Closes the current document and opens the document specified by the name parameter.
 *
 * \commanddef{setCurrentDocumentWithPassword, name password async}
 *
 * \commandparams
 * \commandparam{name,input,string}
 * \commandparam{name,input,password}
 * \commandparam{name,input,integer}
 *  
 * Closes the current document and opens the document specified by the name parameter using the password. If the async parameter is non-zero the password is supplied asynchronously, other it is supplied synchronously.
 *
 * \commanddef{setEnvironmentMatrix, ctm}
 *
 * \commandparams
 * \commandparam{ctm,input,matrix}
 *  
 * Executes the dpdoc::Renderer::setEnvironmentMatrix API with the given parameter.
 * 
 * \commanddef{setFontSize, size}
 *
 * \commandparams
 * \commandparam{size,input,real}
 *  
 * Executes the dpdoc::Renderer::setDefaultFontSize API passing value from the size parameter as the fontSize.
 * 
 * \commanddef{setMargins, top right bottom left}
 *
 * \commandparams
 * \commandparam{top,input,real}
 * \commandparam{right,input,real}
 * \commandparam{bottom,input,real}
 * \commandparam{left,input,real}
 *  
 * Executes the dpdoc::Renderer::setMargins API passing the values from the top, right, bottom, left left parameters.
 * 
 * \commanddef{setNavigationMatrix, ctm}
 *
 * \commandparams
 * \commandparam{ctm,input,matrix}
 *  
 * Executes the dpdoc::Renderer::setNavigationMatrix API with the given parameter.
 * 
 * \commanddef{setPageDecoration, pageGap borderWidth shadowWidth shadowOffset pageBackgroundColor shadowColor borderColorTL borderColorBR}
 *
 * \commandparams
 * \commandparam{pageGap,input,real}
 * \commandparam{borderWidth,input,real}
 * \commandparam{shadowWidth,input,real}
 * \commandparam{shadowOffset,input,real}
 * \commandparam{pageBackgroundColor,input,integer}
 * \commandparam{shadowColor,input,integer}
 * \commandparam{borderColorTL,input,integer}
 * \commandparam{borderColorBR,input,integer}
 *  
 * Executes the dpdoc::Renderer::setPageDecoration API with a PageDecoration structure containing the information from the pageGap, borderWidth, shadowWidth, shadowOffset, pageBackgroundColor, shadowColor, borderColorTL, and borderColorBR parameters. 
 * 
 * \commanddef{setPagingModeToFlowPages}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::setPagingMode API passing PM_FLOW_PAGES as the pagingMode.
 * 
 * \commanddef{setPagingModeToHardPages}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::setPagingMode API passing PM_HARD_PAGES as the pagingMode.
 * 
 * \commanddef{setPagingModeToScrollPages}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::setPagingMode API passing PM_SCROLL_PAGES as the pagingMode.
 * 
 * \commanddef{setPlayMode, mode}
 *
 * \commandparams
 * \commandparam{mode,input,integer}
 *  
 * Executes the dpdoc::Renderer::setPlayMode API passing value from the size parameter as the playMode.
 * 
 * \commanddef{setSelectHighlightColor, index color}
 *
 * \commandparams
 * \commandparam{index,input,integer}
 * \commandparam{color,input,integer}
 *  
 * Executes the dpdoc::Renderer::setHighlightColor API passing HT_SELECTION as the highlight type the value from the index parameter as the index, and the value from the color parameter as the color.
 * 
 * \commanddef{setTimeAccumulator, name}
 *
 * \commandparams
 * \commandparam{name,input,string}
 *  
 * Changes the time accumulator in use to the one specified by the name parameter. If the name parameter contains an empty string, accumulaton into the old time accumulator stops , but accumulation into a new one does not start.
 * 
 * \commanddef{setViewportSize, width height}
 *
 * \commandparams
 * \commandparam{width,input,real}
 * \commandparam{height,input,real}
 *  
 * Executes the Renderer::setViewport API with the given parameters.
 * 
 * \commanddef{supportsPixelLayout, layout}
 *
 * \commandparams
 * \commandparam{layout,input,integer}
 *  
 * Prints a line to stdout indicating if the pixel layout specified by the layout parameter is supportted by the current Renderer as determined by executing the Renderer::supportsPixelLayout API. The layout parameter must be one of the values in dpdoc::PixelLayout.
 * 
 * \commanddef{textInput, string }
 *
 * \commandparams
 * \commandparam{string,input,string}
 *   
 * Executes the dpdoc::Renderer::handleEvent API with an event of EK_TEXT kind and TEXT_INPUT type. The event contains the information from the string parameter. 
 * 
 * \commanddef{wakeUp}
 *
 * \commandparams
 * none
 *
 * Executes the dpdoc::Renderer::wakeUp API.
 *
 * \commanddef{walkScreen, string }
 *
 * \commandparams
 * \commandparam{string,input,string}
 *   
 * Executes the dpdoc::Renderer::walkScreen API with flags defined by the string parameter. The following words in the 
 * string parameter result in the listed flags to be set:
 * 
 * <table>
 * <tr><th>Word</th><th>Flag</th></tr>
 * <tr><td>region</td><td>dpdoc::DE_BODY_REGION</td></tr>
 * <tr><td>side</td><td>dpdoc::DE_SIDE_REGION</td></tr>
 * <tr><td>text</td><td>dpdoc::DE_TEXT</td></tr>
 * <tr><td>embed</td><td>dpdoc::DE_EMBED</td></tr>
 * <tr><td>walk</td><td>dpdoc::DE_WALK_EMBED</td></tr>
 * </table>
 * 
 * \commanddef{writeFile, content filename}
 *
 * \commandparams
 * \commandparam{content,input,string}
 * \commandparam{filename,input,string}
 *  
 * Writes the string into a text file. File will be truncated, if exists.
 */

#include "dp_core.h"
#include "dp_timer.h"
#include "dp_crypt.h"
#include "dp_io.h"
#include "dp_net.h"
#include "dp_dev.h"
#include "dp_drm.h"
#include "dp_doc.h"
#include "dp_lib.h"
#include "dp_fuse.h"
#include "dp_utils.h"
#include "dp_res.h"

#endif _DP_ALL_H

