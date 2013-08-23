/**
 * Configuration settings available in File Viewer
 *
 * This file contains the key strings that can be used to set configuration
 *
 * $Id: picsel-config-fileviewer.h,v 1.62.2.2 2010/02/17 17:02:41 neilk Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @defgroup TgvConfigFileViewer Configuration settings for File Viewer
 * @ingroup TgvFileViewer
 *
 * Configuration settings available in File Viewer.
 *
 * These are key strings that can be used to set configuration
 * which are relevant to viewing electronic documents. They should be
 * set using PicselConfig_setInt() or PicselConfig_setString().
 *
 * Default values are noted, where appropriate. In some cases, though, the
 * Picsel Library will calculate a default based on its "best guess" for the
 * device it finds itself on. Examples of this are @ref
 * PicselConfigFV_panUpAmount and @ref PicselConfigFV_panDownAmount - how
 * much to pan (scroll) depends on the dimensions of the screen.
 *
 * @{
 */

#ifndef PICSEL_CONFIG_FILEVIEWER_H
#define PICSEL_CONFIG_FILEVIEWER_H

#include "picsel-config.h"

/**
 * Enumeration values to be used for PicselConfigFV_RedrawMode.
 * @see @ref PicselConfigFV_adaptiveRedraw.
 */
enum
{
    /**
     * Always draw fuzzy first.
     */
    PicselConfigFV_RedrawMode_Fuzzy          = (1<<16),

    /**
     * Adaptive redraw - medium quality.
     */
    PicselConfigFV_RedrawMode_AdaptiveMedium,

    /**
     * Adaptive redraw - high quality.
     */
    PicselConfigFV_RedrawMode_AdaptiveHigh,

    /**
     * Always draw medium quality first, then redraw high quality later;
     * unsupported by Picsel.
     */
    PicselConfigFV_RedrawMode_SlowMedium,

    /**
     * Always draw high quality immediately; unsupported by Picsel.
     */
    PicselConfigFV_RedrawMode_SlowHigh
};

/**
 * Determines the quality with which the document is initially drawn to
 * the screen.

 * Drawing a document in high-quality mode for every update is quite
 * processor intensive.  Depending on the device, it may be desirable to
 * draw the screen first in a low-quality mode, where the output is "fuzzy"
 * (slightly defocused), and then immediately draw in high-quality mode.
 * This method can greatly improve the responsiveness of the user interface.
 *
 * This behaviour can be modified by setting the configuration value:
 * - @ref PicselConfigFV_RedrawMode_Fuzzy
 *    - Always draw fuzzy first, then redraw high quality later.
 * - @ref PicselConfigFV_RedrawMode_AdaptiveMedium
 *    - Adaptive redraw - medium quality (see @ref
 *      PicselConfigFV_adaptiveRedraw).
 * - @ref PicselConfigFV_RedrawMode_AdaptiveHigh
 *    - Adaptive redraw - high quality (see @ref
 *      PicselConfigFV_adaptiveRedraw).
 *
 * The following two options are unsupported by Picsel, and may only work for
 * certain platforms. Please contact Picsel before using either.
 *
 * - @ref PicselConfigFV_RedrawMode_SlowMedium
 *    - Always draw medium quality first, then redraw high quality later.
 * - @ref PicselConfigFV_RedrawMode_SlowHigh
 *    - Always draw high quality immediately.
 *
 * The default is @ref PicselConfigFV_RedrawMode_AdaptiveMedium, with a @ref
 * PicselConfigFV_adaptiveRedraw value of 0. Set this using
 * PicselConfig_setInt().
 *
 * @see @ref TgvCoordinate_Space_Zoom_Pan.
 */
#define PicselConfigFV_RedrawMode            "RendererRedrawMode"


