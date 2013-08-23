/**
 * Information and event reporting from the Picsel Core library to the Alien
 * application.
 *
 * This file contains definitions and declarations needed for the calls
 * from the Picsel library to inform the Alien application of events inside the
 * Picsel library that may require a response or an action from the Alien application.
 *
 * The alien application is free to ignore these events.
 *
 * @file
 * $Id: alien-information.h,v 1.2 2008/12/11 16:34:19 roger Exp $
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
/**
 * @addtogroup TgvContentInformation
 *
 * @{
 */

#ifndef ALIEN_INFORMATION_H
#define ALIEN_INFORMATION_H

#include "picsel-entrypoint.h"

/**
 * Asynchronous notifications from the Picsel library to the Alien
 * application. These are sent to AlienEvent_information() and usually
 * include a structure giving more details; the type of that structure
 * depends on the event notified.
 *
 * Picsel uses information events extensively to respond to requests
 * made by the Alien application earlier, but for which the status
 * could not be confirmed immediately. Some responses can be made
 * within a few milliseconds, but others make take many seconds.
 * Some information events are provided unsolicited, regarding the
 * state of another part of the system, such as the Internet connection
 * or behaviour within the content being viewed.
 *
 * There are additional values that may be notified, which are not listed
 * here. These are specific to optional features, and are defined in
 * API header files specific to those features; for example
 * @ref AlienInformation_AnnotationResult.
 *
 * See @ref TgvAsync_Queue for further explanation.
 */
typedef enum AlienInformation_Event
{
    /** Initialisation complete,
        @c eventData is @ref AlienInformation_AppHandle.    */
    AlienInformation_InitComplete = (1<<16),

    /** Initialisation failed; @c eventData is
        @ref AlienInformation_InitFailedInfo. */
    AlienInformation_InitFailed,

    AlienInformation_CoreLast

}
AlienInformation_Event;


/**
 * Gives a handle for the running Picsel product, sent with
 * an @ref AlienInformation_InitComplete event.
 *
 * Passed as @c eventData for an @ref AlienInformation_InitComplete event,
 * which is sent after the Picsel library is initialised using PicselApp_start()
 * or PicselApp_startEmebedded().
 * This handle may be required for a later call to PicselApp_removeEmbedded().
 */
typedef struct AlienInformation_AppHandle
{
    Picsel_AppHandle *appHandle;
    Picsel_initialiseFn initFn; /**< Tells the alien application which TGV
                                     product has been initialised. */
}
AlienInformation_AppHandle;

/**
 * Indicates why the Picsel library failed to initialise.
 * You should find guidance below on how to resolve any errors in the alien
 * implementation.
 *
 * If you continue to encounter issues initialising the Picsel library,
 * contact your Picsel representative quoting the failure code returned.
 */
typedef enum AlienInformation_InitFailedCode
{
    /**
     * Unspecified error initialising the library.
     */
    AlienInformation_InitFailedCode_Unknown = 0x10000,

    /**
     * An error was encountered while trying to allocate the Picsel context.
     * Please ensure you have correctly implemented the AlienMemory
     * functions, especially AlienMemory_malloc() and
     * AlienMemory_getPicselContext(). See @ref alien-memory.h for more
     * information.
     */
    AlienInformation_InitFailedCode_PicselContext,

    /**
     * The Picsel library is restricted to run within a licensed time-frame.
     * Ensure you have implemented the AlienTimer functions (see
     * @ref alien-timer.h) and that your system clock is set correctly.
     */
    AlienInformation_InitFailedCode_License,

    /**
     * An error was encountered while trying to initialise the splash screen.
     */
    AlienInformation_InitFailedCode_Splashscreen,

    /**
     * An error was encountered while trying to initialise the main ePage
     * context.
     */
    AlienInformation_InitFailedCode_Epage,

    /**
     * Unable to initialise the threading model.
     */
    AlienInformation_InitFailedCode_ThreadModel,

    /**
     * Unable to initialise the TGV core.
     */
    AlienInformation_InitFailedCode_TgvCore,

    /**
     * Unable to initialise the user request module.
     */
    AlienInformation_InitFailedCode_UserRequests,

    /**
     * An error was encountered in the product entrypoint function.
     * Please ensure you have specified the correct initialise
     * function for your Picsel library when calling PicselApp_start().
     * See @ref picsel-entrypoint.h for more information.
     */
    AlienInformation_InitFailedCode_EntryPoint,

    /**
     * An error was encountered while trying to initialise the product
     * interface.
     */
    AlienInformation_InitFailedCode_App
}
AlienInformation_InitFailedCode;

/**
 * Gives the reason the Picsel library failed to initialise,
 * sent with an an @ref AlienInformation_InitFailed event.
 *
 * Passed as @c eventData for an @ref AlienInformation_InitFailed event.
 *
 */
typedef struct AlienInformation_InitFailedInfo
{
    AlienInformation_InitFailedCode failureCode;
}
AlienInformation_InitFailedInfo;

/**
 * @}
 */

#endif /* !ALIEN_INFORMATION_H */
