/**
 * Functions for dealing with 16bpp bitmaps.
 *
 * $Id: picsel-pixelblock.h,v 1.20 2009/06/18 16:06:30 dpt Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @defgroup TgvConversionUtilities Conversion Utilities for Platform Formats
 * @ingroup TgvSystem
 *
 * The Picsel Library provides support for 16-bit colour displays. For devices
 * with different display formats, the Alien Application is responsible for
 * mapping between Picsel's format and the format available on the target
 * device.
 *
 * To aid this process the Picsel Library supplies the following functions,
 * which provide convenient conversion services between the formats used
 * internally by Picsel TGV, such as the UTF-8 character encoding and the
 * B5G6R5 pixel colour, and other formats which are occasionally used by device
 * hardware or other software.
 *
 * For example, Picsel_PixelBlock_convert_ycbcr420_r5g6b5() will convert a
 * block of bitmap data from Y-Cb-Cr 420 encoding to 16-bit RGB encoding.
 *
 * Each conversion could take a few milliseconds to complete, depending on
 * available resources on the device. For this reason, only call these
 * functions when absolutely necessary; probably as part of a call to
 * AlienScreen_update().
 *
 * @par Memory requirements
 *
 * The conversion functions provided require a pointer to the source bitmap
 * (*srcPtr) and also a pointer to a destination buffer (*destPr). To avoid
 * memory overruns, it is important to allocate the correct amount of memory
 * to destPtr.
 *
 * A useful formula for this is:
 * @code
 * bufferSizeInBytes = screenWidthPixels * screenHeightPixels * (bitsPerPixel / 8)
 * @endcode
 *
 * If the format of the target device is not listed below, the Alien
 * Application can perform the conversion by extending AlienScreen_update()
 * with new functionality.
 *
 * @par Some conventions used in these functions
 *
 * RGB, or BGR      -   The red, green and blue components making up the colour
 *                      of an individual pixel.
 *
 * 16 bit           -   Where a pixel is described as RGB or BGR (16 bit), the
 *                      bits are arranged as 5,6,5.
 *
 * bxgxrx or rxgxbx -   For example "b5g6r5"; indicates the number of bits
 *                      assigned for describing each component in little endian
 *                      order.
 *
 * Screen width     -   The width of the screen in pixels.
 *
 * Screen height    -   The height of the screen in pixels.
 *
 * @{
 */

#ifndef PICSEL_PIXELBLOCK_H
#define PICSEL_PIXELBLOCK_H

#include "alien-types.h"
#include "alien-screen.h"

/* Include old interface definitions, for backwards compatibility */
#include "alien-legacy.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Copy a patch, or region, from within a bitmap from one location to another.
 *
 * This is analogous to, within an image-editing application, dragging the
 * cursor over an area within an image and copying it to an area within the same
 * image.
 *
 * @param[in]  srcPtr       Source bitmap data.  Must not be null.
 * @param[in]  srcWidth     Source width in pixels.
 * @param[in]  srcHeight    Source height in pixels.
 * @param[in]  srcSpan      Source bytes per row.
 * @param[out] destPtr      Destination for bitmap data.  This must have
 *                          already been allocated by the caller.
 * @param[in] destSpan      Destination bytes per row.
 * @param[in] patchX        Patch X offset.
 * @param[in] patchY        Patch Y offset.
 * @param[in] patchW        Patch Width.
 * @param[in] patchH        Patch Height.
 * @param[in] rotation      Rotation value - any @ref PicselRotation value
 *                          except PicselRotationNoChange
 */
void Picsel_PixelBlock_copy_16bpp(const unsigned short *srcPtr,
                                  unsigned int          srcWidth,
                                  unsigned int          srcHeight,
                                  int                   srcSpan,
                                  unsigned short       *destPtr,
                                  int                   destSpan,
                                  unsigned int          patchX,
                                  unsigned int          patchY,
                                  unsigned int          patchW,
                                  unsigned int          patchH,
                                  PicselRotation        rotation);

