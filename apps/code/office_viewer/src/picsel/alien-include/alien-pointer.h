/**
 * Mouse Pointer Control
 *
 * These functions must be implemented by the application integrator, before
 * the application can be linked with the Picsel library. If these features
 * are not required by the application, stub functions may be provided which
 * compile but do not work, and return errors. Please see the integration
 * guide section on optional function implementations for more information
 * about this process.
 *
 * @file
 * $Id: alien-pointer.h,v 1.11 2009/09/11 13:51:30 roger Exp $
 */
/* Copyright (C) Picsel, 2007-2008. All Rights Reserved. */
/**
 * @addtogroup TgvDevicePointer
 *
 * @details The Alien pointer API delegates the display and movement of
 * the pointer to the device platform. It is intended for devices with
 * their own pointer (i.e. mouse or joystick) and output hardware (i.e.
 * display driver with hardware cursor overlay).
 *
 * Alien Pointer functions like AlienPointer_setVisibility() refer
 * only to the displayed cursor (arrow). They are typically used in
 * the CUI Browser product to request that the device's cursor be
 * hidden while the application is in a non-interactive mode.
 *
 * The Picsel library may request that the pointer be displayed or
 * moved, for example to facilitate interaction with dialogue boxes.
 * The main purpose of the pointer API is to allow interactive user input
 * at specific screen coordinates, for example to select hypertext links
 * in a document.
 *
 * @{
 */

#ifndef ALIEN_POINTER_H
#define ALIEN_POINTER_H

#include "alien-types.h"

typedef enum PicselPointer_Visibility
{
    PicselPointer_Visible  = (1<<16),  /**< to change the pointer state to visible */
    PicselPointer_Invisible            /**< to change the pointer state to invisible */
}
PicselPointer_Visibility;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Shows or hides the device mouse pointer.
 *
 * Requests display of the device mouse pointer on screen, or hides it.
 * The actual display and movement should be handled by the device (not
 * by Picsel).
 *
 * @see AlienPointer_setPosition().
 *
 * @note This is not related to PicselCursor_show(), which is part of the
 *       emulated pointer API.
 *
 * @param[in]    alienContext    See @ref Alien_Context.
 * @param[in]    state           What the visibility of the pointer should be
 *
 */
void AlienPointer_setVisibility(Alien_Context            *alienContext,
                                PicselPointer_Visibility  state);

/**
 * Explicitly moves the device mouse pointer on screen.
 *
 * Requests that the device should move its mouse pointer. The device is
 * also expected to move the pointer directly under user instruction, for
 * example when the touchscreen is touched. Coordinates are measured
 * starting from 0, 0 as the top left coordinate, with positive coordinates
 * going right and down. Note that these coordinates are not affected by
 * the orientation of the Alien screen.
 *
 * @see AlienPointer_setVisibility()
 *
 * @param[in]    alienContext    See @ref Alien_Context.
 * @param[in]    x               What the X coordinate of the pointer should be
 * @param[in]    y               What the Y coordinate of the pointer should be
 *
 */
void AlienPointer_setPosition(Alien_Context            *alienContext,
                              unsigned int              x,
                              unsigned int              y);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_POINTER_H */
