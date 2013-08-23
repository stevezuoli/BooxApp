/**
 * Interaction Focus and Highlighting
 *
 * $Id: picsel-focus.h,v 1.31 2009/09/11 13:51:30 roger Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @defgroup TgvInteractionFocus Interaction Focus
 * @ingroup TgvFileViewer
 *
 * This group of features allows keyboard control of which interactive
 * item is highlighted.
 *
 * Web pages are interactive in two key ways. They allow hypertext
 * links to other web pages, which will be downloaded across the
 * network when the user selects them; and the allow dynamic content
 * such as form text entry fields and plug-ins.
 *
 * @{
 */

#ifndef PICSEL_FOCUS_H
#define PICSEL_FOCUS_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * AlienInformation types used to report result of the last command.
 *
 * All of the commands will raise an AlienInformation event to indicate
 * if the command succeeded or failed. The event is NOT raised if the request
 * returned 0 (event not queued).
 *
 * @ref AlienInformation_FocusInformation indicates a PicselFocus_Information
 * structure, raised by PicselFocus_getInformation
 *
 * @ref AlienInformation_FocusResult indicates a PicselFocus_Result structure,
 * raised by all other PicselFocus_* functions.
 *
 * @ref AlienInformation_ImageNavigationAllowed indicates a
 * @ref PicselFocus_ImageNavigationAllowedInfo, raised when a file type is
 * determined.
 */
enum AlienInformation_Focus
{
    AlienInformation_FocusResult            = 0x15000, /**< result of last
                                                            command. */
    AlienInformation_FocusInformation       = 0x15001, /**< information about
                                                            focused item. */
    AlienInformation_ImageNavigationAllowed = 0x15002  /**< Notifies whether
                                                            image navigation
                                                            is allowed for the
                                                            current doc. */
};

/**
 * Passed as the data for an @ref AlienInformation_ImageNavigationAllowed event.
 */
typedef struct PicselFocus_ImageNavigationAllowedInfo
{
    Picsel_View *picselView;    /**< View handle */
    int imageNavigationAllowed; /**< Set to 1 if image navigation
                                 *   is allowed for the current document,
                                 *   0 otherwise */
}
PicselFocus_ImageNavigationAllowedInfo;

/**
 * Data for PicselFocus_navigate()
 */
typedef enum PicselFocus_Navigation
{
   /** Structural navigation positions */
   PicselFocus_Navigation_First = 65540,  /**< Set focus to first item in
                                           *   DOM order                  */

   PicselFocus_Navigation_Last,           /**< Set focus to last item in
                                           *   DOM order                  */

   /** Structural navigation directions*/
   PicselFocus_Navigation_MoveNext = 65550,
                                          /**< Move focus to next item in
                                            *   DOM order                  */
   PicselFocus_Navigation_MovePrev,       /**< Move focus to next item in
                                           *   DOM order                  */

   /** Spatial navigation positions */
   PicselFocus_Navigation_PageTop = 65560,
                                          /**< Set focus to topmost item on
                                           *   the current page           */
   PicselFocus_Navigation_PageLeft,       /**< Set focus to leftmost item on
                                           *   the current page           */
   PicselFocus_Navigation_PageRight,      /**< Set focus to rightmost item on
                                           *   the current page           */
   PicselFocus_Navigation_PageBottom,     /**< Set focus to bottommost item on
                                           *   the current page           */

   PicselFocus_Navigation_ScreenTop = 65570,
                                          /**< Set the focus to an item near
                                           *   the top of the screen      */
   PicselFocus_Navigation_ScreenLeft,     /**< Set the focus to an item near
                                           *   the left of the screen     */
   PicselFocus_Navigation_ScreenRight,    /**< Set the focus to an item near
                                           *   the right of the screen    */
   PicselFocus_Navigation_ScreenBottom,   /**< Set the focus to an item near
                                           *   the bottom of the screen   */

   /** Spatial navigation directions*/
   PicselFocus_Navigation_MoveUp = 65580, /**< Move the focus upwards     */
   PicselFocus_Navigation_MoveDown,       /**< Move the focus downwards   */
   PicselFocus_Navigation_MoveLeft,       /**< Move the focus leftwards   */
   PicselFocus_Navigation_MoveRight,       /**< Move the focus rightwards  */

   /** Focus at the provided position.  For use with
       PicselFocus_navigateScreen().  When used on unfocused items,
       will focus them.  When used on focused items, will activate
       them. */
   PicselFocus_Navigation_FocusAtPoint = 65590,

   /** Focus at the provided position.  For use with
       PicselFocus_navigateScreen().  Will never activate
       focused links. */
   PicselFocus_Navigation_HighlightAtPoint,

   /**
    * Retrieve information about item at point specified. Information will be
    * returned in a @ref AlienInformation_FocusInformation. The item will not be
    * focused or activated.
    */
   PicselFocus_Navigation_ItemAtPoint
}
PicselFocus_Navigation;