/**
 *
 * Cap the redrawing rate.
 *
 * Enabling adaptive redraw (@ref PicselConfigFV_RedrawMode_AdaptiveMedium
 * or @ref PicselConfigFV_RedrawMode_AdaptiveHigh) allows the Picsel
 * Library to make a more intelligent decision on when to draw fuzzy
 * content and when to draw immediately in medium or high quality. Set this
 * using PicselConfig_setInt().
 *
 * In addition to selecting one of the PicselConfigFV_RedrawMode_AdaptiveXXXX
 * flags, there is an integer preference to control adaptive redraw.
 *
 * The value to choose will vary depending on the speed of the device
 * processor, the speed it is run at when Picsel operates and the bandwidth
 * that other system services use while Picsel operate.
 *
 * The supported range of values is 0..MAX_INT. For example:
 * - 0 means do not enable adaptive redraw (equivalent to
 *   @ref PicselConfigFV_RedrawMode_Fuzzy).
 * - 32768 would be suitable for a processor running at 100MHz & a QCIF
 *   screen or a processor running at 150MHz and a QVGA screen.
 * - 65536 would be suitable for a processor running at 200MHz and a QCIF
 *   screen
 *
 * The values may also be affected by the language(s) supported by a device.
 * A low value may be necessary for any of Korean, Japanese or Chinese.
 *
 * This value needs to be carefully tuned for each device.  Setting too high
 * a value will result in poor performance and screen "tearing" (where
 * different areas of the screen update at different times
 */
#define PicselConfigFV_adaptiveRedraw         "RendererCostLimit"

/**
 * Width of the High Quality Rendering Cache
 *
 * For devices with sufficient memory and resource, a cache bigger than the
 * screen can be kept in order to speed up panning and remove the need for
 * fuzzy redrawing for sufficiently small movements.
 *
 * This cache is particularly useful for touchscreen devices and is updated
 * when the pen is lifted up after a pan (so when the pen comes down again
 * the cache is ready for the next pan), or when the contents are invalidated
 * by either the content changing or a zoom occurring.
 *
 * This value is the width of the cache, in pixels.  It must be at least as
 * big as the screen, or else the cache is not created.  It can be as large
 * as required, but it should be noted that large values will require
 * considerably more memory and there can be a more noticeable pause when the
 * cache is updated.
 *
 * The default value is 0, which means the cache will not be present.  This
 * is the most sensible solution in most cases (limited memory).  If a cache
 * is required, it is recommended a value of 3 times the screen width is
 * picked, in order to have a full screen width each side of the currently
 * rendered screen, which is almost as much as can be reached with a single
 * pen stroke.
 */
#define PicselConfigFV_hqCacheWidth          "Picsel_HQCacheWidth"


/**
 * Height of the High Quality Rendering Cache
 *
 * This value is the height of the High Quality Rendering Cache.  For more
 * details of the High Quality Rendering Cache, please see
 * @ref PicselConfigFV_hqCacheWidth.
 *
 * If this value is smaller than the height of the screen, then no cache is
 * used.  The default value is 0, which means no cache is used.  This is
 * sensible on most devices, as memory requirements for such a cache can be
 * very demanding.
 *
 * If a cache is required, it is recommended this value is set to 3 times the
 * screen height.  This allows a full screen of cache above and below the
 * content currently on-screen, which is about as much as can be reached with
 * a single pen stroke.
 */
#define PicselConfigFV_hqCacheHeight         "Picsel_HQCacheHeight"


/**
 * Mode whereby the contents of the High Quality Rendering Cache are scaled
 * to the screen during a zoom operation.  Note: This mode is an optional
 * feature which may not be supported by the build.
 *
 * The default value is 0, which means that this mode will not be enabled.
 * Setting the value to something other than 0 will result in the mode being
 * enabled, provided it is supported in the build.
 */
#define PicselConfigFV_useHqCacheZoom       "Picsel_UseHQCacheZoom"


/**
 * State the minimum limit that a document can be zoomed to. Set this using
 * PicselConfig_setInt().
 *
 *
 * The Picsel engine will never allow the page to be zoomed out further
 * than necessary to see the whole of the current page.
 *
 * When @ref FlowMode_FitScreenWidth or @ref FlowMode_PowerZoom is set, the
 * minimum zoom will be limited to showing the page at 100%.
 *
 * The supported range of values is 10..1000, although numbers high in
 * that range may result in undesirable behaviour. The minimum supported
 * scale, therefore, is 10% (and the maximum is 1000%). Attempts to
 * set limits beyond these will be rounded to the supported limits.
 *
 * @see @ref TgvZoom_Magnification.
 */

#define PicselConfigFV_zoomLimitMin           "FV_ZoomLimitMin"

/**
 * State the maximum limit (unsigned long integer between 10 & 1000) that
 * a document can be zoomed to.
 *
 * The supported range of values is 10..1000, although numbers low in
 * that range may result in undesirable behaviour. The minimum supported
 * scale, therefore, is 10% (and the maximum is 1000%). Attempts to
 * set limits beyond these will be rounded to the supported limits. Set this
 * using PicselConfig_setInt().
 *
 * @see @ref TgvZoom_Magnification.
 */