/**
 * Translate a partial screen area into a rotated screen space.
 *
 * This is useful for identifying what area of the native screen has been
 * updated after doing a combined copy and rotate with
 * Picsel_PixelBlock_copy_16bpp(). Some platforms may require this knowledge
 * if a separate transfer to a real framebuffer is necessary.
 *
 * @param[in]  blockX            Screen X coordinate.
 * @param[in]  blockY            Screen Y coordinate.
 * @param[in]  blockWidth        Width of block updated.
 * @param[in]  blockHeight       Height of block updated.
 * @param[in]  physScreenWidth   Physical screen width.
 * @param[in]  physScreenHeight  Physical screen height.
 * @param[in]  rotation          Rotation value - one of @ref PicselRotation,
 *                               must not be NULL.
 *
 * @param[out] outX              Translated X.
 * @param[out] outY              Translated Y.
 * @param[out] outWidth          Translated width.
 * @param[out] outHeight         Translated height.
 */
void Picsel_PixelBlock_areaCoords(unsigned int    blockX,
                                  unsigned int    blockY,
                                  unsigned int    blockWidth,
                                  unsigned int    blockHeight,
                                  unsigned int    physScreenWidth,
                                  unsigned int    physScreenHeight,
                                  PicselRotation  rotation,
                                  unsigned int   *outX,
                                  unsigned int   *outY,
                                  unsigned int   *outWidth,
                                  unsigned int   *outHeight);

/**
 * Translate co-ordinates, from a rotated screen, back into co-ordinates
 * relative to the actual screen.
 *
 * This is used to translate the x and y co-ordinates of the current cursor
 * position as seen by the end user, into co-ordinates relative to the
 * device as understood by the Picsel Library.
 *
 * For example, the device user has their device rotated to landscape mode, and
 * touches the screen at x = 320, y = 0 (from their point of view).
 *
 * @verbatim
 ------------------------
 |*                     |
 |                      |
 |                      |
 |                      |
 |                      |
 ------------------------
 @endverbatim
 *
 * It may be useful for the Picsel Library to translate that into a touch-event
 * in the real top left-hand corner of the screen:
 * @verbatim
  ----------
  |*       |
  |        |
  |        |
  |        |
  |        |
  |        |
  ----------
 @endverbatim
 *
 * @code
 *      unsigned int x;
 *      unsigned int y;
 *
 *      Picsel_PixelBlock_pointCoords(
 *          tsEvent->x, //320 (rotated co-ordinate
 *          tsEvent->y, // 1
 *          globalAlienContext->physScreenW, // 320px
 *          globalAlienContext->physScreenH, // 480px
 *          PicselRotation270, // rotation is measured clockwise
 *          &x,
 *          &y);
 *
 *      // x = 0, y = 0
 *
 * @endcode
 *
 * @param[in]  inX               Input X coordinate.
 * @param[in]  inY               Input Y coordinate.
 * @param[in]  physScreenWidth   Physical screen width.
 * @param[in]  physScreenHeight  Physical screen height.
 * @param[in]  rotation          Rotation value - one of @ref PicselRotation,
 *                               must not be NULL.
 *
 * @param[out] outX              Translated X.
 * @param[out] outY              Translated Y.
 */
void Picsel_PixelBlock_pointCoords(unsigned int    inX,
                                   unsigned int    inY,
                                   unsigned int    physScreenWidth,
                                   unsigned int    physScreenHeight,
                                   PicselRotation  rotation,
                                   unsigned int   *outX,
                                   unsigned int   *outY);

/**
 * Translate coordinates, relative to the device, into co-ordinates taking
 * into account the rotation of the device.
 *
 * This is used to translate the x and y co-ordinates of the current cursor
 * position as understood by the Picsel Library (for instance from
 * the properties of an event) into co-ordinates that take into account the
 * orientation, or rotation, of the Alien Application relative to the device.
 *
 * For example, the user has their device rotated to portrait (i.e. no
 * rotation), and touches the screen at x = 0, y = 0:
 *
 * @verbatim
  ----------
  |*       |
  |        |
  |        |
  |        |
  |        |
  |        |
  ----------
 @endverbatim
 *
 * However, if the device is rotated 90 degrees counter-clockwise, the cursor
 * should appear to the user to be in the top left hand corner... but in terms
 * of the physical screen, the cursor needs to be here:
 * @verbatim
  ----------
  |       *|
  |        |
  |        |
  |        |
  |        |
  |        |
  ----------
 @endverbatim
 *
 * @code
 *      unsigned int x;
 *      unsigned int y;
 *
 *      Picsel_PixelBlock_reversePointCoords(
 *          tsEvent->x,
 *          tsEvent->y,
 *          globalAlienContext->physScreenW, // 320px
 *          globalAlienContext->physScreenH, // 480px
 *          PicselRotation270, // rotation is measured clockwise
 *          &x,
 *          &y);
 *
 *      // x = 320, y = 0
 *
 * @endcode
 *
 * @param[in]  inX               Input X coordinate.
 * @param[in]  inY               Input Y coordinate.
 * @param[in]  physScreenWidth   Physical screen width.
 * @param[in]  physScreenHeight  Physical screen height.
 * @param[in]  rotation          Rotation value - one of @ref PicselRotation,
 *                               must not be NULL.
 *
 * @param[out] outX              Translated X.
 * @param[out] outY              Translated Y.
 */
