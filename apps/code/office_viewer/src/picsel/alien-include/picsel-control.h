/**
 * Document View Control
 *
 * The functions in this file are offered by the TGV library for use by
 * the Alien application. If these features are not required by the
 * application, it is not necessary to call these.
 *
 * @file
 * $Id: picsel-control.h,v 1.25.4.1 2010/02/03 13:26:28 neilg Exp $
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @defgroup TgvViewControl Document View Control
 * @ingroup TgvCommand
 *
 * Commands to control the current view of the document. This includes
 * the coordinate system, the zoom scale, pan position, current page, and so
 * on. It is based around the features of @ref TgvFileViewer.
 * See @ref TgvViewStates and @ref TgvCoordinate_Space_Zoom_Pan.
 *
 * @{
 */

#ifndef PICSEL_CONTROL_H
#define PICSEL_CONTROL_H

#include "alien-types.h"
#include "picsel-flowmode.h"
#include "picsel-screen.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * State of a pan or zoom operation when mixing quick and high
 * quality updates. Used by PicselControl_zoom() and PicselControl_pan().
 *
 * @implement Use PicselControl_Start at the beginning of the operation
 *            (start of stylus gesture detected), PicselControl_Continue for
 *            all intermediate states (gesture in progress), and
 *            PicselControl_End at the completion of the operation (gesture
 *            complete). See @ref TgvControl_State.
 *
 * @product   Document Viewing products, such as @ref TgvFileViewer.
 *
 * @dotfile control-state.dot
 *
 * Similarly for zoom gestures and PicselControl_zoom().
 */
typedef enum PicselControl_State
{
    /**
     * Requests quick update; PicselControl_End must follow soon afterwards.
     */
    PicselControl_Start = 65539,

    /**
     * Requests quick update; PicselControl_End must follow soon afterwards.
     */
    PicselControl_Continue,

    /**
     * The same as PicselControl_End, unless the optional inertial scrolling
     * feature is enabled.
     *
     * If the optional inertial scrolling feature is enabled, this initiates
     * an inertial scroll based on recent points supplied with
     * PicselControl_Continue.  The Pan will end automatically when the
     * animation completes.  PicselControl_End may be called to
     * terminate the animation before it completes.
     */
    PicselControl_Release,

    /**
     * Requests high quality update.
     */
    PicselControl_End
}
PicselControl_State;

/**
 * The preset positions on the document to which PicselControl_panPosition()
 * can pan.
 */
typedef enum PicselControl_Position
{
    /**
     * Scroll to the top left of the document.
     */
    PicselPan_TopLeft = 65539,

    /**
     * Scroll to the top right of the document.
     */
    PicselPan_TopRight,

    /**
     * Scroll to the bottom left of the document.
     */
    PicselPan_BottomLeft,

    /**
     * Scroll to the bottom right of the document.
     */
    PicselPan_BottomRight,

    /**
     * Scroll to the centre of the document.
     */
    PicselPan_Centre
}
PicselControl_Position;

/**
 * Types of @ref AlienInformation_Event for @ref TgvViewStates
 */
enum
{
    /** View state has been captured;  @c eventData is @ref
    AlienInformation_ViewStateInfo.*/
    AlienInformation_GetViewStateResult = 0x1E000,

    /** View state has been set;  @c eventData is @ref
    PicselControl_ViewStateResult.*/
    AlienInformation_SetViewStateResult
};

/**
 * Results of PicselControl_getViewState() and PicselControl_setViewState()
 * delivered in an @ref AlienInformation_Event.
 */
typedef enum PicselControl_ViewStateResult
{
    /**
     * If the request failed.
     */
    PicselControl_ViewStateResult_Failure = 0,

    /**
     * If the request is successfully completed.
     */
    PicselControl_ViewStateResult_Success,

    /**
     * If the previous request is not completed then the new request will
     * be rejected.
     */
    PicselControl_ViewStateResult_Reject
}
PicselControl_ViewStateResult;

/**
 * Document parameters capturing a view on screen. Used for recording and
 * moving to bookmarks. See @ref TgvViewStates.
 */
