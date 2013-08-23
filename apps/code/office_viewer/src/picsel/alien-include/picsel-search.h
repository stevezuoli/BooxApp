/**
 * Search for Text within document
 *
 * These functions are provided by the Picsel library, and are
 * intended to be called by the alien application. They may only be
 * used in specific system states.
 *
 * $Id: picsel-search.h,v 1.29 2009/04/20 13:37:34 roger Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @defgroup TgvSearch Search for Text
 * @ingroup TgvFileViewer
 *
 * The Picsel library can search for text within the current
 * document, and will highlight matching words or phrases one
 * by one, panning and zooming the document view to highlight
 * the match clearly. The process involves several calls to
 * this API, and requires the dialogue box for the user to
 * enter the search terms to be implemented by the Alien
 * application, as usual.
 *
 * A search term is typically a word or a short phrase.
 * The minimum length is one character, and the maximum
 * is more than 200 characters, but these are rarely
 * reached because users would have to type them at the
 * keyboard of a device. The Picsel library will search
 * for this text contiguously within the document.
 * Certain characters are considered equivalent, such as:
 * - Full width and normal (half width) Latin
 * - Full width and half-width Katakana
 * - Upper and lower case Latin if "matchCase" is non-zero.
 * See PicselSearch_start() for details.
 *
 * The Alien application should prompt the user for the
 * search term before calling the APIs described here.
 *
 * @dotfile search-states-simple.dot
 *
 * As shown in the diagram above the Alien application should
 * call PicselSearch_start() with the search terms entered by the
 * user first, to initialise the system. It must then call
 * PicselSearch_forward() to begin the process asynchronously,
 * or PicselSearch_back() if preferred.
 *
 * The time taken to perform a search result depends on the complexity and
 * size of the document, and whether @ref PicselConfigFV_singlePageLoad is
 * enabled.  A complex search through many pages may take some seconds.
 * During this time, the user can continue to interact with the system.
 * When a match is found or all text within the
 * current page has been searched the Alien application will
 * be notified with @ref AlienInformation_SearchResult.
 *
 * After a matching phrase has been found, the text will be
 * highlighted and the Picsel library will await further commands
 * such as PicselSearch_forward().
 * To finish the process, the Alien application should call
 * PicselSearch_cancel().
 *
 * @section TgvSearchingWithSinglePageLoading Searching with Single Page Loading
 *
 * If Single Page Loading has been enabled with
 * @ref PicselConfigFV_singlePageLoad it is essential that
 * the Alien application must respond to the call to
 * AlienUserRequest_request() when the request type is
 * @ref PicselUserRequest_Type_SearchNextPage (see @ref alien-request.h) -
 * as shown in the diagram below.
 *
 * @dotfile search-states-spl.dot
 * @{
 */

#ifndef PICSEL_SEARCH_H
#define PICSEL_SEARCH_H

#include "alien-types.h"
#include "picsel-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------*/

/** Information events from the Picsel Application                        */

enum
{
    AlienInformation_SearchResult     = 0x11000, /**< Search result
                                                      returned             */
    AlienInformation_SearchListResult = 0x11001  /**< Search list result
                                                      returned             */
};

/**
 * Outcome or progress of a search-for-text request.
 * Passed as AlienInformation_SearchResultInfo.result
 * with an @ref AlienInformation_SearchResult event.
 */
typedef enum PicselSearch_Result
{

    /** A fragment of text in the document matches the search string. It will
      * normally be highlighted on screen. There may be more matches yet to
      * be found. */
    PicselSearch_Found = (1<<16),

    /** The 'current position' of searching has returned to the starting point.
      * If matches were found, they would have been notified already. This
      * event does not indicate any success or failure of matching.
      * It indicates only that the whole document has been covered. */
    PicselSearch_NotFound,

    /** The last page of the document has been searched. This
      * event does not indicate any success or failure of matching.
      * It indicates only that the last page of the document has been covered. */
    PicselSearch_EndOfDocument,

    /** An error occurred while searching.  */
    PicselSearch_Error,

    /** The search is underway.  This event may be sent out multiple
      * times for each search, and can be used to indicate the current
      * page being searched. */
    PicselSearch_Progressing,

    /** The display has successfully been snapped to show the matching text
      * within the document. */
    PicselSearch_SnapToResultDone
}
PicselSearch_Result;

/**
 * Error code
 */
