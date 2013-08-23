/**
 *
 * Touchscreen gesture handler
 *
 * Copyright (C) Picsel, 2007. All Rights Reserved.
 *
 * $Id: touchscreen.c,v 1.5.2.2 2010/07/28 09:53:18 alans Exp $
 *
 * @author Picsel Technologies Ltd
 * @file
 *
 */


#include "touchscreen.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/* Picsel alien interface include files */
#include "alien-context.h"
#include "alien-screen.h"
#include "picsel-control.h"
#include "picsel-screen.h"
#include "picsel-fileviewer.h"

#define COUNTDOWN_CLICKS   (10)
#define COUNTDOWN_TIMESTEP (100)

#define DOUBLECLICK_TIME   (300)
#define JITTERCLICK_TIME   (20)

/* Magnification factor for zooming in */
#define MAG_SCALE          (12)


/* Amount of allowed movement between taps of a double-tap */
#define DOUBLETAP_DISTANCE (32)


/*Main data structure for pointer event handler*/
struct TouchscreenHandler
{
    Alien_Context            *alienContext;

    TouchscreenTimerCallback *timerCb;
    TouchscreenModeCallback  *modeCb;
    void                     *userContext;
    int                       timerCount;

    TouchscreenMode           mode;

    int                       gesturesEnabled;

    unsigned long             zoomInfo;
    unsigned long             zoom;

    int                       firstX;
    int                       firstY;
    unsigned int              firstTime;
    unsigned int              secondTime;

    int                       prevX;
    int                       prevY;

    int                       pageZone;

    bool                      panTimer;
    int                       panX;
    int                       panY;

    int                       gutterLeft;
    int                       gutterTop;
    int                       gutterRight;
    int                       gutterBottom;
};

TouchscreenHandler*  Touchscreen_initialise(Alien_Context* ac)
{

    TouchscreenHandler *handler = malloc(sizeof(*handler));

    assert(ac != NULL);

    handler->alienContext = ac;

    handler->timerCb = NULL;
    handler->timerCount = 0;
    handler->zoomInfo = 65536;
    handler->pageZone = 1<<30;
    handler->mode = TouchscreenMode_Idle;
    handler->firstTime = 0;
    handler->panTimer = false;
    handler->gutterLeft = -1<<30;
    handler->gutterRight = 1<<30;
    handler->gutterTop = -1<<30;
    handler->gutterRight = 1<<30;
    handler->gesturesEnabled = 0;

    return handler;
}


void Touchscreen_pageTurnZone(TouchscreenHandler *handler, int height)
{
    assert (handler != NULL);

    handler->pageZone = height;
}


void Touchscreen_finalise(TouchscreenHandler **handler)
{
    assert (*handler != NULL);

    free(*handler);
    *handler = NULL;

}

void Touchscreen_setCallbacks(TouchscreenHandler        *handler,
                              TouchscreenTimerCallback  *timerCb,
                              TouchscreenModeCallback   *modeCb,
                              void                      *context)
{
    assert(handler != NULL);

    handler->timerCb = timerCb;
    handler->modeCb = modeCb;
    handler->userContext = context;
}

void Touchscreen_setAutopanGutter(TouchscreenHandler *handler,
                                  int                 left,
                                  int                 top,
                                  int                 right,
                                  int                 bottom )
{
    assert(handler != NULL);

    handler->gutterLeft = left;
    handler->gutterTop = top;
    handler->gutterRight = right;
    handler->gutterBottom = bottom;
}

void Touchscreen_enableGestures(TouchscreenHandler *handler, int gestures)
{
    assert(handler != NULL);

    handler->gesturesEnabled = gestures;
}

void Touchscreen_timerCb(TouchscreenHandler  *handler,
                         struct timeval       timestamp)
{
    timestamp = timestamp; /* Unused, shush compiler */

    assert(handler != NULL);

    if ( handler->mode == TouchscreenMode_Selection )
    {
        if ( handler->panTimer )
        {
            struct timeval noTime;
            noTime.tv_sec = 0;
            noTime.tv_usec = 0;

            handler->panTimer = false;

            Touchscreen_handlePointerMove( handler,
                                           handler->panX,
                                           handler->panY,
                                           noTime );
        }
        return;
    }