#define PicselConfigFV_zoomLimitMax           "FV_ZoomLimitMax"

/**
 * State the percentage amount that individual zoom-in key events will
 * zoom by.
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvZoom_Magnification.
 */
#define PicselConfigFV_zoomInAmount           "FV_zoomInAmount"

/**
 * State the percentage amount that individual zoom-out key events will zoom
 * by.
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvZoom_Magnification.
 */
#define PicselConfigFV_zoomOutAmount          "FV_zoomOutAmount"

/**
 * Set the percentage amount that zoom-in key repeat events will zoom by. See
 * @ref TgvZoom_Magnification.
 *
 * This will be used when PicselApp_keyPress() is called repeatedly
 * before a call to PicselApp_keyRelease() .
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvZoom_Magnification.
 */
#define PicselConfigFV_zoomInRepeatAmount     "FV_zoomInRepeatAmount"

/**
 * Set the percentage amount that zoom-out key repeat events will zoom by.
 *
 * This will be used when PicselApp_keyPress() is called repeatedly
 * before a call to PicselApp_keyRelease() .
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvZoom_Magnification.
 */
#define PicselConfigFV_zoomOutRepeatAmount    "FV_zoomOutRepeatAmount"

/**
 * Zoom level (percentage) when a document is first loaded. See @ref
 * TgvZoom_Magnification.
 *
 * This is the percentage that a document, still in @ref
 * PicselConfigFV_RedrawMode "fuzzy" mode, will be zoomed to.
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvZoom_Magnification.
 */
#define PicselConfigFV_initialZoom            "FV_InitialZoom"

/**
 * Set the percentage amount that individual zoom-in key events will zoom
 * by, in PowerZoom flow mode only.
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvZoom_Magnification.
 */
#define PicselConfigFV_powerZoomInAmount        "FV_powerZoomInAmount"

/**
 * Set the amount that individual zoom-out key events will zoom by, in
 * PowerZoom flow mode only.
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvZoom_Magnification.
 */
#define PicselConfigFV_powerZoomOutAmount       "FV_powerZoomOutAmount"

/**
 * Set the percentage amount that zoom-in key repeat events will zoom by, in
 * PowerZoom flow mode only.
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvZoom_Magnification.
 */
#define PicselConfigFV_powerZoomInRepeatAmount  "FV_powerZoomInRepeatAmount"

/**
 * Set the percentage amount that zoom-out key repeat events will zoom by,
 * in PowerZoom flow mode only.
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvZoom_Magnification.
 */
#define PicselConfigFV_powerZoomOutRepeatAmount "FV_powerZoomOutRepeatAmount"

/**
 * Set the amount, in screen pixels, that a key event will pan (scroll) up
 * by. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvPanning for general information and @ref TgvPanning_key for details
 * about sign, direction and default value.
 */
#define PicselConfigFV_panUpAmount              "FV_panUpAmount"

/**
 * Set the amount, in screen pixels, that a key event will pan (scroll)
 * down by. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvPanning for general information and @ref
 * TgvPanning_key for details about sign, direction and default value.
 */
#define PicselConfigFV_panDownAmount            "FV_panDownAmount"

/**
 * Set the amount, in screen pixels, that a key event will pan (scroll) left
 * by.
 *
 * Set this using PicselConfig_setInt().
 *
 * See @ref TgvPanning for general information and @ref TgvPanning_key for
 * details about sign, direction and default value.
 */

#define PicselConfigFV_panLeftAmount          "FV_panLeftAmount"

/**
 * Set the amount, in screen pixels, that a key event will pan (scroll) right
 * by. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvPanning for general information and @ref TgvPanning_key for
 * details about sign, direction and default value.
 */

#define PicselConfigFV_panRightAmount         "FV_panRightAmount"

/**
 * Set the amount, in screen pixels, that a key repeat event will pan (scroll)
 * up by.
 *
 * This will be used when PicselApp_keyPress() is called repeatedly
 * before a call to PicselApp_keyRelease() .
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvPanning for general information and @ref TgvPanning_key for
 * details about sign, direction and default value.
 */
#define PicselConfigFV_panUpRepeatAmount      "FV_panUpRepeatAmount"

/**
 * Set the amount, in screen pixels, that a key repeat event will pan (scroll)
 * down by.
 *
 * This will be used when PicselApp_keyPress() is called repeatedly
 * before a call to PicselApp_keyRelease() .
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvPanning for general information and @ref TgvPanning_key for
 * details about sign, direction and default value.
 */
