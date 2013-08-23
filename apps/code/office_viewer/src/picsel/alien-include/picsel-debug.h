/**
 * Debug interface between Picsel and Alien.
 *
 * This file contains some handy functions for debug purposes.
 *
 * The implementations of functions in this file are not optimal and they
 * should not be used for any purpose other than debugging.
 *
 * Where the functions return 'const char *' strings, these are static strings
 * stored in the library and will remain valid as long as the library
 * is in memory. There is no need to call free() on them.
 *
 * All functions supplied by Picsel are prefixed by 'Picsel'. The integrator
 * will need to make calls to these functions as described in the Picsel
 * supplied documentation, but should not attempt to provide implementations
 * of them.
 *
 * Copyright (C) Picsel, 2007. All Rights Reserved.
 *
 * $Id: picsel-debug.h,v 1.13 2008/12/11 16:34:21 roger Exp $
 *
 * @file
 * @author Picsel Technologies Ltd
 *
 */

/**
 * @defgroup TgvPicselStatusDebug Picsel Status Information
 * @ingroup TgvTest
 *
 * Information about the status of the Picsel TGV library.
 *
 * @{
 */
#ifndef PICSEL_DEBUG_H
#define PICSEL_DEBUG_H

#include "alien-notify.h"
#include "alien-types.h"
#include "alien-request.h"

#include "picsel-control.h"
#include "picsel-flowmode.h"
#include "picsel-focus.h"
#include "picsel-screen.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Get the textual representation of a user request type.
 *
 * Note: This function is for debug use only.
 *
 * @param type User request type to be converted
 * @return pointer to string representation of user request type in English
 */
const char *PicselDebug_getUserRequestType(PicselUserRequest_Type type);


/**
 * Get the textual representation of a user request result.
 *
 * Note: This function is for debug use only.
 *
 * @param result User request result to be converted
 * @return pointer to string representation of user request result in English
 */
const char *PicselDebug_getUserRequestResult(PicselUserRequest_Result result);


/**
 * Get the textual representation of an alien information event type.
 *
 * Note: This function is for debug use only.
 *
 * @param event     Alien information event type to be converted
 * @param eventData Alien information event data
 * @return pointer to string representation in English
 */
const char *PicselDebug_getInformationEvent(AlienInformation_Event event,
                                            void                  *eventData);


/**
 * Get the textual representation of an picsel document error.
 *
 * Note: This function is for debug use only.
 *
 * @param error Picsel document error to be converted
 * @return pointer to string representation in English
 */
const char *PicselDebug_getDocumentError(PicselError error);

/**
 * Get the textual reprentation of a PicselFocus_Navigation
 *
 * Note: This function is for debug use only.
 *
 * @param navigation Picsel focus navigation to be converted
 * @return pointer to string representation in English
 */
const char *PicselDebug_getNavigation(PicselFocus_Navigation navigation);

/**
 * Get the textual reprentation of a PicselControl_State
 *
 * Note: This function is for debug use only.
 *
 * @param state Picsel control state to be converted
 * @return pointer to string representation in English
 */
const char *PicselDebug_getControlState(PicselControl_State state);

/**
 * Get the textual reprentation of a PicselControl_Position
 *
 * Note: This function is for debug use only.
 *
 * @param position Picsel control position to be converted
 * @return pointer to string representation in English
 */
const char *PicselDebug_getControlPosition(PicselControl_Position position);

/**
 * Get the textual reprentation of a PicselRotation
 *
 * Note: This function is for debug use only.
 *
 * @param rotation Picsel rotation to be converted
 * @return pointer to string representation in English
 */
const char *PicselDebug_getRotation(PicselRotation rotation);

/**
 * Get the textual representation of an picsel flowmode type
 *
 * Note: This function is for debug use only.
 *
 * @param flowMode Picsel FlowMode to be converted
 * @return pointer to string representation in English
 */
const char *PicselDebug_getFlowMode(PicselFlowMode flowMode);

/**
 * Display the internal state of the progress bar
 *
 * The data is printed via AlienDebug_output
 * NB: This function is for debug use only.
 *
 * @param picselContext  the picsel context
 *
 * @return 1 if the event has been accepted; 0 if the event was rejected.
 */
int PicselDebug_dumpProgress(Picsel_Context *picselContext);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_DEBUG_H */
