/**
 * Display Screen Output and configuration
 *
 * This file contains definitions and declarations needed for rotating and
 * resizing the screen
 *
 * All functions supplied by Picsel are prefixed by 'Picsel'. The integrator
 * will need to make calls to these functions as described in the Picsel
 * supplied documentation, but should not attempt to provide implementations
 * of them.
 *
 * $Id: picsel-screen.h,v 1.25 2009/09/11 13:51:30 roger Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/** @addtogroup TgvScreen
 *
 * @{
 */

#ifndef PICSEL_SCREEN_H
#define PICSEL_SCREEN_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Screen formats */
typedef enum PicselScreenFormat
{
     /** Unsupported.                 */
    PicselScreenFormat_g8       = 0,

     /** Unsupported.                 */
    PicselScreenFormat_r5g5b5x1,

    /** 16 bits per pixel RGB,  arranged like this: @n
   most significant bit --> @c rrrrrggggggbbbbb  <-- least significant bit.*/
    PicselScreenFormat_b5g6r5,

    /** Unsupported.                 */
    PicselScreenFormat_r8g8b8x8,

    /** Unsupported.                 */
    PicselScreenFormat_b4g4r4x4,

    /** Unsupported for display. Can be passed to PicselScreen_capture()
        to produce a screenshot in Y-Cb-Cr 444 format,
        with 32 bits per pixel.  The most significant 8 bits are unused.  */
    PicselScreenFormat_ycbcr444,

    /** Unsupported for display. Can be passed to PicselScreen_capture()
        to produce a screenshot in planar Y-Cb-Cr 420 format,
        with an average of 12 bits per pixel. */
    PicselScreenFormat_ycbcr420,

    /** Unsupported for display. Can be passed to PicselScreen_capture()
        to produce a screenshot in planar Y-Cb-Cr 422 format,
        with 16 bits per pixel. */
    PicselScreenFormat_ycbcr422,

    /** Unsupported for display. Can be passed to PicselScreen_capture()
        to produce a screenshot in UYVY format, with 16 bits per pixel.  */
    PicselScreenFormat_uyvy,

    /**  Force size to be 32-bit           */
    PicselScreenFormat_ForceSize   = (1<<16)
}
PicselScreenFormat;

/** Screen rotation */
typedef enum PicselRotation
{
    /**< Leave current rotation unchanged  */
    PicselRotationNoChange = 0,

    /**< Non-rotated screen - 0 degrees    */
    PicselRotation0,

    /**< 90 degrees clockwise rotation     */
    PicselRotation90,

    /**< 180 degrees clockwise rotation    */
    PicselRotation180,

    /**< 270 degrees clockwise rotation    */
    PicselRotation270,

    /**  Force size to be 32-bit           */
    PicselRotation_ForceSize   = (1<<16)
}
PicselRotation;


/**
 * Returns the size, in bytes, of a pixel with the given @c format.
 *
 * The supported formats are
 *  - @ref PicselScreenFormat_g8
 *  - @ref PicselScreenFormat_r5g5b5x1
 *  - @ref PicselScreenFormat_b5g6r5
 *  - @ref PicselScreenFormat_b4g4r4x4
 *  - @ref PicselScreenFormat_r8g8b8x8
 *
 * @param[in] format  The pixel format.
 *
 * @return  The size of the pixel, in bytes, or 0 if the @c format is
 *          unsupported.
 */
int PicselScreen_getScreenFormatSize(PicselScreenFormat format);

/**
 * Rotate the screen.
 *
 * Call this function when the Alien application detects that the user has
 * turned the device on its side.
 *
 * The new orientation will be reflected in future calls to
 * AlienScreen_update(). Because of the asynchronous architecture of TGV,
 * there could be further calls to AlienScreen_update() with the old
 * dimensions, even after this function has returned.
 *
 * If the document is set to reflow, resizing the screen will
 * cause the document to be reflowed. See @ref TgvFlowMode.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] rotation      Absolute rotation of screen
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 *           Later, @ref AlienInformation_ScreenResized will be sent by
 *           the Picsel library, indicating whether the screen buffer was
 *           actually resized or not.
 *           Also, if the document is set to reflow and the screen rotate
 *           succeeded an @ref AlienInformation_LayoutComplete event will be
 *           sent to the Alien application. See @ref TgvFlowMode.
 *
 * @see @ref TgvRotating_And_Resizing.
 */