typedef struct PicselControl_ViewState
{
    char           *url;
    PicselFlowMode  flowMode;
    int             zoomValue;
    int             pageNumber;
    int             xScroll;
    int             yScroll;
    int             layoutWidth;
}
PicselControl_ViewState;

/**
 * Definition of a list of document views. Used even for a recording or
 * moving to the bookmark for a single document (viewCount = 1). See @ref
 * TgvViewStates.
 */
typedef struct PicselControl_ViewListState
{
    PicselControl_ViewState **views;
    int                       viewCount;
    PicselRotation            rotation;
    int                       fullScreen;
}
PicselControl_ViewListState;

/**
 * Passed as the data for @ref AlienInformation_GetViewStateResult and
 * @ref AlienInformation_SetViewStateResult events. See @ref TgvViewStates.
 *
 * 'state' pointer should be NULL except a success of
 * PicselControl_getViewState().
 */
typedef struct AlienInformation_ViewStateInfo
{
    Picsel_View                    *picselView; /**< View handle */
    PicselControl_ViewListState    *state;
    PicselControl_ViewStateResult   result;
}
AlienInformation_ViewStateInfo;


/**
 * Zoom document and control display quality.
 *
 * Zoom document to the desired magnification, centred about a certain point.
 * If the Alien application requests a magnification that would
 * go beyond a preset limit, it will zoom to that limit. See @ref
 * TgvZoom_Magnification. For zoomState sequencing and control of display
 * quality, see @ref PicselControl_State.
 *
 * This is similar to PicselControl_setZoom() but is recommended in
 * interactive cases where the user may wish to zoom to other sizes
 * immediately afterwards.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 * @param[in] zoomCentreX   X-coordinate of centre point of zoom, measured
 *                          in pixels from the left of the screen.
 * @param[in] zoomCentreY   Y-coordinate of centre point of zoom, measured
 *                          in pixels from the top of the screen.
 * @param[in] zoomState     Whether starting, continuing, or completing a
 *                          zoom operation. See @ref PicselControl_State.
 * @param[in] magnification Required magnification where 100% is (1<<16).
 *                          See @ref TgvZoom_Magnification.
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 *
 * @pre If another PicselControl* function has already been called for this
 * movement gesture, zoomState should be set to @ref PicselControl_Continue
 * or @ref PicselControl_End.
 * @post If panState was not @ref PicselControl_End, then another call to a
 * PicselControl* function should be called later with a state of @ref
 * PicselControl_End. The screen display quality will be poor until this is
 * done.
 * @product Document Viewing products, such as @ref TgvFileViewer.
 */
int PicselControl_zoom( Picsel_Context      *picselContext,
                        unsigned int         zoomCentreX,
                        unsigned int         zoomCentreY,
                        PicselControl_State  zoomState,
                        unsigned long        magnification);


/**
 * Get the page size on screen as a zoom magnification.
 *
 * The result will be reported to the Alien application using
 * AlienEvent_information() and an event of type @ref AlienInformation_Zoom.
 * See @ref TgvZoom_Magnification.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue. The requested information is
 *                          returned later in an @ref
 *                          AlienInformation_ZoomInfo struct.
 */
int PicselControl_getZoom(Picsel_Context *picselContext);

/**
 * Zoom the page size on screen to an absolute scale and centred about
 * a particular position on screen. This is similar to PicselControl_zoom()
 * but is recommended for non-interactive use, to go directly to a chosen
 * zoom scale.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 * @param[in] magnification Required magnification where 100% is (1<<16).
 *                          See @ref TgvZoom_Magnification.
 * @param[in] zoomCentreX   X-coordinate of centre point of zoom, measured
 *                          in pixels from the left edge of the screen.
 * @param[in] zoomCentreY   Y-coordinate of centre point of zoom, measured
 *                          in pixels from the top edge of the screen.
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 */
int PicselControl_setZoom(Picsel_Context *picselContext,
                          unsigned long   magnification,
                          unsigned int    zoomCentreX,
                          unsigned int    zoomCentreY);

