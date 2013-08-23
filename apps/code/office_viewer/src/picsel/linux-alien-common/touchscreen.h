/**
 *
 * Touchscreen gesture handler
 *
 * Copyright (C) Picsel, 2007. All Rights Reserved.
 *
 * $Id: touchscreen.h,v 1.1 2009/01/16 14:42:48 maxim Exp $
 *
 * @author Picsel Technologies Ltd
 * @file
 *
 */

#ifndef LINUX_TOUCHSCREEN_H
#define LINUX_TOUCHSCREEN_H

#include <sys/time.h>

#include "picsel-screen.h"

/**
* Opaque type for touchscreen handler structure
*/
typedef struct TouchscreenHandler TouchscreenHandler;

/**
 * Touchscreen modes
 */
typedef enum TouchscreenMode
{
    TouchscreenMode_Idle,
    TouchscreenMode_Pan,
    TouchscreenMode_Zoom,
    TouchscreenMode_Page,
    TouchscreenMode_Selection,
    TouchscreenMode_Hold
}
TouchscreenMode;

/**
 * Gestures allowed - a bitfield
 */
#define TOUCHSCREEN_GESTURE_ZOOM        (1<<0)
#define TOUCHSCREEN_GESTURE_PAN         (1<<1)
#define TOUCHSCREEN_GESTURE_PAGE        (1<<2)
#define TOUCHSCREEN_GESTURE_HOLD        (1<<3)
#define TOUCHSCREEN_GESTURE_SELECTION   (1<<4)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Timer Callback
 *
 * @param void*     Callback context
 * @param int       MSec to call back after
 */
typedef void (TouchscreenTimerCallback)(void *, int);

/**
 * Mode Callback
 *
 * @param void*     Callback context
 * @param TouchscreenMode mode
 */
typedef void (TouchscreenModeCallback)(void *, TouchscreenMode);

/**
  * Initialise the pointer handler
  *
  * @param context   The context
  * @return          Pointer to touchscreen handler
  */
TouchscreenHandler*  Touchscreen_initialise(Alien_Context* context);

 /**
  * Finalise the pointer-handler
  *
  * @param handler   The handler
  */
void Touchscreen_finalise(TouchscreenHandler **handler);


/**
 * Give touchscreen a callback to ask for a timer and mode
 *
 * @param handler      The handler
 * @param timberCb     Timer callback pointer
 * @param modeCb       Mode callback pointer
 * @param context      User context for callback
 */
void Touchscreen_setCallbacks(TouchscreenHandler        *handler,
                              TouchscreenTimerCallback  *timerCb,
                              TouchscreenModeCallback   *modeCb,
                              void                      *context);

/**
 * Timer callback
 *
 * @param handler      The handler
 * @param timestamp    time of event
 */
void Touchscreen_timerCb(TouchscreenHandler  *handler,
                         struct timeval       timestamp);

/**
 * Define gutters for auto panning during selection
 *
 * @param handler      The handler
 * @param left         Left side
 * @param top          Top side
 * @param right        Right side
 * @param bottom       Bottom side
 */
void Touchscreen_setAutopanGutter(TouchscreenHandler *handler,
                                  int                 left,
                                  int                 top,
                                  int                 right,
                                  int                 bottom );

/**
 * Define zone for recognising page turn gesture
 *
 * @param handler      The handler
 * @param height       Height for zone
 */
void Touchscreen_pageTurnZone(TouchscreenHandler *handler, int height);

/**
 * Define gestures to recognise
 *
 * @param handler      The handler
 * @param gestures     Bitfield of allowed gestures
 */
void Touchscreen_enableGestures(TouchscreenHandler *handler, int gestures);


 /**
  * Handle pen down event
  *
  * @param handler      The handler
  * @param x            x-coord
  * @param y            y-coord
  * @param timestamp    time of event
  */
void Touchscreen_handlePointerDown(TouchscreenHandler  *handler,
                                   int                  x,
                                   int                  y,
                                   struct timeval       timestamp);
 /**
  * Handle double-click  event(pen-up, pen-down, pen-up)
  *
  * @param handler      The handler
  * @param x            x-coord
  * @param y            y-coord
  * @param timestamp    time of event
  */
void Touchscreen_handlePointerDblClk(TouchscreenHandler *handler,
                                     int                 x,
                                     int                 y,
                                     struct timeval      timestamp);
 /**
  * Handle pen move event
  *
  * @param handler      The handler
  * @param x            x-coord
  * @param y            y-coord
  * @param timestamp    time of event
  */
void Touchscreen_handlePointerMove(TouchscreenHandler  *handler,
                                   int                  x,
                                   int                  y,
                                   struct timeval       timestamp);
 /**
  * Handle pen up event
  *
  * @param handler      The handler
  * @param x            x-coord
  * @param y            y-coord
  * @param timestamp    time of event
  */
void Touchscreen_handlePointerUp(TouchscreenHandler *handler,
                                 int                 x,
                                 int                 y,
                                 struct timeval      timestamp);
 /**
  * Update zoom value from value received from information event
  *
  * @param handler   The handler
  * @param zoom      The zoom value
  */
void Touchscreen_updateZoomValue( TouchscreenHandler *handler,
                                  unsigned long       zoom);

 /**
  * Update rotation angle
  *
  * @param handler   The handler
  * @param rotation  The rotation value
  */
void Touchscreen_updateRotation( TouchscreenHandler *handler,
                                 PicselRotation      rotation);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LINUX_TOUCHSCREEN_H */