int PicselScreen_rotate(Picsel_Context *picselContext,
                        PicselRotation  rotation);

/**
 * Set the  screen buffer to the given dimensions.
 *
 * The new dimensions will be reflected in future calls to
 * AlienScreen_update(). Because of the asynchronous architecture of TGV,
 * there could be further calls to AlienScreen_update() with the old
 * dimensions, even after this function has returned.
 *
 * If the document is set to reflow, resizing the screen will
 * cause the document to be reflowed. See @ref TgvFlowMode.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 *
 * @param[in] width         New width of the screen, in pixels.
 * @param[in] height        New height of the screen, in pixels.
 *
 * @param[in] xTopLeft      Typically 0.
 *                          Horizontal offset of the top left of the
 *                          screen in pixels.  This value will be
 *                          passed as the xTopLeft parameter
 *                          in AlienScreen_update().
 *
 * @param[in] yTopLeft      Typically 0.
 *                          Vertical offset of the top left of the
 *                          screen in pixels.  This value will be
 *                          passed as the yTopLeft parameter
 *                          in AlienScreen_update().
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 *           Later, @ref AlienInformation_ScreenResized will be sent by
 *           the Picsel library, indicating whether the screen buffer was
 *           actually resized or not.
 *           Also, if the document is set to reflow and the screen resize
 *           succeeded an @ref AlienInformation_LayoutComplete event will be
 *           sent to the Alien application. See @ref TgvFlowMode.
 *
 *
 * @see @ref TgvRotating_And_Resizing.
 *
 */
int PicselScreen_resize(Picsel_Context *picselContext,
                        int             width,
                        int             height,
                        int             xTopLeft,
                        int             yTopLeft);

/**
 * Update the orientation of the screen following the screen's physical
 * rotation.
 *
 * Call this function when the Alien application detects that the screen
 * has been turned on a pivot, or the configuration of the devices's hardware
 * has been altered in some other way to indicate that the user is viewing
 * the screen from a different angle.
 *
 * The new orientation will be reflected in future calls to
 * AlienScreen_update(). Because of the asynchronous architecture of TGV,
 * there could be further calls to AlienScreen_update() with the  old
 * orientation, even after this function has returned.
 *
 * If the document is set to reflow, resizing the screen will
 * cause the document to be reflowed. See @ref TgvFlowMode.
 *
 * @param[in] picselContext  Set by AlienEvent_setPicselContext().
 * @param[in] rotation       The rotation to apply to the screens.
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 *           Later, @ref AlienInformation_ScreenResized will be sent by
 *           the Picsel library, indicating whether the screen buffer was
 *           actually resized or not.
 *           Also, if the document is set to reflow and the screen rotation
 *           succeeded an @ref AlienInformation_LayoutComplete event will be
 *           sent to the Alien application. See @ref TgvFlowMode.
 *
 * @see @ref TgvRotating_And_Resizing.
 */
int PicselScreen_physicalRotation(Picsel_Context *picselContext,
                                  PicselRotation  rotation);

/**
 * Resize and rotate the screen in one operation
 *
 * Call this function when the Alien application detects that the user has
 * turned the device on its side, and as a result changes the are of the
 * screen available to the Picsel library.
 *
 * If the document is set to reflow, resizing the screen will
 * cause the document to be reflowed. See @ref TgvFlowMode.
 *
 * The new orientation and dimensions will be reflected in future calls to
 * AlienScreen_update(). Because of the asynchronous architecture of TGV,
 * there could be further calls to AlienScreen_update() with the old
 * orientation and dimensions, even after this function has returned.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] width         New width of screen, in pixels.
 * @param[in] height        New height of screen, in pixels.
 * @param[in] xTopLeft      New x co-ordinate of top left point of screen.
 * @param[in] yTopLeft      New y co-ordinate of top left point of screen.
 * @param[in] rotation      Absolute rotation of screen.
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.

 *           Later, @ref AlienInformation_ScreenResized will be sent by
 *           the Picsel library, indicating whether the screen buffer was
 *           actually resized or not.
 *           Also, if the document is set to reflow and the screen resize
 *           succeeded an @ref AlienInformation_LayoutComplete event will be
 *           sent to the Alien application. See @ref TgvFlowMode.
 *
 * @see @ref TgvRotating_And_Resizing.
 */