typedef enum PicselSearch_ErrorType
{
    PicselSearch_ErrorType_None = (1<<16),

    /** A null or zero length search string was supplied.
        Search strings should be at least 1 character long. */
    PicselSearch_ErrorType_EmptyString,

    /** Out of memory error occurred during search */
    PicselSearch_ErrorType_OutOfMemory,

    /** The document was closed during the search */
    PicselSearch_ErrorType_DocumentClosed,

    /** A search feature was used when search is not started.  This means
        that PicselSearch_start() has not been called, or it has failed. */
    PicselSearch_ErrorType_SearchNotStarted,

    /** The maximum number of search results has been exceeded.
        This event will only be sent if a search is initiated with
        PicselSearch_startList(). The value used as the maximum number of
        search results is given in the "maxResults" argument to the
        PicselSearch_startList() call. */
    PicselSearch_ErrorType_ExceededMaxListSize,

    /** Search is not currently supported in Powerzoom mode */
    PicselSearch_ErrorType_PowerzoomNotSupported,

    /** An error occurred that search was not able to identify.  Please
        check for errors reported to AlienError_error(). */
    PicselSearch_ErrorType_Unknown
}
PicselSearch_ErrorType;

/** Search direction.  Indicates whether the last search performed
 *  was a forward or back through the text, indicating that the last
 *  call was to PicselSearch_forward() or PicselSearch_back().  Calls to
 *  other functions will result in @ref PicselSearch_None.                   */

typedef enum PicselSearch_Direction
{
    PicselSearch_Forward = (1<<16), /**< Searching forwards                */
    PicselSearch_Back,              /**< Searching back                    */
    PicselSearch_None               /**< No direction                      */
}
PicselSearch_Direction;

/** Search result information.  Contains information about the last search
 *  performed.  This structure will be passed to AlienEvent_information.  */

typedef struct AlienInformation_SearchResultInfo
{
    PicselSearch_Result     result;     /**< Outcome of search             */
    PicselSearch_ErrorType  errorType;  /**< Error type, if any            */
    PicselSearch_Direction  direction;  /**< Direction of search           */
    int                     resultPage; /**< Page associated with event    */
}
AlienInformation_SearchResultInfo;

/**
 * Search list result information. Contains information about the last
 * search list operation performed. Once a search list operation has been
 * started by calling PicselSearch_startList(), the search list results will
 * be sent in response to every call to PicselSearch_forward() or
 * PicselSearch_back().                                                    */
typedef struct AlienInformation_SearchListResultInfo
{
    PicselSearch_Result     result;     /**< Outcome of search             */
    PicselSearch_ErrorType  errorType;  /**< Error type, if any            */
    PicselSearch_Direction  direction;  /**< Direction of search           */
    int                     resultPage; /**< Page which result is on       */
    int                     id;         /**< The result ID.                */
    int                     offset;     /**< Offset within 'text' to the
                                             search term.                  */
    Picsel_Utf8            *text;       /**< The search result text, which
                                             will include at least the search
                                             term, but will also include any
                                             words before or after the search
                                             term as requested, where
                                             possible.                     */
}
AlienInformation_SearchListResultInfo;


/*------------------------------------------------------------------------*/

/**
 * This function initialises a search from the current position. A search
 * result will not be returned until a call is made to PicselSearch_forward()
 * or PicselSearch_back()
 *
 * The search string must be of non-zero length, otherwise the
 * @ref PicselSearch_ErrorType_EmptyString error will be sent.
 *
 * @note Search is not currently supported in Powerzoom mode.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 * @param text          Text to search for (UTF-8 encoded).  This must be
 *                      non-NULL, or this function will return 0.
 *                      A zero-length string is not permitted - this will
 *                      generate an error when PicselSearch_forward() or
 *                      PicselSearch_back() is called.
 * @param matchCase     Nonzero if exact case match is required
 * @param minFontSize   Minimum font size in points. Will zoom in if result
 *                      is smaller than this. 0 if no minimum size.
 * @param maxFontSize   Maximum font size in points. Will zoom out if result
 *                      is larger than this. 0 if no maximum size.
 *
 * @return              Queue status, normally 1. See @ref TgvAsync_Queue
 *
 */
int PicselSearch_start(Picsel_Context      *picselContext,
                       const unsigned char *text,
                       int                  matchCase,
                       unsigned int         minFontSize,
                       unsigned int         maxFontSize);


