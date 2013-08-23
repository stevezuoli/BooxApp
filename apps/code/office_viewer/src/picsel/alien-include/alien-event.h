/**
 * Miscellaneous important functions
 *
 * @see @ref picsel-app.h
 *
 * The functions in this file must be implemented by the Alien application
 * before linking with the TGV library.
 *
 * @file
 * $Id: alien-event.h,v 1.72 2008/12/11 16:34:19 roger Exp $
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */

#ifndef ALIEN_EVENT_H
#define ALIEN_EVENT_H

#include "alien-types.h"
#include "alien-information.h"

/**
 * Some definitions were moved from alien-event.h to picsel-app.h.
 * Include PicselApp functions that use to be found in alien-event.h, for
 * backwards compatibility
 */
#include "picsel-app.h"

/**
 * Include old interface definitions, for backwards compatibility.
 */
#include "alien-legacy.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Informs the Alien application of the Picsel_Context pointer which this
 * instance of the Picsel library will use. The Picsel_Context contains
 * data which is effectively global, but is only accessed through this
 * pointer, to avoid using actual global variables, which are not allowed
 * on some device operating systems. The @ref Picsel_Context pointer will
 * be needed as a parameter to most Picsel* functions. The Alien application
 * should store the pointer, but never read or write the data it points to.
 *
 * This function will be called by Picsel during handling of
 * PicselApp_start().
 *
 * @param[in] alienContext  The Alien_Context which was provided by the Alien
 *            application when it called PicselApp_start(). The Alien
 *            application may optionally use this to store information about
 *            its own state. The Picsel library will not read or write it.
 * @param[in] picselContext The pointer which the Picsel library uses
 *            internally to store its state. The Alien application should
 *            record this, for use when it calls any Picsel* function, but
 *            should never read or write the data.
 *
 * @ingroup TgvInitialisation
 */
void AlienEvent_setPicselContext(Alien_Context  *alienContext,
                                 Picsel_Context *picselContext);

/**
 * Notifies the Alien application of information about the state of the
 * Picsel library, for example completion of loading a document, or
 * provides information such as its title, when requested. These events
 * can usually be ignored.
 *
 * @param alienContext  See PicselApp_start()
 * @param eventType     Which type of information is being notified
 * @param eventData     A pointer to an AlienInformation_* structure
 *                      appropriate to this notification, for example
 *                      @ref AlienInformation_TitleInfo
 *
 * @ingroup TgvContentInformation
 */
void AlienEvent_information(Alien_Context          *alienContext,
                            AlienInformation_Event  eventType,
                            void                   *eventData);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_EVENT_H */