    handler->timerCount--;

    if ( handler->timerCount>0 )
    {
        if ( (0!=(handler->gesturesEnabled & TOUCHSCREEN_GESTURE_HOLD)) &&
            (handler->mode == TouchscreenMode_Hold) )
        {
            if ( handler->timerCount < COUNTDOWN_CLICKS-2 )
            {
                if ( handler->modeCb )
                    handler->modeCb( handler->userContext, TouchscreenMode_Hold );

            }

            handler->timerCb( handler->userContext, 100 );
        }
    }
    else
    {
        handler->mode = TouchscreenMode_Idle;

        if ( handler->modeCb )
            handler->modeCb( handler->userContext, handler->mode );

    }
}


void Touchscreen_updateZoomValue( TouchscreenHandler *handler,
                                  unsigned long       zoom)
{
    assert(handler != NULL);

    handler->zoomInfo = zoom;
}


void Touchscreen_handlePointerDown(TouchscreenHandler *handler,
                                   int x,
                                   int y,
                                   struct timeval timestamp)
{
    unsigned long time;

    assert (handler != NULL);

    if ( handler->timerCb )
    {
        if ( 0!=(handler->gesturesEnabled &
                  (TOUCHSCREEN_GESTURE_SELECTION|TOUCHSCREEN_GESTURE_HOLD)) )
        {
            handler->timerCount = COUNTDOWN_CLICKS;
            handler->timerCb( handler->userContext, COUNTDOWN_TIMESTEP );
        }
    }

    time = timestamp.tv_usec/1000;
    time += timestamp.tv_sec*1000;

    /* Hack to try and see if we're really getting jittery clicks */
    if ( time-handler->firstTime < JITTERCLICK_TIME )
        return;

    handler->firstX = x;
    handler->firstY = y;
    handler->secondTime = 0;

    if ( (time-handler->firstTime < DOUBLECLICK_TIME) &&
         (handler->mode == TouchscreenMode_Idle) )
    {
        int dx = handler->firstX - x;
        int dy = handler->firstY - y;

        int dist = (dx*dx)+(dy*dy);

        if ( dist<DOUBLETAP_DISTANCE * DOUBLETAP_DISTANCE )
        {
            handler->secondTime = handler->firstTime;
            time = 0;
        }
    }

    handler->firstTime = time;

    handler->mode = TouchscreenMode_Hold;

    /*request the current zoom value.  Wish this was synchronous!*/
    (void)PicselControl_getZoom(handler->alienContext->picselContext);
}

void Touchscreen_handlePointerDblClk(TouchscreenHandler *handler,
                                     int x,
                                     int y,
                                     struct timeval timestamp)
{
    x=x; /* Unused, shush compiler */
    y=y; /* Unused, shush compiler */

    assert (handler != NULL);

    if ( handler->timerCb )
    {
        handler->timerCount = COUNTDOWN_CLICKS;
        handler->timerCb( handler->userContext, COUNTDOWN_TIMESTEP );
    }

    /* Barmyness happened.  Hello Moto! */
    if ( handler->secondTime )
        return;

    handler->secondTime = timestamp.tv_usec/1000;
    handler->secondTime += timestamp.tv_sec*1000;
    handler->mode = TouchscreenMode_Hold;
}

void Touchscreen_handlePointerUp(TouchscreenHandler *handler,
                                 int x,
                                 int y,
                                 struct timeval timestamp)
{
    x=x; /* Unused, shush compiler */
    y=y; /* Unused, shush compiler */
    timestamp=timestamp; /* Unused, shush compiler */

    Picsel_Context *picsel;

    assert (handler != NULL);

    picsel = handler->alienContext->picselContext;

    if ( handler->timerCb )
        handler->timerCb( handler->userContext, 0 );

    switch ( handler->mode )
    {
        case TouchscreenMode_Zoom:
            PicselControl_zoom( picsel, 0, 0, PicselControl_End, 0);
            break;

        case TouchscreenMode_Pan:
            PicselControl_pan( picsel, 0, 0, PicselControl_Release);
            break;

        case TouchscreenMode_Selection:
            if ( 0==(handler->gesturesEnabled & TOUCHSCREEN_GESTURE_SELECTION) )
                break;
            break;

        default:
            break;
    }