#define PicselConfigFV_panDownRepeatAmount    "FV_panDownRepeatAmount"

/**
 * Set the amount, in screen pixels, that a key repeat event will pan (scroll)
 * left by.
 *
 * This will be used when PicselApp_keyPress() is called repeatedly
 * before a call to PicselApp_keyRelease() .
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvPanning for general information and @ref TgvPanning_key for
 * details about sign, direction and default value.
 */
#define PicselConfigFV_panLeftRepeatAmount    "FV_panLeftRepeatAmount"

/**
 * Set the amount, in screen pixels, that a key repeat event will pan (scroll)
 * right by.
 *
 * This will be used when PicselApp_keyPress() is called repeatedly
 * before a call to PicselApp_keyRelease() .
 *
 * This must be an unsigned long integer, representing a 16.16
 * fixed point number. Set this using PicselConfig_setInt().
 *
 * @see @ref TgvPanning for general information and @ref TgvPanning_key for
 * details about sign, direction and default value.
 */
#define PicselConfigFV_panRightRepeatAmount   "FV_panRightRepeatAmount"

/**
 * Set the pan (scroll) to page top/bottom enable flag: 1 = set, 0 = unset.
 * Set this using PicselConfig_setInt().
 *
 * If the pan to page enable flag is set, any attempt to pan over a
 * page border, top or bottom, results in the previous or next page of the
 * document respectively being displayed, if it exists.
 *
 * For example, if a user is viewing page 4 of a document and tries to pan
 * over the top page border, page 3 will be displayed if this setting is
 * enabled. Similar panning on page 1 would have no effect.
 *
 * This is independent of @ref PicselConfigFV_panLimitsReporting setting.
 *
 * @see @ref PicselConfigFV_panToPageLeftRight.
 */
#define PicselConfigFV_panToPageEnable        "FV_panToPageEnable"

/**
 * Set the pan (scroll) to page left/right enable flag: 1 = set, 0 = unset.
 * Set this using PicselConfig_setInt().
 *
 * If the pan to page enable flag is set, any attempt to pan over a
 * page border, left or right, results in the previous or next page of the
 * document respectively being displayed, if it exists.
 *
 * For example, if a user is viewing page 4 of a 6 page document and tries to
 * pan over the right page border, page 5 will be displayed if this setting
 * is enabled. Similar panning on a 4 page document would have no effect.
 *
 * This is independent of @ref PicselConfigFV_panLimitsReporting setting.
 *
 * @see @ref PicselConfigFV_panToPageEnable.
 */
#define PicselConfigFV_panToPageLeftRight     "FV_panToPageLeftRight"

/**
 * Set the pan border width.
 *
 * The pan border width is the amount of background (in pixels) that can
 * be seen outside the page before panning stops. Set this using
 * PicselConfig_setInt().
 */
#define PicselConfigFV_panBorderWidth         "FV_panBorderWidth"

/**
 * Configure the inertial scrolling behaviour
 *
 * Inertial scrolling recalculates the page's velocity at regular intervals.
 * The following equation is used to calculate the velocity of the page:
 *
 * V[n] = V[n-1] - ((decay[2] * V[n-1] * V[n-1]) + (decay[1] * V[n-1]) + decay[0]);
 *
 * Where:
 *   V[n]        = the new velocity
 *   V[n-1]      = the previous velocity
 *   decay[0..2] = tunable constants
 *
 * The decay values can be tuned to change the behaviour of the inertial
 * motion.  Larger values will make the velocity decay more quickly.  Lower
 * values will make the velocity decay more slowly.
 *
 * Suitable values are in the range 0 to 512
 *
 * Default value is 32
 *
 * @see @ref PicselConfigFV_inertialScrollingDecay1
 *      @ref PicselConfigFV_inertialScrollingDecay2
 */
#define PicselConfigFV_inertialScrollingDecay0   "FV_inertialScrollingDecay0"

/**
 * Configure the inertial scrolling behaviour
 *
 * See @ref PicselConfigFV_inertialScrollingDecay0 for more information
 *
 * Suitable values are in the range 0 to 512
 *
 * Default value is 64
 *
 * @see @ref PicselConfigFV_inertialScrollingDecay0
 *      @ref PicselConfigFV_inertialScrollingDecay2
 */
#define PicselConfigFV_inertialScrollingDecay1   "FV_inertialScrollingDecay1"

