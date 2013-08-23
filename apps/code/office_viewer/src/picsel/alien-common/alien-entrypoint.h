/**
 * Functions for recognising a Picsel product from a string
 *
 * Libraries supplied by Picsel may contain one or many products.
 * This set of utility functions makes it easier to select which
 * product to run from a string.  It is imagined that this will
 * be used to accelerate startup by reading such a string from
 * command line or from an environment property of some kind.
 *
 * Note that in order to make use of these functions, an alien
 * must have the following capabilities:
 *
 * 1) It must have working copies of certain standard C library
 *    routines:
 *       printf()
 *       strcmp()
 *       assert()
 *
 * 2) The alien must have no restrictions on static arrays of
 *    pointers [and consequently this code is *not* suitable for
 *    use on Symbian or Brew].
 *
 * Copyright (C) Picsel, 2008. All Rights Reserved.
 *
 * $Id: alien-entrypoint.h,v 1.9 2009/05/15 13:03:33 stever Exp $
 *
 * @file
 * @author Picsel Technologies Ltd
 *
 */

/**
 * @defgroup EntryPoint_API Interface for Selecting Product to launch
 * @{
 */

#ifndef ALIEN_ENTRYPOINT_H
#define ALIEN_ENTRYPOINT_H

#include "alien-types.h"
#include "picsel-entrypoint.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct Alien_StartupData
{
    Picsel_initialiseFn     initFn;   /* NULL if no appname given */
    const char             *filename; /* NULL if no filename given */
    int                     suppressGui; /* Nonzero if UI should
                                          * not be shown           */
}
Alien_StartupData;

/**
 * Turn an array of command line arguments into startup data
 *
 * @param[in]  argc         The number of entries in the argv[] array
 * @param[in]  argv         An array of command line arguments
 * @param[out] startupData  The initialisation structure for the desired app
 * @param[out] errorMsg     A string buffer to be filled with a message
 *                          if an error occurs.
 * @param[out] errorMsgSize Size of the errorMsg buffer.
 * @param allowAutoStart    If non-zero, automatically select application
 *                          if there is only one application present and
 *                          no application is specified.
 *
 * @return          0 if an error occurred, 1 otherwise
 */
int Alien_EntryPoint_processCommandLine(int                 argc,
                                        char               *argv[],
                                        Alien_StartupData  *startupData,
                                        char               *errorMsg,
                                        size_t              errorMsgSize,
                                        int                 allowAutoStart);



/**
 * Print the list of available applications into a supplied buffer
 *
 * @param[out] msg     A string buffer to be filled with a message
 *                     if an error occurs.
 * @param[out] msgSize Size of the errorMsg buffer.
 */
void Alien_EntryPoint_printValidApps(char   *msg,
                                     size_t  msgSize);



/**
 * Turn an app and file name into startup data
 *
 * @param[in]   appname     Name of application to start
 * @param[in]   filename    (Optional) name of document to load
 * @param[out]  startupData The initialisation structure for the desired app
 * @return          0 if an error occurred, 1 otherwise
 */
int Alien_EntryPoint_getStartupData(const char        *appname,
                                    const char        *filename,
                                    Alien_StartupData *startupData);



/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_ENTRYPOINT_H */