    handler->mode = TouchscreenMode_Idle;
    if ( handler->modeCb )
        handler->modeCb( handler->userContext, handler->mode );
}

void Touchscreen_handlePointerMove(TouchscreenHandler *handler,
                                   int x,
                                   int y,
                                   struct timeval timestamp)
{
    Picsel_Context *picsel;

    timestamp = timestamp; /* Unused, shush compiler */

    assert (handler != NULL);

    picsel = handler->alienContext->picselContext;

    if ( handler->mode == TouchscreenMode_Idle )
        return;

    if ( handler->mode == TouchscreenMode_Hold )
    {
        if ( handler->secondTime )
        {
            if ( 0!=(handler->gesturesEnabled & TOUCHSCREEN_GESTURE_ZOOM ) )
            {
                handler->mode = TouchscreenMode_Zoom;
            }
        }

        if ( handler->timerCb )
            handler->timerCb( handler->userContext, 0 );

        if ( 0!=(handler->gesturesEnabled & TOUCHSCREEN_GESTURE_PAGE ) )
        {
            /* Horizontal motion in the bottom part of the screen = page turns */
            if ( y>handler->pageZone && handler->firstY>handler->pageZone )
                if ( abs(x-handler->firstX) > abs(y-handler->firstY) )
                    handler->mode = TouchscreenMode_Page;

        }

        if ( 0!=(handler->gesturesEnabled & TOUCHSCREEN_GESTURE_PAN ) )
        {
            /* Still not decided?  Then it's a pan */
            if ( handler->mode == TouchscreenMode_Hold )
                handler->mode = TouchscreenMode_Pan;
        }

        /* STILL not decided?  Then give up */
        if ( handler->mode == TouchscreenMode_Hold )
            handler->mode = TouchscreenMode_Idle;

        if ( handler->modeCb )
            handler->modeCb( handler->userContext, handler->mode );

        handler->prevX = handler->firstX;
        handler->prevY = handler->firstY;

        handler->zoom = handler->zoomInfo;
    }

    handler->firstTime = 0;

    switch ( handler->mode )
    {

        case TouchscreenMode_Page:
        {
            int dx = handler->prevX - x;

            /* A single flick turn has to be a large gesture */
            if ( !handler->secondTime )
                if ( abs(dx)<64 )
                    return;

            handler->prevX = x;

            PicselFileviewer_turnPage( picsel,
                       dx < 0 ? PicselPageTurn_Previous : PicselPageTurn_Next );

            /* A single flick page turn has been done, ignore further moves */
            if ( !handler->secondTime )
            {
                handler->mode = TouchscreenMode_Idle;
                if ( handler->modeCb )
                    handler->modeCb( handler->userContext, handler->mode );
            }
        }
        break;

        case TouchscreenMode_Zoom:
        {
            /* We calculate zoom as a direct fraction (256ths) of the amount
               of zoom we had when we first started this zoom op */

            int dy = handler->firstY - y;
            if ( dy>0 )
                dy=(dy*dy)/MAG_SCALE;

            if ( dy<-(1<<8) )
                dy=-(1<<8);

            PicselControl_zoom( picsel,
                                handler->firstX,
                                handler->firstY,
                                PicselControl_Continue,
                                (handler->zoom*((1<<8) + dy))>>8 );

        }
        break;

        case TouchscreenMode_Pan:
        {
            /* We pan using deltas for each motion seen */

            int dx = x - handler->prevX;
            int dy = y - handler->prevY;

            handler->prevX = x;
            handler->prevY = y;

            PicselControl_pan( picsel,
                               dx,
                               dy,
                               PicselControl_Continue );

        }
        break;

        default:
        /* Nothing to do */
        break;
    }
}

void Touchscreen_updateRotation( /*@unused@*/TouchscreenHandler *handler,
                                 /*@unused@*/PicselRotation      rotation)
{
    /* FIXME */
    handler = handler;      /* unused */
    rotation = rotation;    /* unused */
}
