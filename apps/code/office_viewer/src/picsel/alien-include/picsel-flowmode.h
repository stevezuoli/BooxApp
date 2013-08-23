/**
 * Flow Layout Modes
 *
 * This file contains definitions and declarations needed for flow mode.
 *
 * $Id: picsel-flowmode.h,v 1.25 2008/12/11 16:34:21 roger Exp $
 * @file picsel-flowmode.h
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @defgroup TgvFlowMode Flow Layout Modes
 * @ingroup TgvFileViewer
 *
 * The Picsel library usually displays the document content as it should
 * be printed, or would appear on a desktop computer screen.
 * On a mobile device with a small screen, users may wish to read the
 * text in a larger font, without having to scroll along each line.
 * The flow mode options here allow that to be configured at run-time,
 * as an option.
 *
 * Currently, only the following types of document are reflowable:
 * - HTML
 * - MHTML
 * - Microsoft Word
 * - Plain text
 *
 * @{
 */

#ifndef PICSEL_FLOWMODE_H
#define PICSEL_FLOWMODE_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Flow mode settings.
 */
typedef enum PicselFlowMode
{
    /**
     * Document is in its original form <HR>
     */
    FlowMode_Normal = (1<<16),

    /**
     * Document is flowed so that it fits to the width of the screen,
     * and horizontal scrolling is not required. <HR>
     */
    FlowMode_FitScreenWidth,

    /**
     * As for @c FlowMode_FitScreenWidth but document is re-flowed after
     * each zoom operation. Some documents are unsuitable for
     * @c FlowMode_PowerZoom and may display poorly.  For such documents,
     * use a different flow mode. This is not a standard feature of
     * Picsel products. If the mode is not supported, calls
     * to PicselFlowMode_set() will result in the mode being set
     * to @c FlowMode_FitScreenWidth instead.
     *
     * This feature is not enabled in the Picsel library by default.
     * Please contact your Picsel support representative to discuss
     * enabling this feature. <HR>
     */
    FlowMode_PowerZoom
}
PicselFlowMode;

/**
 * Values suitable for passing in to PicselFlowMode_setTextSize().
 * Only values between @c TextSize_Minimum and @c TextSize_Maximum
 * are supported. Picsel recommends using given values:
 * @c TextSize_Small, @c TextSize_Medium and @c TextSize_Large.
 */
typedef enum PicselFlowMode_TextSize
{
    TextSize_Minimum = 3216,    /**< Minimum text size permitted */

    TextSize_Small   = 6<<16,
    TextSize_Medium  = 8<<16,   /**< Medium is the default text size */
    TextSize_Large   = 10<<16,

    TextSize_Maximum = 2139216  /**< Maximum text size permitted */
}
PicselFlowMode_TextSize;

/**
 * Information events from the Picsel Library
 *
 * @ingroup TgvContentInformation
 */
enum
{
    /**
     * Flow mode has changed; @c eventData
     * is @ref AlienInformation_FlowModeInfo
     */
    AlienInformation_FlowMode = 0x14000
};

/**
 * Result of the set flow mode command.
 * @see PicselFlowMode_set().
 */
typedef enum PicselFlowResult
{
    /**
     * If last request failed <HR>
     */
    FlowResult_Failure = 0,

    /**
     * If last request succeeded <HR>
     */
    FlowResult_Success = 1,

    /**
     * Ignoring setting flow mode on unsupported file types
     * e.g. pdf, ppt, xls, images. <HR>
     */
    FlowResult_UnsupportedFileType = (1<<16)
}
PicselFlowResult;

/**
 * A structure passed as the data for an @ref AlienInformation_FlowMode event
 */
typedef struct AlienInformation_FlowModeInfo
{
    Picsel_View        *picselView;       /**< View handle                */
    PicselFlowMode      flowMode;         /**< The current flow mode      */
    PicselFlowResult    result;           /**< Result of flow mode change */
}
AlienInformation_FlowModeInfo;

/**
 * Set the flow mode of the document.  Response will be returned as
 * an @ref AlienInformation_Event.
 *
 * When thumbnail magnify mode is enabled, only the normal flow mode is supported.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 * @param flowMode      The flow mode
 *
 * @retval   1 if the event has been accepted;
 * @retval   0 if the event was rejected.
 */
int PicselFlowMode_set(Picsel_Context *picselContext,
                       PicselFlowMode  flowMode);

/**
 * Get the current flow mode of the document. Response will be returned
 * as an @ref AlienInformation_Event.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @return The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 */
int PicselFlowMode_get(Picsel_Context *picselContext);

/**
 * Set the size of display text in @c FlowMode_FitScreenWidth mode.
 *
 * @param picselContext      Set by AlienEvent_setPicselContext().
 *                           It must not be NULL.
 * @param textSize           The text size, specified as a numerical value.
 *                           Suggested values are between @c TextSize_Small
 *                           and @c TextSize_Large. The text size shouldn't
 *                           be set any smaller than @c TextSize_Minimum,
 *                           or any larger than @c TextSize_Maximum. Values
 *                           outside this range will be clipped to
 *                           the minimum or maximum value.
 *
 * @return The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselFlowMode_setTextSize(Picsel_Context *picselContext,
                               unsigned int    textSize);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_FLOWMODE_H */