/**
 * Configure the inertial scrolling behaviour
 *
 * See @ref PicselConfigFV_inertialScrollingDecay0 for more information
 *
 * Suitable values are in the range 0 to 512
 *
 * Default value is 128
 *
 * @see @ref PicselConfigFV_inertialScrollingDecay0
 *      @ref PicselConfigFV_inertialScrollingDecay1
 */
#define PicselConfigFV_inertialScrollingDecay2   "FV_inertialScrollingDecay2"

/**
 * Sets the way page fitting works when a document is first loaded.
 *
 * This value also sets the way page fitting works when a user changes page
 * if @ref PicselConfigFV_applyFitOnPageChange is enabled.
 *
 * Valid values are listed in @ref PicselConfigFV_PageFit_Values. Set this
 * using PicselConfig_setInt().
 *
 * If this value is not set, the document will default to opening fitted to
 * screen. This usually means fitting to height to show the whole page on
 * screen at once.
 *
 * If it is set to @ref PicselConfigFV_PageFit_None, the document will be
 * shown with 100% zoom and will not be centered on the screen.
 *
 * @see @ref PicselConfig_flowModeKeepZoom.
 */
#define PicselConfigFV_pageFitDefault         "FV_pageFitDefault"

/**
 * Enumeration values for PicselConfigFV_pageFitDefault
 */
typedef enum PicselConfigFV_PageFit_Values
{
    PicselConfigFV_PageFit_Screen     = 0,
    PicselConfigFV_PageFit_Width      = 1,
    PicselConfigFV_PageFit_None       = 2,
    PicselConfigFV_PageFit_ForceWidth = 65537
    /**< Do not use; this is to force enum into a 32 bit value type */
}
PicselConfigFV_PageFit_Values;

/**
 * Zoom in/out so that a new page fits the screen. Set this using PicselConfig_setInt().
 *
 * The page fitting behaviour, and so the amount of zoom, will be as defined
 * in @ref PicselConfigFV_pageFitDefault, unless overridden by a call to
 * PicselApp_fitDocument().
 *
 * - 0 = Do not change the zoom level when changing page.
 * - 1 = Zoom in/out, as appropriate, when changing page (default).
 */
#define PicselConfigFV_applyFitOnPageChange      "FV_applyFitOnPageChange"

/**
 * Set the display manager buffer size.
 *
 * The display manager buffer size (in bytes) controls the amount of memory
 * available to the display manager to render pages. It is most closely
 * related to screen size, not the document complexity.
 *
 * Altering this value can affect the speed of loading pages with large
 * bitmaps. Large values speed up render times.
 *
 * A value in the range (75 * 1024) - (150 * 1024) is appropriate for most
 * platforms. Set this using PicselConfig_setInt().
 */
#define PicselConfigFV_dispmanBufferSize      "DispmanPageBufferSize"

/**
 * Sets number of steps pan key operations will take.
 *
 * Value should be 1-20.  Larger values make panning appear smoother whilst
 * smaller values make panning more responsive. Set this using
 * PicselConfig_setInt().
 *
 * If this is not set, the default is 1.
 */
#define PicselConfigFV_panIterations         "FV_panIterations"

/**
 * Sets number of steps powerZoom pan key operations will take. Set this
 * using PicselConfig_setInt().
 *
 * If this is not set, the default is 1.
 */
#define PicselConfigFV_powerZoomPanIterations   "FV_powerZoomPanIterations"

/**
 * Sets number of steps zoom key operations will take.
 *
 * Value should be 1-20.  Larger values make zooming appear smoother whilst
 * smaller values make zooming more responsive. Set this using
 * PicselConfig_setInt().
 *
 * If this is not set, the default is 3.
 */
#define PicselConfigFV_zoomIterations        "FV_zoomIterations"

/**
 * Sets number of steps zoom key operations will take in PowerZoom flow mode.
 *
 * Value should be 1-20.  Larger values make zooming appear smoother whilst
 * smaller values make zooming more responsive. Set this using
 * PicselConfig_setInt().
 *
 * If this is not set, the default is 1.
 */
#define PicselConfigFV_powerZoomIterations   "FV_powerZoomIterations"

/**
 * Sets whether a page will zoom proportionately after resizing the screen.
 *
 * - 0 = leave page as it is.
 * - 1 = zoom (default)
 * - 2 = fit to width - If the page was fitted to width before the resize,
 *       or is narrower than the screen after resize, fit to width.  Otherwise
 *       maintain the current zoom level.
 *
 * If 0 is specified, the xTopLeft and yTopLeft values are used to calculate
 * how much the page should be panned in order to leave it in the same
 * position on the physical screen. It is essential that these values are
 * provided correctly, otherwise this will not function properly. Set this
 * using PicselConfig_setInt().
 */
