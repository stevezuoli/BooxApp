/**
 *
 * Alien Context declaration
 *
 * Private data used by Alien
 *
 *
 * Copyright (C) Picsel, 2005. All Rights Reserved.
 *
 * $Id: alien-context.h,v 1.19 2009/12/16 16:38:05 andrewc Exp $
 *
 * @file
 * @author Picsel Technologies Ltd
 *
 */
#ifndef ALIEN_CONTEXT_H
#define ALIEN_CONTEXT_H
#include <stdio.h>
#include "alien-types.h"
#include "alien-event.h"
#include "alien-thread.h"
#include "picsel-entrypoint.h"
#include "picsel-flowmode.h"
#include "picsel-screen.h"
#include "preferences.h"

#include "picsel-search.h"

#include "touchscreen.h"

#include "picsel-views.h"

#if defined(__cplusplus)
extern "C" {
#endif /* (__cplusplus) */

typedef struct AlienLinuxMain  AlienLinuxMain;
typedef struct AlienTcpContext AlienTcpContext;
typedef struct LinuxQueue_Context LinuxQueue_Context;
typedef struct AlienPrintContext AlienPrintContext;
typedef struct AlienLauncherData AlienLauncherData;
typedef struct AlienFileSysData AlienFileSysData;

typedef enum
{
    PendingAction_None,
    PendingAction_Shutdown,
    PendingAction_Restart,
    PendingAction_Load
} PendingAction_Type;


struct Alien_Context
{
    Picsel_Context     *picselContext;   /**< PicselContext that we must pass
                                          *   to all Picsel calls           */
    Picsel_initialiseFn initFn;          /**< Initialisation function for
                                          *   currently running application */
    AlienLinuxMain     *alienLinuxMain;  /**< Information for linux-main.c  */
    AlienTcpContext    *alienTcpContext; /**< Information for alien-tcp.c   */
    LinuxQueue_Context *queueContext;    /**< Information for linux-queue.c */

    int                 exitApp;         /**< Set to 'true' to initiate
                                          *   clean shutdown                */
    int                 initFailed;      /**< TGV core failed to initialise */
    int                 initOk;          /**< TGV core initialised ok */
    int                 useExpandingHeap;/**< Ask Epage to use an expanding
                                              heap */

    TouchscreenHandler *tsHandler;       /**< Touchscreen handle information
                                          *  event                          */

    char              *currentFile;

    int                currentPage;
    int                numPages;
    AlienPrintContext *alienPrint;
    PicselInputType    inputType;
    AlienThread       *mainThread;
    AlienThread       *currentThread;
    AlienThread       *threadList;

    PicselFlowMode     flowMode;         /**< The current flowmode */
    int                changingFlowMode; /**< 1 if flow mode is changing */
    int                picselInitComplete;
    AlienLauncherData  *alienLauncherData;
    AlienFileSysData   *alienFileSysData;

    /* Rotation structure only used for direct and direct from GTK drawing */
    PicselRotation      rotation;  /**< Used for rotating screen in Alien */

    int                 pointerSizeThreshold; /* drop moves less than this */

    /** Pointer dummy database used in alien-db-messages.c*/
    struct db_handle *messagingHandle;

    int                    foundSomething;
    PicselSearch_Direction searchDirection;
    Preferences        *preferences;          /* Preferences context. */
    int                 status;

    PrefError           prefInitStatus;       /* Records the status   *
                                               * of preference        *
                                               * initialisation.      */

    PrefFontError       prefFontInitStatus;   /* Records the status   *
                                               * of preference font   *
                                               * initialisation.      */

    unsigned long       overrideHeapSize;     /* If non 0 use         *
                                               * this heap size       *
                                               * instead of the       *
                                               * default.             *
                                               * Note fortify         *
                                               * builds use 4x as     *
                                               * much heap.           */

    int                 fvGestures;           /* Gestures to support  *
                                               * in fileviewer apps   */

    PendingAction_Type  pendingAction;        /* Action to process    *
                                               * once the library is  *
                                               * ready                */

    int                 lastPointerX;         /* X position of the    *
                                               * last pointer down or *
                                               * move event           */

    int                 lastPointerY;         /* X position of the    *
                                               * last pointer down or *
                                               * move event           */

    int                 touchscreenMode;      /* The current          *
                                               * touchscreen mode     */

    unsigned int        overrideScreenWidth;  /* If non 0 use this    *
                                               * value instead of the *
                                               * platform default     *
                                               * screen width.        */

    unsigned int        overrideScreenHeight; /* If non 0 use this    *
                                               * value instead of the *
                                               * platform default     *
                                               * screen height.       */

};

extern Alien_Context *globalAlienContext;

/**
 * Clean up the alien messaging database
 *
 * @param[in]alienContext          alien context
 *
 */
extern void AlienDb_Messages_clearUp(Alien_Context        *alienContext);

#if defined(__cplusplus)
}
#endif /* (__cplusplus) */

#endif /* !ALIEN_CONTEXT_H */