void Picsel_PixelBlock_reversePointCoords(unsigned int    inX,
                                          unsigned int    inY,
                                          unsigned int    physScreenWidth,
                                          unsigned int    physScreenHeight,
                                          PicselRotation  rotation,
                                          unsigned int   *outX,
                                          unsigned int   *outY);

/**
 * Convert a block of data, formatted as BGR (16 bit), into a block
 * formatted in BGR 18-bit format (6 bits per component), padded to 32 bits.
 *
 * The lowest (least significant) bits of both input and output buffers are
 * the blue component.  The output buffer only uses the bottom 18 bits of
 * each pixel.
 *
 * @param srcPtr           Source bitmap data.  Must not be null.
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller.
 * @param width            Number of pixels per row.
 * @param height           Number of rows.
 * @param srcWidthBytes    Width in bytes of source row.
 * @param destWidthBytes   Width in bytes of destination row.
 */
void Picsel_PixelBlock_convert_b5g6r5_b6g6r6x14(
                                       const unsigned short *srcPtr,
                                             unsigned int   *destPtr,
                                             int             width,
                                             int             height,
                                             int             srcWidthBytes,
                                             int             destWidthBytes);


/**
 * Convert a block of data, formatted as BGR (16 bit), into a block of data
 * formatted as BGR (24 bit, 8 bits per component), padded to 32 bits.
 *
 * The lowest (least significant) bits of both input and output buffers are the
 * blue component.  The output buffer only uses the bottom 3 bytes of each
 * pixel.
 *
 * @param srcPtr           Source bitmap data.  Must not be null.
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller.
 * @param width            Number of pixels per row.
 * @param height           Number of rows.
 * @param srcWidthBytes    Width in bytes of source row.
 * @param destWidthBytes   Width in bytes of destination row.
 */
void Picsel_PixelBlock_convert_b5g6r5_b8g8r8x8(
                                      const unsigned short *srcPtr,
                                            unsigned int   *destPtr,
                                            int             width,
                                            int             height,
                                            int             srcWidthBytes,
                                            int             destWidthBytes);


/**
 * Convert a block of data, formatted as BGR (16 bit), into a block of data
 * formatted as BGR (24 bit, 8 bits per component). No padding is carried out.
 *
 * The lowest (least significant) bits of both input and output buffers are the
 * blue component.
 *
 * @param srcPtr           Source bitmap data.  Must not be null.
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller.
 * @param width            Number of pixels per row.
 * @param height           Number of rows.
 * @param srcWidthBytes    Width in bytes of source row.
 * @param destWidthBytes   Width in bytes of destination row.
 */
void Picsel_PixelBlock_convert_b5g6r5_b8g8r8(
                                      const unsigned short *srcPtr,
                                            unsigned int   *destPtr,
                                            int             width,
                                            int             height,
                                            int             srcWidthBytes,
                                            int             destWidthBytes);


/**
 * Convert a block of data, formatted as BGR (16 bit), into a block
 * of data formatted as RGB (24 bit, 8 bits per component). No padding is
 * carried out.
 *
 * The lowest bits of the input buffer are blue.
 *
 * The lowest bits of the output buffer are red.
 *
 * @param srcPtr           Source bitmap data.  Must not be null.
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller.
 * @param width            Number of pixels per row.
 * @param height           Number of rows.
 * @param srcWidthBytes    Width in bytes of source row.
 * @param destWidthBytes   Width in bytes of destination row.
 */
void Picsel_PixelBlock_convert_b5g6r5_r8g8b8(
                                      const unsigned short *srcPtr,
                                            unsigned int   *destPtr,
                                            int             width,
                                            int             height,
                                            int             srcWidthBytes,
                                            int             destWidthBytes);


