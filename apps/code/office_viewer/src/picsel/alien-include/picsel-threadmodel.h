/**
 * Threading models used in the Picsel library for concurrent processing.
 *
 * @file
 * $Id: picsel-threadmodel.h,v 1.8 2008/12/11 16:34:23 roger Exp $
 */
/* Copyright (C) Picsel, 2006-2008. All Rights Reserved. */
/**
 * @addtogroup TgvThreading
 *
 * @{
 */

#ifndef PICSEL_THREADMODEL_H
#define PICSEL_THREADMODEL_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Prototype function that will initialise the threading
 * model when passed to PicselApp_start().
 *
 * There are only two specific instances of this type for
 * the Picsel's own and Device platform's native threading models:
 *  - Picsel_ThreadModel_softThreads()
 *  - Picsel_ThreadModel_alienThreads()
 *
 * @param picselContext See AlienEvent_setPicselContext()
 */
typedef void (*Picsel_ThreadModelFn)(Picsel_Context *picselContext);


/**
 * Enable Picsel's soft-threading. This is the recommended model.
 * It may be passed as a function-pointer parameter to
 * PicselApp_start(). The Alien application does not need to
 * make any other preparations for threading; it will be handled
 * inside the Picsel library and using PicselApp_timerExpiry().
 *
 * @note The Alien application must not call this function directly, but
 * must pass them as function pointer parameters to PicselApp_start().
 *
 * @see @ref AlienPal_stackDirection, @ref AlienPal_stackIndex,
 *      @ref AlienPal_discardedStack.
 */
Picsel_ThreadModelFn Picsel_ThreadModel_softThreads(void);

/**
 * Enable native (alien) threading using the Device platform OS.
 * It may be passed as a function-pointer parameter to PicselApp_start(),
 * in which case the functions in @ref alien-thread.h must be implemented.
 *
 * @note The Alien application must not call this function directly, but
 * must pass them as function pointer parameters to PicselApp_start().
 *
 */
Picsel_ThreadModelFn Picsel_ThreadModel_alienThreads(void);

/** @} */ /* End of Doxygen group */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_THREADMODEL_H */