/**
 *  Flag values for PicselFocus_Information::flags. This is a bitfield.
 */
enum
{
    PicselFocus_FocusedItem       = 0x10000, /**< An item has focus        */
    PicselFocus_StrongFocus       = 0x20000, /**< The focused item would
                                              *   like keyboard input      */
    PicselFocus_SelectWidget      = 0x40000, /**< The focused item is a
                                              *   Select form widget       */
    PicselFocus_MultiSelectWidget = 0x80000  /**< The focused item is a
                                              *   Multi Select form widget */
};

/**
 *  Type to store a combination of the above flags.
 */
typedef unsigned int PicselFocus_InformationFlags;

/**
 * Type of navigation mode. This is used to determine the
 * focus navigation mode.
 */
typedef enum PicselFocus_NavigationMode
{
    PicselFocus_Normal     = 0,  /** < link and widget navigation only */
    PicselFocus_Image      = 1,  /** < Image navigation only */
    PicselFocus_Annotation = 2  /** < Annotation navigation only */
}
PicselFocus_NavigationMode;

/**
 * Error codes for PicselFocus_Result
 */
typedef enum PicselFocus_ErrorCode
{
    PicselFocus_ErrorNone        = 0,     /**< Last command completed
                                           *   successfully */
    PicselFocus_ErrorUnknown     = 65539, /**< General error */
    PicselFocus_OffDocumentEdge  = 65540  /**< Tried to focus on point off the
                                               document edge */
}
PicselFocus_ErrorCode;


/**
 * Return data for all PicselFocus_* functions, unless noted otherwise.
 * Sent to AlienEvent_information() with code @ref AlienInformation_FocusResult
 */
typedef struct PicselFocus_Result
{
    PicselFocus_ErrorCode failure;       /**< Result of last command */
}
PicselFocus_Result;


/**
 * The type of a focusable item.
 */
typedef enum PicselFocus_ItemType
{
    PicselFocus_Item_None = (1<<16),      /**< Nothing is highlighted */
    PicselFocus_Item_Link,                /**< A link */
    PicselFocus_Item_Widget               /**< A form widget (button, etc) */
}
PicselFocus_ItemType;


/**
 * Identifier for item
 *
 * This structure identifies a link on a page.  It is returned as part
 * of the @ref PicselFocus_Information structure.
 *
 * This structure will be deleted on return from AlienEvent_information(),
 * so the structure and the data must be copied if it is intended to
 * be used.
 */
typedef struct PicselFocus_ItemIdentifier
{
    int         length;                  /**< Data length in bytes */
    const char *data;                    /**< Pointer to data */
}
PicselFocus_ItemIdentifier;


/**
 *  Return data for PicselFocus_getInformation(), sent to
 *  AlienEvent_information() with code AlienInformation_focusInformation
 */
typedef struct PicselFocus_Information
{
    /**
     * Information flags, 1 or more from PicselFocus_InformationFlags
     */
    PicselFocus_InformationFlags      flags;

    /**
     * Target URL in UTF-8 format, or NULL.
     *
     * When the focus is on a hypertext anchor <A> tag, this is the
     * contents of the @c HREF attribute.
     * If there is a conflicting @c onClick attribute and JavaScript is enabled,
     * the destination may not be accurately reported.
     * For example:                                                      @code
     <a href="http://apple.com"
        onclick="window.location='http://banana.com'; return false;"
        title="Guess">Link where?</a>                                    @endcode
     * will return the @a targetUrl "http://apple.com" but a typical browser
     * will visit http://banana.com
     */
    const char                       *targetUrl;

    /**
     * Type of highlighted item.
     */
    PicselFocus_ItemType              itemType;

    /**
     * Identifier of highlighted item.  NULL if no item is highlighted.
     */
    const PicselFocus_ItemIdentifier *identifier;
}
PicselFocus_Information;