/**
 * Pan (scroll) document on screen, relative to its current position.
 *
 * Moves the document page currently on screen by a number of pixels
 * horizontally and vertically. This might be used in a touch-screen
 * device, or to animate the page, for example. (For keystroke control see
 * @ref PicselKey and PicselCmd* such as @ref PicselCmdPanUp.) If the Alien
 * application requests a movement that would go beyond the limits of
 * that page boundary, it will pan to the limit.
 *
 * See also PicselThumbnail_panPageOrThumbnail(), whose behaviour can be
 * the same, depending on its parameters.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext()
 * @param[in] x             Horizontal offset to pan, in screen pixels. See
 *                          @ref TgvCoordinate_Space and @ref
 *                          TgvCoordinates_Relative.
 * @param[in] y             Vertical offset to pan, in screen pixels. See
 *                          @ref TgvCoordinate_Space and @ref
 *                          TgvCoordinates_Relative.
 * @param[in] panState      Position of this function call within a sequence
 *                          for the current gesture. This affects the speed
 *                          of redrawing the screen. See @ref
 *                          PicselControl_State.
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 * @pre If another PicselControl* function has already been called for this
 * movement gesture, panState should be set to @ref PicselControl_Continue
 * or @ref PicselControl_End.
 * @post If panState was not @ref PicselControl_End, then another call to a
 * PicselControl* function should be called later with a state of @ref
 * PicselControl_End. The screen display quality will be poor until this is
 * done.
 * @product Document Viewing products, such as @ref TgvFileViewer.
 */
int PicselControl_pan(Picsel_Context      *picselContext,
                      int                  x,
                      int                  y,
                      PicselControl_State  panState);

/**
 * Pan (scroll) the page to a corner or the centre of the document.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 * @param[in] position      Preset position to pan to. See @ref
 *                          PicselControl_Position.
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 */
int PicselControl_panPosition(Picsel_Context         *picselContext,
                              PicselControl_Position  position);

/**
 * Pan (scroll) document to the specified coordinates.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 * @param[in] x             x coordinate to pan to in units of 1/65536 of an
 *                          inch. See @ref TgvCoordinate_Space and @ref
 *                          TgvCoordinates_Absolute.
 * @param[in] y             y coordinate to pan to in units of 1/65536 of an
 *                          inch. See @ref TgvCoordinate_Space and @ref
 *                          TgvCoordinates_Absolute.
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 */
int PicselControl_setPan(Picsel_Context *picselContext,
                         int             x,
                         int             y);

/**
 * Get the pan position of the current document.
 *
 * The result will be reported to the Alien application using
 * AlienEvent_information() and an event of type @ref AlienInformation_Pan.
 * See @ref TgvCoordinate_Space.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue. The requested information is
 *                          returned later in an @ref
 *                          AlienInformation_PanInfo struct.
 */
int PicselControl_getPan(Picsel_Context *picselContext);

/**
 * Get the current view state on the screen in response to a user request
 * to set a bookmark. This allows a later call of
 * PicselControl_setViewState(). See also @ref TgvViewStates.
 *
 * The result will be reported to the Alien application using
 * AlienEvent_information() and an event of type
 * @ref AlienInformation_GetViewStateResult.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue. The requested information is
 *                          returned later in an @ref
 *                          AlienInformation_ViewStateInfo struct.
 * @post The relevant View State data must be used unchanged when calling
 * PicselControl_setViewState() later.
 */
int PicselControl_getViewState(Picsel_Context *picselContext);

/**
 * Set the view state on the screen in response to a user request to move
 * back to a bookmark previously captured with PicselControl_getViewState().
 * See also @ref TgvViewStates.
 *
 * The result will be reported to the Alien application using
 * AlienEvent_information() and an event of type
 * @ref AlienInformation_SetViewStateResult.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 * @param[in] state         View state to be set.
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 * @pre The View State must be data generated from a previous call to
 * PicselControl_getViewState().
 */
int PicselControl_setViewState(Picsel_Context              *picselContext,
                               PicselControl_ViewListState *state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_CONTROL_H */

/** @} */ /* End of Doxygen group */
