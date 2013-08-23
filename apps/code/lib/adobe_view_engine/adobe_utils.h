#ifndef ADOBE_UTILS_H_
#define ADOBE_UTILS_H_

#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/ui.h"
#include "onyx/cms/content_thumbnail.h"
#include "onyx/cms/cms_tags.h"

#include "onyx/ui/render_policy.h"
#include "onyx/base/base_tasks.h"
#include "onyx/base/tasks_handler.h"
#include "onyx/data/configuration.h"
#include "onyx/sys/sys_utils.h"

namespace adobe_view
{

const double PI                 = 3.14159265f;
const double ZOOM_ERR           = 0.00001f;
const int DIAPLAY_MAX           = 0x7FFFFFFF;
const double INVALID_ZOOM_VALUE = 10000.0f;
const QImage::Format IMG_FORMAT = QImage::Format_Indexed8;

enum AdobeRenderOperation
{
    RENDER_SCALE_ZOOM = 0,
    RENDER_SCALE_FONT_INDEX,
    RENDER_SCALE_FONT_SIZE,
    RENDER_SCALE_AREA,
    RENDER_THUMBNAIL_SAVE,
    RENDER_THUMBNAIL_NAVIGATE_TO_LOCATION,
    RENDER_THUMBNAIL_NEXT_SCREEN,
    RENDER_THUMBNAIL_PREVIOUS_SCREEN,
    RENDER_NAVIGATE_TO_LOCATION = 16,
    RENDER_NAVIGATE_TO_HYPERLINK_TARGET,
    RENDER_NAVIGATE_TO_HIGHLIGHT,
    RENDER_NAVIGATE_PREV_SCREEN,
    RENDER_NAVIGATE_NEXT_SCREEN,
    RENDER_NAVIGATE_SCROLLING,
    RENDER_ROTATION = 32,
    RENDER_UPDATE_BY_MATRIX,
    RENDER_SWITCH_PAGE_MODE,
    RENDER_RESTORE
};

enum AdobeSearchOperation
{
    SEARCH_NEXT = 0,
    SEARCH_ALL
};

enum AdobeDocumentType
{
    INVALID_DOCUMENT = 0,
    REFLOWABLE_DOCUMENT = 1,
    FIX_PAGE_DOCUMENT = 2
};

enum AdobeContentIterationFlags
{
    CI_JOIN_ALPHA = 1,              /**< Join consecutive alphabetic characters */
    CI_JOIN_NUMERIC = 2,            /**< Join consecutive numeric characters */
    CI_IGNORE_TRAILING_SPACE = 4,   /**< Ignore trailing and leading space characters */
    CI_IGNORE_TRAILING_PUNC = 8,    /**< Ignore trailing and leading punctuation characters */
    CI_IGNORE_SHY = 16              /**< Ignore all soft hyphens */
};

enum TOCProperty
{
    TOC_ITEM = Qt::UserRole + 1
};

enum AdobeKeyboardEventType
{
    KEY_DOWN,   /**< corresponds to DOM Level 3 keydown */
    KEY_UP      /**< corresponds to DOM Level 3 keyup */
};

enum AdobeKeyLocation
{
    KEY_LOCATION_STANDARD      = 0x00, /**< default key location */
    KEY_LOCATION_LEFT          = 0x01, /**< "left" key if there are more than one key, e.g. left Shift */
    KEY_LOCATION_RIGHT         = 0x02, /**< "right" key if there are more than one key, e.g. right Shift */
    KEY_LOCATION_NUMPAD        = 0x03  /**< numpad-located key */
};

enum AdobeMouseEventType
{
    MOUSE_MOVE,     /**< Pointing device position changed. */
    MOUSE_UP,       /**< Pointing device button released. */
    MOUSE_DOWN,     /**< Pointing device button pressed. */
    MOUSE_CLICK,    /**< Pointing device button pressed and released in the small time interval and without change in position. */
    MOUSE_ENTER,    /**< Pointing device entered the area occupied by this renderer. */
    MOUSE_EXIT      /**< Pointing device left the area occupied by this renderer. */
};

enum AdobePagingMode
{
    PM_HARD_PAGES,      /**< single-page-based view that only shows a single page at a time */
    PM_HARD_PAGES_2UP,  /**< double-page-based view that shows 2 pages at a time */
    PM_FLOW_PAGES,      /**< a paginated view, where a screen takes up the whole viewport and the content is reflowed */
    PM_SCROLL_PAGES,    /**< scrollable page-based view showing a sequence of pages */
    PM_SCROLL           /**< HTML-browser-like view that can be scrolled and does not have pages */
};

enum AdobeSearchFlags
{
    SF_MATCH_CASE       = 1,    /**< Match the case of characters in the string being searched. */
    SF_BACK             = 2,    /**< Search toward the beginning of the document. */
    SF_WHOLE_WORD       = 4,    /**< Match whole word only. */
    SF_WRAP             = 8,    /**< Wrap search: start from the beginning when the end is reached. */
    SF_IGNORE_ACCENTS   = 0x10  /**< Ignore all accents on latin characters. SF_MATCH_CASE and SF_IGNORE_ACCENTS may be used in any combination. */
};

enum AdobeHitTestFlags
{
    HF_EVENT        = 1,    /**< Event-style hit testing - element which event handler should be invoked */
    HF_SELECT       = 2,    /**< Selection-style hit testing - which location in the document to use for selection */
    HF_FORCE        = 4     /**< Find some meaningful location even if nothing is hit (e.g. by looking for a closest element), this value can be ORed with others */
};

struct FulfillmentItemInfo
{
    QString            fulfillment_id;
    bool               is_returnable;
    bool               has_returned;
    unsigned long long expiration_date;

    FulfillmentItemInfo() : is_returnable(false), has_returned(false), expiration_date(0) {}
    bool isValid() { return !fulfillment_id.isEmpty() || expiration_date != 0; }
};

enum AdobeViewportLocation
{
    NO_SPACE        = 0x00,
    LEFT_SPACE      = 0x01,
    RIGHT_SPACE     = 0x02,
    UP_SPACE        = 0x04,
    BOTTOM_SPACE    = 0x08,
};

Q_DECLARE_FLAGS(AdobeViewportLocations, AdobeViewportLocation)
Q_DECLARE_OPERATORS_FOR_FLAGS(AdobeViewportLocations)

bool initializePlatform();

};

#endif