/**
 * Convert a block of data, formatted as BGR (16 bit), into a block of data
 * formatted as grey (8 bit).
 *
 * The lowest bits of the input buffer are blue.
 *
 * This uses fixed weights of 0.299, 0.587 and 0.114.
 *
 * @param srcPtr           Source bitmap data.  Must not be null.
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller.
 * @param width            Number of pixels per row.
 * @param height           Number of rows.
 * @param srcWidthBytes    Width in bytes of source row.
 * @param destWidthBytes   Width in bytes of destination row.
 */
void Picsel_PixelBlock_convert_b5g6r5_g8(
                                   const unsigned short *srcPtr,
                                         unsigned char  *destPtr,
                                         int             width,
                                         int             height,
                                         int             srcWidthBytes,
                                         int             destWidthBytes);


/**
 * Convert a block of data, formatted as planar Y-Cb-Cr 420, to RGB (16 bit).
 *
 * @param srcPtr           Source bitmap data.  Must not be null.
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller.
 * @param width            Number of pixels per row.
 * @param height           Number of rows.
 * @param destWidthBytes   Width in bytes of destination row.
 */
void Picsel_PixelBlock_convert_ycbcr420_r5g6b5(
                                             unsigned char  *srcPtr,
                                             unsigned short *destPtr,
                                             int             width,
                                             int             height,
                                             int             destWidthBytes);



/**
 * Convert a block of data, formatted as BGR (16 bit), to planar Y-Cb-Cr 422
 * format.
 *
 * The Y, Cb and Cr planes are contiguous:
 *
 * destPtr                                     => Y_Plane
 *
 * destPtr + sizeof(Y_Plane)                   => Cb_Plane
 *
 * destPtr + sizeof(Y_Plane) + sizeof(Cb_Plane) => Cr_Plane
 *
 * or, visually:
 * @verbatim

destPtr<--- Y plane ----><---- Cb plane ----><---- Cr plane ---->end of data

@endverbatim
 *
 * All values are 8 bit.
 *
 * @param srcPtr           Source bitmap data.  Must not be null
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller
 * @param width            Number of pixels per row
 * @param height           Number of rows
 * @param srcWidthBytes    Width in bytes of source row
 */
void Picsel_PixelBlock_convert_b5g6r5_ycbcr422(
                                      const unsigned short *srcPtr,
                                            unsigned char  *destPtr,
                                            int             width,
                                            int             height,
                                            int             srcWidthBytes);
/**
 * Convert a block of data, formatted as BGR (16 bit), to Planar Y-Cb-Cr 420
 * format.
 *
 * The Y, Cb and Cr planes are contiguous:
 *
 * destPtr                                     => Y_Plane
 *
 * destPtr + sizeof(Y_Plane)                   => Cb_Plane
 *
 * destPtr + sizeof(Y_Plane) + sizeof(Cb_Plane) => Cr_Plane
 *
 * or, visually:
 * @verbatim

destPtr<--- Y plane ----><---- Cb plane ----><---- Cr plane ---->end of data

@endverbatim
 *
 * All values are 8 bit.
 *
 * @param srcPtr           Source bitmap data.  Must not be null
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller
 * @param width            Number of pixels per row
 * @param height           Number of rows
 * @param srcWidthBytes    Width in bytes of source row
 */
void Picsel_PixelBlock_convert_b5g6r5_ycbcr420(
                                      const unsigned short *srcPtr,
                                            unsigned char  *destPtr,
                                            int             width,
                                            int             height,
                                            int             srcWidthBytes);


/**
 * Convert a block of data, formatted as BGR (16 bit), to planar Y-Cb-Cr 444
 * (packed) format.
 *
 * The Y, Cb and Cr planes are contiguous:
 *
 * destPtr                                     => Y_Plane
 *
 * destPtr + sizeof(Y_Plane)                   => Cb_Plane
 *
 * destPtr + sizeof(Y_Plane) + sizeof(Cb_Plane) => Cr_Plane
 *
 * or, visually:
 * @verbatim

destPtr<--- Y plane ----><---- Cb plane ----><---- Cr plane ---->end of data

@endverbatim
 *
 * All values are 8 bit.
 *
 * @param srcPtr           Source bitmap data.  Must not be null.
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller.
 * @param width            Number of pixels per row.
 * @param height           Number of rows.
 * @param srcWidthBytes    Width in bytes of source row.
 * @param destWidthBytes   Width in bytes of destination row.
 */
