/**
 * Multiple view support.  If multiple view support is required,
 * the application should keep an updated list of Picsel_View pointers,
 * and provide a UI method of switching between views.
 *
 * $Id: picsel-views.h,v 1.5 2008/12/11 16:34:23 roger Exp $
 * @file
 */
/* Copyright (C) Picsel, 2007-2008. All Rights Reserved. */
/**
 * @defgroup TgvMultipleViews Multiple Views
 * @ingroup TgvBrowser
 *
 * Opening of Multiple Views (tabs) within a single browser
 * context. Each view may contain a different document (web page).
 *
 * @{
 */
#ifndef PICSEL_VIEWS_H
#define PICSEL_VIEWS_H

#include "alien-types.h"
#include "alien-legacy.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Information events from the Picsel Application */
enum
{
    /**
     * New view opened.  This event will be sent in two cases:
     * 1. At startup, to create the first view;
     * 2. When loading a document and specifying the "new view" option.
     *
     * If multiple view support is required, this pointer can be used to
     * identify this view.  The application should store the
     * Picsel_View pointer.  The application may also add a tab or a
     * menu item to represent this view.
     *
     * There will always be at least one view available while the
     * application is running.
     *
     * @warning NOT YET IMPLEMENTED - DO NOT USE.
     */
    AlienInformation_NewView = 0x1C000,

    /**
     * An error has occurred while the library was attempting to create a
     * new view.  The document will not be loaded.
     *
     * @warning NOT YET IMPLEMENTED - DO NOT USE.
     */
    AlienInformation_NewViewError,

    /**
     * Indicates that a view has been removed.  After this is sent,
     * the Picsel_View pointer specified in AlienInformation_RemoveViewInfo
     * should not be used.  The application may use this to remove a tab or
     * menu item that represents this view.
     *
     * @warning NOT YET IMPLEMENTED - DO NOT USE.
     */
    AlienInformation_RemoveView,

    /**
     * Indicates that the specified view is the current view.  The
     * application may use this to highlight a tab or a menu item.
     *
     * @warning NOT YET IMPLEMENTED - DO NOT USE.
     */
    AlienInformation_CurrentView
};

typedef enum PicselViewResult
{
    /**< A new view could not be opened, because there are already a
     *   maximum number of views open. */
    PicselViewResult_MaximumReached = 0x10000
}
PicselViewResult;

/**
 * Data sent with the AlienInformation_NewView event.
 */
typedef struct AlienInformation_NewViewInfo
{
    Picsel_View *picselView;  /**< Handle for new view */
}
AlienInformation_NewViewInfo;

/**
 * Data sent with the AlienInformation_NewView event.
 */
typedef struct AlienInformation_NewViewErrorInfo
{
    Picsel_View     *picselView; /**< Handle for the existing view */
    PicselViewResult result;     /**< Indicates the error that occurred */
}
AlienInformation_NewViewErrorInfo;

/**
 * Data sent with the AlienInformation_RemoveView event.
 */
typedef struct AlienInformation_RemoveViewInfo
{
    Picsel_View *picselView;  /**< View that has been removed*/
}
AlienInformation_RemoveViewInfo;

/**
 * Data sent with the AlienInformation_CurrentView event.
 */
typedef struct AlienInformation_CurrentViewInfo
{
    Picsel_View *picselView;  /**< Current view */
}
AlienInformation_CurrentViewInfo;

/**
 * Flags to specify the behaviour when opening a link in a new view.
 */
enum
{
    PicselView_OpenFlags_InBackground = 0x01 /**< Open the new view in the
                                                  background */
};

/* Type representing a combination of the above flags */
typedef int PicselView_OpenFlags;

/**
 * Select which view is to be visible.  All other views will be hidden.
 *
 * AlienInformation_CurrentView will be sent once this has completed
 * successfully.
 *
 * @warning NOT YET IMPLEMENTED - DO NOT USE.
 *
 * @param[in] picselContext  The Picsel context
 * @param[in] picselView     The view
 *
 * @return                   1 if the request is queued for action, or 0 if
 *                           an error occurs.
 */
int PicselView_select(Picsel_Context *picselContext,
                      Picsel_View    *picselView);

/**
 * If the focused item is a link, open a new view to load the linked page.
 * The view will then become the current view, unless
 * PicselView_OpenFlags_InBackground is specified.
 *
 * An error is raised if no item has focus, or the item type can not be
 * followed.
 *
 * @warning NOT YET IMPLEMENTED - DO NOT USE.
 *
 * @param[in] picselContext  The Picsel context
 * @param[in] openFlags      Flags
 *
 * @return                   1 if the request is queued for action, or 0 if
 *                           an error occurs.
 */
int PicselView_openLinkInNewView(Picsel_Context       *picselContext,
                                 PicselView_OpenFlags  openFlags);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_VIEWS_H */