#define PicselConfigFV_zoomAfterResize        "FV_zoomAfterResize"

/**
 * Sets whether a @ref AlienInformation_Zoom event will be sent after each
 * zoom operation performed.
 *
 * - 0 = no event will be sent (default)
 * - 1 = an @ref AlienInformation_Zoom event will be sent after each zoom operation.
 *
 * Set this using PicselConfig_setInt().
 *
 */
#define PicselConfigFV_zoomNotify             "FV_zoomNotify"

/**
 * Allow text edge tuning to be specified. 1 = set, 0 = unset.
 *
 * Set this using PicselConfig_setInt().
 *
 * Edge tuning allows the Alien application to compensate for variations of
 * brightness, contrast and gamma in screen display characteristics. These
 * must be determined for particular screen types, not for individual phones,
 * as the values are initialised during each application start and remain
 * fixed while the application is running.
 *
 * These parameters have the greatest effect with large numbers of edges on
 * the screen. Typically, this is seen when zooming out from a page
 * containing a large volume of text.
 *
 */
#define PicselConfigFV_textEdgeTuningAdjustEnable "FV_textEdgeTuningAdjustEnable"

/**
 * Sets the initial intensity value for text edge edge tuning and is
 * equivalent to gamma correction.
 *
 * The range is 0 to 131072, a 16.16 fixed
 * point value, where 131072 represents a gamma of 2.0. The default setting
 * is 65536 and represents a gamma of 1.0. Set this using
 * PicselConfig_setInt().
 *
 * This parameter is only active if edge tuning is enabled. See @ref
 * PicselConfigFV_textEdgeTuningAdjustEnable.
 */
#define PicselConfigFV_textEdgeTuningIntensity   "FV_textEdgeTuningIntensity"

/**
 * Sets the brightness value for text edge tuning.
 *
 * The range is -255 to 255 and higher values increase the brightness of the
 * display. The default value is 64. Set this using PicselConfig_setInt().
 *
 * This parameter is only active if edge tuning is enabled. See @ref
 * PicselConfigFV_textEdgeTuningAdjustEnable.
 */
#define PicselConfigFV_textEdgeTuningBrightness  "FV_textEdgeTuningBrightness"

/**
 * Sets the contrast value for text edge tuning.
 *
 * The range is -255 to 255. Higher values increase the constrast of the
 * display. The default value is 32. Set this using PicselConfig_setInt().
 *
 */
#define PicselConfigFV_textEdgeTuningContrast    "FV_textEdgeTuningContrast"

/**
 * Configure whether PDF and PowerPoint documents are fully loaded or
 * only have the current page loaded.  Fully loading allows rapid changes
 * between pages but single page load allows more content to be displayed
 * from large documents.
 *
 * This setting can be changed many times during one application session.
 * If set after AlienConfig_ready, the current document will have to be
 * reloaded before the change in policy takes effect.
 *
 * The default value is 0 (off); set to 1 to enable this. Set this using
 * PicselConfig_setInt().
 *
 */
#define PicselConfigFV_singlePageLoad            "FV_singlePageLoad"

/**
 * Change from static to dynamic subsampling of PDF documents.
 *
 * If this is set, the Picsel Library will either include images at their
 * original resolution, or subsample to a lower resolution, if memory is
 * nearly exhausted during document loading. Subsampling reduces the
 * resolution of bitmap images in PDF documents according to available
 * memory. Set this using PicselConfig_setInt().
 *
 * In static subsampling mode, the level of subsampling is chosen
 * according to the size of the original image, irrespective of how much
 * memory is available.
 *
 * In dynamic subsampling mode, the level of subsampling is adapted according
 * to how much memory is available. It will generally result in big images
 * displaying to a higher quality, and will also allow images that would not
 * normally be able to be displayed to appear (although at a lower quality).
 *
 * The disadvantage of dynamic subsampling is that, occasionally, it can
 * cause "screen-tearing", where a document is drawn at two different
 * sub-sampling rates
 *
 * - 0 enables static subsampling (default).
 * - 1 enables dynamic subsampling.
 *
 * Dynamic subsampling only affects PDF documents.
 *
 *  @see @ref PicselConfigFV_StaticSubsampleThresholdOverride.
 */