int PicselScreen_resizeRotate(Picsel_Context *picselContext,
                              int             width,
                              int             height,
                              int             xTopLeft,
                              int             yTopLeft,
                              PicselRotation  rotation);

/**
 * Captures the document view for saving screenshots.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext()
 * @param[in] dest          The destination buffer.  The Picsel library
 *                          assumes that this has been allocated with
 *                          sufficient space to hold the screen capture, in
 *                          the requested format. @c dest must not be freed
 *                          until AlienInformation_ScreenCaptureComplete is
 *                          sent by the Picsel library.  Because of the
 *                          asynchronous architecture of TGV, @c dest will
 *                          not contain the screenshot data until it is
 *                          returned to the Alien application in
 *                          @ref AlienInformation_ScreenCaptureData.dest.
 * @param[in] bufferSize    The size of the buffer allocated, in bytes.
 * @param[in] format        The format of the output image. Currently the
 *                          only supported formats are
 *                          - @ref PicselScreenFormat_ycbcr420.
 *                          - @ref PicselScreenFormat_ycbcr422.
 *                          - @ref PicselScreenFormat_ycbcr444.
 *                          - @ref PicselScreenFormat_uyvy.
 *                          - @ref PicselScreenFormat_b5g6r5.
 *
 * If the output format is @ref PicselScreenFormat_ycbcr420,
 * @ref PicselScreenFormat_ycbcr422, or @ref PicselScreenFormat_uyvy and
 * either (or both) of the screen dimensions is an odd number, the output
 * image is increased in size to make all dimensions even.  The new
 * row/column of pixels is a duplicate of the last one. The destination
 * buffer must be large enough to hold this.
 *
 * @return The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 *         Later, @ref AlienInformation_ScreenCaptureComplete
 *         will be sent by the Picsel library when the screenshot data is
 *         available. @ref AlienInformation_ScreenCaptureData.dest will be
 *         equal to @c dest and will contain the screen capture data.
 *         If @ref AlienInformation_ScreenCaptureData.bytesWritten is 0,
 *         then the screen capture has failed, possibly because the
 *         @c dest buffer passed to PicselScreen_capture() was too small.
 */
int PicselScreen_capture(Picsel_Context     *picselContext,
                         void               *dest,
                         int                 bufferSize,
                         PicselScreenFormat  format);

/**
 * Redraw the current Picsel library display.
 *
 * This function tells the Picsel library to redraw its screen buffer and
 * pass it to AlienScreen_update(). Because of the asynchronous architecture
 * of TGV, the Alien application must not assume when this will be completed.
 * The only notification that a redraw has happened is when
 * AlienScreen_update() is called by the Picsel library.
 * It is not possible to identify whether a particular call to
 * AlienScreen_update() was caused by a call to PicselScreen_redraw().
 *
 *
 * @param[in] context  Set by AlienEvent_setPicselContext()
 *
 * @see
 *   @ref TgvAsync_Queue.
 */
void PicselScreen_redraw(Picsel_Context *context);


/**
 * Notify the Picsel library that an
 * @ref AlienInformation_ChangeHwOrientation event has been received and
 * processed.
 *
 * After @ref AlienInformation_ChangeHwOrientation has been received,
 * the buffer passed to AlienScreen_update() will be oriented according
 * to the @ref AlienInformation_HwOrientation.newOrientation.
 * This allows the Alien application to make use of hardware support for the
 * rotation. If no hardware support is available, the alien application must
 * rotate the buffer in software.
 *
 * The uId passed to this function must be the same as that
 * passed with the @ref AlienInformation_ChangeHwOrientation event.
 *
 * @product CUI Photoviewer only.
 *
 * @param[in] picselContext  Set by AlienEvent_setPicselContext().
 * @param[in] uId            The @ref AlienInformation_HwOrientation.uId
 *                           sent with the
 *                           @ref AlienInformation_ChangeHwOrientation event.
 * @param newRotation        New orientation.  Should be the orientation
 *                           requested by the
 *                           @ref AlienInformation_ChangeHwOrientation event.
 * @param[in] success        1 if the Alien application can display the new
 *                             orientation,
 *                           0 if it cannot.
 *
 */
void PicselScreen_notifyHwOrientationChange(Picsel_Context *picselContext,
                                            PicselRotation  newRotation,
                                            int             uId,
                                            int             success);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_SCREEN_H */
