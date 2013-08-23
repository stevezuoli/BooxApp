/**
 * Retrieve Picsel Library version information.
 *
 * $Id: picsel-version.h,v 1.6 2009/03/05 11:42:37 dominic Exp $
 * @file
 */
/* Copyright (C) Picsel, 2007-2008. All Rights Reserved. */
/**
 * @addtogroup TgvPicselStatusDebug
 * @{
 */

#ifndef PICSEL_VERSION_H
#define PICSEL_VERSION_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Gets a version string for the Picsel Library. This would typically be used
 * in "Help... About..." scenarios.
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext().
 * @param[in,out] buffer        A buffer allocated by the Alien Application to
 *                              receive the version string.
 * @param[in,out] bufferSize    Size of the buffer - if buffer is NULL then the
 *                              required buffer size is returned here.
 *
 * @return 1 for success, 0 otherwise.
 */
int PicselVersion_getVersion(Picsel_Context *picselContext,
                             unsigned char  *buffer,
                             unsigned int   *bufferSize);

/**
 * Gets a string containing the name of the customer. This would typically be
 * used in "Help... About..." scenarios.
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext().
 * @param[in,out] buffer        A buffer allocated by the Alien Application to
 *                              receive the string.
 * @param[in,out] bufferSize    Size of the buffer - if buffer is NULL then the
 *                              required buffer size is returned here.
 *
 * @return 1 for success, 0 otherwise.
 */
int PicselVersion_getCustomer(Picsel_Context *picselContext,
                              unsigned char  *buffer,
                              unsigned int   *bufferSize);

/**
 * Gets a string containing the issue number for this version of the Picsel
 * Library and identifies the release from Picsel and is unique to each
 * customer and release version. This would typically be used in "Help...
 * About..." scenarios.
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext().
 * @param[in,out] buffer        A buffer allocated by the Alien Application to
 *                              receive the string.
 * @param[in,out] bufferSize    Size of the buffer - if buffer is NULL then the
 *                              required buffer size is returned here.
 *
 * @return 1 for success, 0 otherwise.
 */
int PicselVersion_getIssue(Picsel_Context *picselContext,
                           unsigned char  *buffer,
                           unsigned int   *bufferSize);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_VERSION_H */