/**
 * This function initialises a search from the current position.  The results
 * generated are suitable for use in a search results list.
 * The application should call PicselSearch_startList() to initialise the
 * search and then call PicselSearch_forward() or PicselSearch_back() to
 * start receiving results.  A single @ref AlienInformation_SearchListResult event
 * will be returned for each PicselSearch_forward() or PicselSearch_back() call.
 * The document position will not be changed when a result is found.
 * The application may pan and/or zoom to a result using the function
 * PicselSearch_snapToResultId() providing the search result ID (returned
 * from the @ref AlienInformation_SearchListResult event).
 * The list of search results is invalidated when PicselSearch_cancel() is
 * called, or a new search is initiated.
 * The text given in a search result will include the search term, with up to
 * maxCharsBefore before the search term and up to maxCharsAfter after the
 * search term. The number of characters added before and after the search
 * term will be rounded down to the nearest whole word.
 *
 * The search string must be of non-zero length, otherwise the
 * @ref PicselSearch_ErrorType_EmptyString error will be sent.
 *
 * @note Search is not currently supported in Powerzoom mode.
 *
 * @param picselContext  Set by AlienEvent_setPicselContext()
 * @param text           Text to search for (UTF-8 encoded).  This must be
 *                       non-NULL, or this function will return 0.
 *                       A zero-length string is not permitted - this will
 *                       generate an error when PicselSearch_forward() or
 *                       PicselSearch_back() is called.
 * @param matchCase      Nonzero if exact case match is required
 * @param minFontSize    Minimum font size in points. Will zoom in if result
 *                       is smaller than this. 0 if no minimum size.
 * @param maxFontSize    Maximum font size in points. Will zoom out if result
 *                       is larger than this. 0 if no maximum size.
 * @param maxResults     The maximum number of results to return
 * @param maxCharsBefore The maximum number of characters to return before
 *                       the search term.
 * @param maxCharsAfter  The maximum number of characters to return after the
 *                       search term.
 *
 * @return               Queue status, normally 1. See @ref TgvAsync_Queue
 *
 */
PicselCommand_Result PicselSearch_startList(
                                         Picsel_Context      *picselContext,
                                         const unsigned char *text,
                                         int                  matchCase,
                                         unsigned int         minFontSize,
                                         unsigned int         maxFontSize,
                                         unsigned int         maxResults,
                                         unsigned int         maxCharsBefore,
                                         unsigned int         maxCharsAfter);

/**
 * Search forward from current position
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * See @ref TgvSearchingWithSinglePageLoading "Searching with Single Page Loading"
 * for additional details.
 *
 * @return              Queue status, normally 1. See @ref TgvAsync_Queue
 *
 *                      Later, the result of the search will be
 *                      indicated in a @ref AlienInformation_SearchResult
 *                      event (or a @ref AlienInformation_SearchListResult event)
 *                      to AlienEvent_information().
 *
 */
int PicselSearch_forward(Picsel_Context *picselContext);

/**
 * Search back from current position
 *
 * See @ref TgvSearchingWithSinglePageLoading "Searching with Single Page Loading"
 * for additional details.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @return              Queue status, normally 1. See @ref TgvAsync_Queue
 *
 *                      Later, the result of the search will be
 *                      indicated in a @ref AlienInformation_SearchResult
 *                      event (or a @ref AlienInformation_SearchListResult event)
 *                      to AlienEvent_information().
 *
 */
int PicselSearch_back(Picsel_Context *picselContext);

/**
 * If the current result is offscreen, or on a different page, this
 * will change page and pan and zoom until the result is visible.  For list
 * searches, please use PicselSearch_snapToResultId() below.
 *
 * A search information event of type @ref PicselSearch_SnapToResultDone
 * will be sent when this has completed.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @return              Queue status, normally 1. See @ref TgvAsync_Queue
 *
 */
int PicselSearch_snapToResult(Picsel_Context *picselContext);

/**
 * Highlight and move document to view the given search ID.  This is valid
 * only for searches started using PicselSearch_startList().
 *
 * A search information event of type @ref PicselSearch_SnapToResultDone
 * will be sent when this has completed.
 *
 * The search ID for search results will be obtained from
 * @ref AlienInformation_SearchListResult events.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 * @param id            The id of the result to highlight
 *
 * @return              Queue status, normally 1. See @ref TgvAsync_Queue
 *
 */
PicselCommand_Result PicselSearch_snapToResultId(
                                               Picsel_Context *picselContext,
                                               int             id);

/**
 * Set search position to a given page in document.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 * @param destPage      Page number to search from
 *
 * @return              Queue status, normally 1. See @ref TgvAsync_Queue
 *
 */
int PicselSearch_fromPage(Picsel_Context *picselContext, int destPage);

/**
 * Cancel search. Removes any highlighted results.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @return              Queue status, normally 1. See @ref TgvAsync_Queue
 *
 */
int PicselSearch_cancel(Picsel_Context *picselContext);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_SEARCH_H */