/**
 * Sets or moves the focus in the area/direction indicated
 *
 * The commands starting with PicselFocus_Navigation_Move shall move the
 * focus in the direction indicated. If there is no focus, focus
 * will be set to an appropriate object.
 *
 * The other commands will set the focus to an item in the
 * area indicated.
 *
 * PicselFocus_panTo(picselContext, false) is automatically called if focus
 * is set or moved successfully, however an AlienInformation event is not
 * raised by AlienInformation_panTo
 *
 * Raises an AlienInformation_FocusResult event to indicate if the focus was
 * successfully set or changed.
 *
 * @note the PicselFocus_Navigation_Screen commands do not restrict the
 * search to the screen-visible part of the document. They start the search
 * at that area, but consider the whole page.
 *
 * @param picselContext   set by AlienEvent_setPicselContext()
 * @param command         area of screen to start search at
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselFocus_navigate(Picsel_Context        *picselContext,
                         PicselFocus_Navigation command);

/**
 * Removes the focus from an item
 *
 * An error is raised if no item has focus.
 *
 * Raises an AlienInformation_FocusResult event to indicate if the command
 * failed or succeeded
 *
 * @param picselContext   set by AlienEvent_setPicselContext()
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselFocus_remove(Picsel_Context *picselContext);


/**
 * Activates item that has the focus.
 *
 * if the widget is a link, then it is followed.
 *
 * An error is raised if
 *   no item has focus, or
 *   item type can not be followed.
 *
 * @param picselContext   set by AlienEvent_setPicselContext()
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselFocus_activate(Picsel_Context *picselContext);

/**
 * Scroll the screen so that the item with focus is visible.
 *
 * Document is panned if new item is not wholly visible on screen. If the
 * zoom level is high enough so that the link extends off both the left &
 * right edges of the screen, then the zoom level is not changed, and the
 * centre of the item is centred on the screen. Like wise for the top &
 * bottom of the link.
 *
 * If forceCentre is true, the document is panned even if the item is already
 * wholly visible.
 *
 * An error is raised if no item currently has the focus.
 *
 * @param picselContext   set by AlienEvent_setPicselContext()
 * @param forceCentre     unconditionally centres the item -
 *                        normally the screen isn't panned if the item is on
 *                        the visible screen.
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselFocus_panTo(Picsel_Context *picselContext,
                      int             forceCentre);


/**
 * Get information about focused item.
 *
 * Response will be returned as an AlienInformation event.
 *
 * @param picselContext   set by AlienEvent_setPicselContext()
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselFocus_getInformation(Picsel_Context *picselContext);

/**
 * Sets the focus on the widget based on the coordinates provided, if necessary.
 *
 * If navigation is set to PicselFocus_Navigation_FocusAtPoint:
 * If the widget is previously focused and provided that it is a link,
 * then it is followed.
 *
 * If navigation is set to PicselFocus_Navigation_HighlightAtPoint:
 * The widget will not be followed, even if it was previously focused.
 *
 * @param picselContext   set by AlienEvent_setPicselContext()
 * @param navigation      Controls whether to allowed links to be followed
                          This must be one of
                          @ref PicselFocus_Navigation_FocusAtPoint,
                          @ref PicselFocus_Navigation_HighlightAtPoint,
                          or @ref PicselFocus_Navigation_ItemAtPoint

 * @param x               Horizontal screen coordinate clicked
 * @param y               Vertical screen coordinate clicked
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselFocus_navigateScreen(Picsel_Context        *picselContext,
                               PicselFocus_Navigation navigation,
                               int                    x,
                               int                    y);


/**
 * Sets the focus on the widget based on the identifier provided. The
 * identifier must have been previously obtained using
 * PicselFocus_getInformation().
 *
 * If the page has changed since PicselFocus_getInformation was called,
 * then the intended link may not be highlighted.
 *
 * @param picselContext   set by AlienEvent_setPicselContext()
 * @param identifier      Identifier returned after a call to
 *                        PicselFocus_getInformation().
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselFocus_navigateToItem(Picsel_Context                   *picselContext,
                               const PicselFocus_ItemIdentifier *identifier);


/**
 * Sets focus navigation mode.
 *
 * This sets the focus navigation mode either to normal mode, image mode,
 * or annotation mode. By default, the focus navigation mode is set to
 * PicselFocus_Normal.
 *
 * For instance, if image navigation mode is toggled, the
 * PicselFocus_navigate() function will move around images rather than links.
 * Image navigation must be sent again to revert back to normal behaviour.
 *
 * @param picselContext Set earlier by AlienEvent_setPicselContext()
 * @param navMode       Any value from @ref PicselFocus_NavigationMode
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselFocus_setNavigationMode(Picsel_Context             *picselContext,
                                  PicselFocus_NavigationMode  navMode);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_FOCUS_H */
