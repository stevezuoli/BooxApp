/**
 * Mouse Pointer Input
 *
 * The file picsel-cursor.h provides a different and incompatible
 * type of mouse pointer which is less commonly used.
 *
 * The functions in this file are offered by the TGV library for use by
 * the Alien application. If these features are not required by the
 * application, it is not necessary to call these.
 *
 * $Id: picsel-pointer.h,v 1.9 2009/09/11 13:51:30 roger Exp $
 * @file
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
/**
 * @defgroup TgvDevicePointer Device Pointer Input
 * @ingroup TgvCommand
 *
 * The Picsel pointer API allows user interaction at specific screen
 * coordinates.
 *
 * This is typically achieved using a touchscreen, joystick or
 * mouse. Functions like PicselPointer_down() indicate user input and
 * can be used whether or not a pointer (arrow) is displayed.
 *
 * @ifnot cui
 * See @ref TgvEmulatedCursor, which provides a different and incompatible
 * type of emulated mouse pointer.
 * @endif
 * @{
 */

#ifndef PICSEL_POINTER_H
#define PICSEL_POINTER_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 *  Enum for PicselPointer button Id.
 *  Used in notifications like PicselPointer_down().
 *  Values in the range 0 to PicselPointerButton_CustomBase are reserved.
 */
typedef enum PicselPointerButton
{
    /** The normal mouse button, typically representing selection of the
     *  item under the pointer.
     */
    PicselPointerButton_Left   = 1,
    PicselPointerButton_Middle = 2,
    PicselPointerButton_Right  = 3,

    /** A base value for manufacturer-specified button values.
     * A manufacturer may extend the range of available button values by
     * defining PointerId values >= 65536
     * Manufacturer button Id values must be provided to application developers
     * as an addition to any Picsel SDK.
     */
    PicselPointerButton_CustomBase = 65536
}
PicselPointerButton;



/**
 * Notification that the device pointer's button has been pressed by the user.
 *
 * The coordinates are based on 0, 0 being the top left corner of the screen,
 * with positive coordinates going right and down. Note that these
 * coordinates are not affected by the orientation of the Alien screen.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 * @param timestamp     Timestamp for the event, in milliseconds
 * @param button        Which button has been pressed.
 *                      See @ref PicselPointerButton.
 * @param x             The x location of the pointer when the button
 *                      was pressed.
 * @param y             The y location of the pointer
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @see                 PicselPointer_up(), PicselPointer_move()
 */
int PicselPointer_down(Picsel_Context *picselContext,
                       unsigned int    timestamp,
                       int             button,
                       int             x,
                       int             y);

/**
 * Notification that the device's pointer has been moved by the user.
 *
 * The coordinates are based on 0, 0 being the top left corner of the screen,
 * with positive coordinates going right and down. Note that these
 * coordinates are not affected by the orientation of the Alien screen
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 * @param timestamp     Timestamp for the event, in milliseconds
 * @param x             The x location of the pointer
 * @param y             The y location of the pointer
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselPointer_move(Picsel_Context *picselContext,
                       unsigned int    timestamp,
                       int             x,
                       int             y);

/**
 * Informs the Picsel engine that the mouse pointer has been released.
 *
 * The coordinates are based on 0, 0 being the top left corner of the screen,
 * with positive coordinates going right and down. Note that these
 * coordinates are not affected by the orientation of the Alien screen
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 * @param timestamp     Timestamp for the event, in milliseconds
 * @param button        The id of the pointer button generating the event.
 *                      This will be a @ref PicselPointerButton value or a
 *                      manufacturer-specific value in the custom id range
 * @param x             The x location of the pointer
 * @param y             The y location of the pointer
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @see                 PicselPointer_down(),
 */
int PicselPointer_up(Picsel_Context *picselContext,
                     unsigned int    timestamp,
                     int             button,
                     int             x,
                     int             y);

/** @} */ /* End of Doxygen group */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_POINTER_H */