void Picsel_PixelBlock_convert_b5g6r5_ycbcr444(
                                      const unsigned short *srcPtr,
                                            unsigned int   *destPtr,
                                            int             width,
                                            int             height,
                                            int             srcWidthBytes,
                                            int             destWidthBytes);


/**
 * Convert a block of data, formatted as RGB (16 bit), to UYVY format.
 *
 * @param srcPtr           Source bitmap data.  Must not be null.
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller.
 * @param width            Number of pixels per row.
 * @param height           Number of rows.
 * @param srcBufferWidth   Width in bytes of source row.
 * @param destBufferWidth  Width in bytes of destination row.
 */
void Picsel_PixelBlock_convert_b5g6r5_uyvy(
                                      const unsigned short *srcPtr,
                                            unsigned short *destPtr,
                                            int             width,
                                            int             height,
                                            int             srcBufferWidth,
                                            int             destBufferWidth);


/**
 * Convert a block of data, formatted in Y-Cb-Cr 422 format, to RGB 565 (16 bit).
 *
 * @param srcPtr           Source bitmap data.  Must not be null.
 * @param destPtr          Destination for converted bitmap data.  Must have
 *                         been pre-allocated by the caller.
 * @param width            Number of pixels per row.
 * @param height           Number of rows.
 */
void Picsel_PixelBlock_convert_ycbcr422_r5g6b5(
                                             unsigned char  *srcPtr,
                                             unsigned short *destPtr,
                                             int             width,
                                             int             height);

/**
 * Copy a patch, or region, within a block of data (formatted as RGB, 16bit) to another
 * space with optional rotation and scale. Scaling is implied by the difference
 * in size between source and destination.
 *
 * @param[in]  picselContext Set by AlienEvent_setPicselContext().
 * @param[in]  src           Source bitmap data.  Must not be null.
 * @param[in]  srcW          Source width in pixels.
 * @param[in]  srcH          Source height in pixels.
 * @param[in]  srcSpan       Source bytes per row.
 * @param[out] dst           Destination for bitmap data. Must have been
 *                           pre-allocated by the caller.
 * @param[in]  dstW          Destination width in pixels.
 * @param[in]  dstH          Destination height in pixels.
 * @param[in]  dstSpan       Destination bytes per row.
 * @param[in]  dstPX         Patch X offset.
 * @param[in]  dstPY         Patch Y offset.
 * @param[in]  dstPW         Patch Width.
 * @param[in]  dstPH         Patch Height.
 * @param[in]  rotation      Rotation value - one of @ref PicselRotation,
 *                           must not be NULL.
 */
void Picsel_PixelBlock_scale_16bpp(Picsel_Context *picselContext,
                                   const void     *src,
                                   unsigned int    srcW,
                                   unsigned int    srcH,
                                   int             srcSpan,
                                   void           *dst,
                                   unsigned int    dstW,
                                   unsigned int    dstH,
                                   int             dstSpan,
                                   unsigned int    dstPX,
                                   unsigned int    dstPY,
                                   unsigned int    dstPW,
                                   unsigned int    dstPH,
                                   PicselRotation  rotation);

/**
 * On the basis of a g8 alpha buffer blend one b5g6r5 colour buffer
 * with another
 *
 * @param[in] srcColour           Source colour data
 * @param[in] srcAlpha            Source alpha data
 * @param[in,out] dest            Target for blend
 * @param[in] srcColourWidthBytes Width in bytes of source colour data
 * @param[in] srcAlphaWidthBytes  Width in bytes of source alpha data
 * @param[in] width               Width of block in pixels
 * @param[in] height              Height of block in pixels
 * @param[in] destWidthBytes      Width of target in bytes
 */
void Picsel_PixelBlock_blend_b5g6r5(
                       const unsigned short *srcColour,
                       const unsigned char  *srcAlpha,
                             unsigned short *dest,
                             int             srcColourWidthBytes,
                             int             srcAlphaWidthBytes,
                             int             width,
                             int             height,
                             int             destWidthBytes);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_PIXELBLOCK_H */