#define PicselConfigFV_DynamicSubsampling      "DynamicSubsampling"

/**
 * Static subsampling threshold in pixels.
 *
 * In static subsampling mode, Images are subsampled (where supported) until
 * their area is less than the square of this threshold. In effect, this is
 * the width that images should be reduced to (where reduction is possible).
 * Picsel recommend that this be set to approximately the width of the
 * screen of the target device.
 *
 * If not set, or set to 0, then the Picsel code will try and pick the most
 * appropriate value. Set this using PicselConfig_setInt().
 *
 * This setting applies for file formats that do not support dynamic
 * subsampling, or when dynamic subsampling is disabled by setting @ref
 * PicselConfigFV_DynamicSubsampling.
 */
#define PicselConfigFV_StaticSubsampleThresholdOverride \
                                         "FV_StaticSubsampleThresholdOverride"

/**
 * Sets the colour of the background upon which a page is displayed, similar
 * to the "desktop" you might see on a PC. Set this using
 * PicselConfig_setInt().
 *
 * The colour is specified as an integer combined red/green/blue/alpha
 * (opacity) as follows: 0xRRGGBBaa.
 *
 * The alpha (opacity) value (the least significant byte) has no meaning in
 * the context of page background and is therefore ignored.
 *
 * The default value is 0x808080ff, which is mid grey.
 *
 * The macro @ref PicselConfig_rgbaFromComponents can be used to specify the
 * colour.
 */
#define PicselConfigFV_bgColour               "FV_bgColour"

/**
 * Set to switch on/off pictures displaying in web pages or PDF documents
 *
 * The default value is 1 (on); set to 0 (off) to disable this. Set this
 * using PicselConfig_setInt().
 */
#define PicselConfigFV_showPics               "EnableImages"

/**
 * Toggle whether small images are fitted to screen. "Small" is defined
 * as an original size smaller than the screen and needing a zoom-in to
 * fit to screen.
 *
 * The default value is 1 (on); set to 0 (off) to leave images at their
 * original size.
 */
#define PicselConfigFV_fitSmallPics           "FitSmallImages"

/**
 * Sets (clamps) the mimimum screen-pixels a pan and pan-repeat can step.
 *
 * On PowerZoom, the pan amount (as set by @ref PicselConfigFV_panUpAmount
 * et al) is scaled by the current zoom level.
 *
 * The default value is 0 (no limit). The units are the same as @ref
 * PicselConfigFV_panUpAmount
 */
#define PicselConfigFV_panVertMin              "FV_panVertMin"

/**
 * Sets the maximum number of screen pixels a pan and pan-repeat can
 * step. This step size determines panning speed. On PowerZoom, the
 * panning step size (as set by @ref
 * PicselConfigFV_panUpAmount et al) is first scaled by the current
 * zoom level and then limited by this value.
 *
 * The default value is 2^31-1 (equivalent to no limit). Set this using
 * PicselConfig_setInt().
 */
#define PicselConfigFV_panVertMax              "FV_panVertMax"

/**
 * Specifies whether a search will centre a result if it is already visible
 * onscreen.
 *
 * If it is 1 (on), searching or snapping to a result will always pan to
 * centre the result onscreen, provided this doesn't conflict with document
 * limits. (Results near the edge of the page may not be centred)
 *
 * If it is 0 (off = default), a result already visible will not be panned.
 * If the result is not already visible, the behaviour will be the same as
 * when this feature is on (as above). Set this using PicselConfig_setInt().
 *
 * This feature will have little effect when the flow mode is @ref
 * FlowMode_FitScreenWidth, as the pan limits are much stricter.
 */
#define PicselConfigFV_alwaysCentreSearchResult "FV_alwaysCentreSearchResult"

/**
 * Specifies whether the screen update will block if the splash screen is
 * suppressed on startup.
 *
 * If it is 1 = set, and the splash screen is suppressed on startup, then
 * screen updates will block until the document is loaded completely. This
 * feature has no effect when the splash screen is not suppressed on startup.
 *
 * The default is 0 = unset.
 */
#define PicselConfigFV_blockScreenUpdatesOnStartup "FV_blockScreenUpdatesOnStartup"

/**
 * Values that @ref PicselConfigFV_panLimitsReporting can be set to.
 */
enum
{
    /**
     * No reporting; @ref AlienInformation_PanLimitsReached will never be
     * sent.
     */
    PicselConfigFV_PanLimitsReporting_None  = (1<<16),

    /**
     * Will report a pan limit as being reached if the edge of the document
     * is reached.
     */
    PicselConfigFV_PanLimitsReporting_PanOnly,

    /**
     * Will report a pan limit as being reached if the edge of the document
     * is reached, and will also report a focus limit as being reached if the
     * edge of the document is reached and there are no further focusable
     * items in that direction.
     */
    PicselConfigFV_PanLimitsReporting_PanAndFocus
};

/**
 * Controls the behaviour of pan limits notifications. These notifications
 * are sent to @ref AlienEvent_information, with the event @ref
 * AlienInformation_PanLimitsReached.
 *
 * These notifications are independent of @ref PicselConfigFV_panToPageEnable
 * and @ref PicselConfigFV_panToPageLeftRight.
 *
 * There is a neglible performance cost for switching on pan-only
 * reporting with @ref PicselConfigFV_PanLimitsReporting_PanOnly.
 * However, focus reporting @ref PicselConfigFV_PanLimitsReporting_PanAndFocus
 * is more processor intensive.
 *
 * Because of the relative processing costs, the default is @ref
 * PicselConfigFV_PanLimitsReporting_PanOnly. Change this using
 * PicselConfig_setInt().
 */
#define PicselConfigFV_panLimitsReporting "FV_panLimitsReporting"

/**
 * Sets whether the page should be panned to show the whole of an object
 * which was highlighted while partially offscreen.
 *
 * This could alter the behaviour of, for example, a zoomed webpage
 * containing a form, where the Alien Application calls
 * PicselFocus_navigateToItem() in order to direct the end users browswer to
 * a particular field in the form.
 *
 * The default value is 0 (off); set to 1 (on) to enable. Set this using
 * PicselConfig_setInt().
 */
#define PicselConfigFV_panFocusObjsOnscreen "FV_panFocusObjsOnscreen"

/**
 * Send notifications of the keyboard input focus state to the Alien
 * application after every key release.
 *
 * If this is set to 1, the Picsel library will inform the Alien application
 * of which item in the document currently has the focus using @ref
 * AlienInformation_FocusInformation, after each key release. This may help
 * in situations where key presses may move the focus between items, such as
 * in browser navigation. There will be a small cost involved, in terms of
 * CPU usage, in processing this event.
 *
 * - The user presses a key.
 * - Alien Application calls PicselApp_keyPress()
 * - PicselLibrary performs appropriate actions (e.g. begin zooming/panning
 *   etc)
 * - The user releases the key.
 * - Alien Applicaton calls PicselApp_keyRelease()
 * - Picsel performs the appropriate actions (eg stop zooming, stop panning
 *   etc).
 * - Picsel calls AlienEvent_information(), with eventType as @ref
 *    AlienInformation_FocusInformation, and eventData will be a pointer to a
 *    @ref PicselFocus_Information struct
 *
 * The default value is 0 (off); set to 1 (on) to enable, using
 * PicselConfig_setInt()
 */
#define PicselConfigFV_outputFocusInfoAfterKeyRelease  "FV_outputFocusInfoAfterKeyRelease"


/**
 * Controls how much of the document is selected for editing when the user
 * indicates a position on screen.
 *
 * This must be a numeric value from @ref PicselEdit_SelectMode, set using
 * PicselConfig_setInt().
 *
 * Selection can be made by the Alien application specifying a
 * screen position or by Picsel's emulated mouse being passed a select event.
 * If the mouse is used, selection is assumed to be at its top left point.
 *
 * @see PicselEdit_selectXY()
 *      @ref PicselEdit_SelectMode
 */
#define PicselConfigFV_editCursorMode       "EditCursorMode"

/**
 * Controls how large documents will be aligned on the screen. The definition
 * of "large" in this context means a document page width greater than twice
 * the screen width. (It does not refer to the number of pages.)
 *
 * The default setting (0 = off) aligns the top-centre of large document
 * pages with the top-centre of the screen.
 *
 * Enabling this option (1 = on) aligns the top-left corner of large document
 * pages with the top-left corner of the screen.
 */
#define PicselConfig_leftAlignLargeDocs       "FV_leftAlignLargeDocs"

/**
 * @}
 */

/**
 * @addtogroup TgvConfigureCore
 * &nbsp;
 *
 * See @ref TgvConfigFileViewer.
 */

#endif /* !PICSEL_CONFIG_FILEVIEWER_H */
