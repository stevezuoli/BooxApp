/**
 * Entrypoint code common to more than one alien implementation
 *
 * Copyright (C) Picsel, 2008-2009. All Rights Reserved.
 *
 * @author Picsel Technologies Ltd
 *
 * $Id: alien-entrypoint.c,v 1.44 2009/11/13 12:39:36 mikea Exp $
 */


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "alien-entrypoint.h"
#include "alien-entrypoint-apptest.h"
#include "picsel-entrypoint.h"

/* Windows doesn't have 'snprintf()' - but it does have an alternative */


typedef Picsel_initialiseFn (GetEntryPointFn)(void);

typedef enum EntryPoint_Flags
{
    EntryPoint_Flag_MustHaveFilename = 1 << 0, /**< application *must* have a
                                                    filename parameter */
    EntryPoint_Flag_SuppressGui      = 1 << 1  /**< suppress the GUI */
}
EntryPoint_Flags;

typedef struct EntryPoint
{
    const char       *name;      /**< name as specified on the command line */
    const char       *desc;      /**< textual description of app */
    EntryPoint_Flags  flags;     /**< flags */
    GetEntryPointFn  *getInitFn; /**< Function to get entrypoint for app */
}
EntryPoint;



/**
 * The set of names/descriptions for all possible apps.  Initialisation
 * functions are filled in by initialiseEntrypoints at runtime.  The order
 * of ifdefs here and in initialiseEntrypoints must be kept in sync.
 */
static EntryPoint entryPoints[] =
{
    { "fileviewer",
      "File Viewer",
      0,
      Picsel_EntryPoint_FileViewer
    },

    { NULL,
      NULL,
      0,
      NULL
    }
};


static EntryPoint* entryPointFromName(const char *appname)
{
    EntryPoint *entryPoint;

    /* assert that we have at least one application */
    assert(entryPoints[0].desc != NULL);

    for (entryPoint        = entryPoints;
         entryPoint->desc != NULL;  /* compare the description, because
                                     * the initFn is NULL for ath */
         entryPoint++)
    {
        if (strcmp(entryPoint->name, appname) == 0)
        {
            return entryPoint;
        }
    }

    return NULL;
}



/* Read command line for possible startup parameters.
 *
 * Format is:
 *   tgv [<appname> [<initialFile>]]
 *
 * where <appname> matches a string from alien-common/alien-entrypoint.c
 */
int Alien_EntryPoint_processCommandLine(int                 argc,
                                        char               *argv[],
                                        Alien_StartupData  *startupData,
                                        char               *errorMsg,
                                        size_t              errorMsgSize,
                                        int                 allowAutoStart)
{
    EntryPoint *entryPoint   = NULL;

    startupData->initFn      = NULL;
    startupData->filename    = NULL;
    startupData->suppressGui = 0;

    /* A NULL string as the only command line argument may indicate
     * a TGV apptest.
     */
    if ( (argc == 2) && (strlen(argv[1]) == 0) )
    {
        return 1;
    }

    /* If an application name is given then it must be a valid one */
    if ( argc >= 2 )
    {
        entryPoint = entryPointFromName(argv[1]);

        if ( entryPoint == NULL )
        {
            /* We didn't recognise the appname */
            Alien_EntryPoint_printValidApps(errorMsg, errorMsgSize);
            return 0;
        }

        /* Set the filename only if we have a 3rd argument and it's
         * not just the empty string
         */
        if ( (argc >= 3) && (*(argv[2] )!= '\0') )
        {
            startupData->filename = argv[2];
        }

        if ( ((entryPoint->flags & EntryPoint_Flag_MustHaveFilename) != 0)
          && (startupData->filename == NULL) )
        {
            /* No filename was supplied, or we failed to copy it. */
            snprintf(errorMsg,
                     errorMsgSize,
                     "Usage: tgv <application> [filename]\n");
            errorMsg[errorMsgSize - 1] = '\0';

            snprintf(errorMsg + strlen(errorMsg),
                     errorMsgSize - strlen(errorMsg),
                     "\n");
            errorMsg[errorMsgSize - 1] = '\0';

            snprintf(errorMsg + strlen(errorMsg),
                     errorMsgSize - strlen(errorMsg),
                     "For the '%s' application you must supply a filename!\n",
                     entryPoint->name);
            errorMsg[errorMsgSize - 1] = '\0';

            return 0;
        }

        if ( argc >= 4 )
        {
            /* Too many arguments.  Must be an error */
            Alien_EntryPoint_printValidApps(errorMsg, errorMsgSize);
            return 0;
        }
    }
    else if (allowAutoStart)
    {
        /* no parameters - if only one app is included, use that */
        if (entryPoints[0].desc != NULL &&
            entryPoints[1].desc == NULL)
        {
            entryPoint = &entryPoints[0];
        }
    }

    if (entryPoint)
    {
        startupData->initFn      = entryPoint->getInitFn();
        startupData->suppressGui = (entryPoint->flags &
                                    EntryPoint_Flag_SuppressGui) != 0;
    }

    return 1;
}



void Alien_EntryPoint_printValidApps(char   *errorMsg,
                                     size_t  errorMsgSize)
{
    EntryPoint *entryPoint;

    /* assert that we have at least one application */
    assert(entryPoints[0].desc != NULL);

    snprintf(errorMsg,
             errorMsgSize,
             "Usage: tgv <application> [filename]\n");
    errorMsg[errorMsgSize - 1] = '\0';

    snprintf(errorMsg + strlen(errorMsg),
             errorMsgSize - strlen(errorMsg),
             "\n");
    errorMsg[errorMsgSize - 1] = '\0';

    snprintf(errorMsg + strlen(errorMsg),
             errorMsgSize - strlen(errorMsg),
             "Valid applications for this build:\n");
    errorMsg[errorMsgSize - 1] = '\0';

    for (entryPoint = entryPoints;
         entryPoint->desc != NULL;
         entryPoint++)
    {
        snprintf(errorMsg + strlen(errorMsg),
                 errorMsgSize - strlen(errorMsg),
                 "%-20s: %s\n", entryPoint->name, entryPoint->desc);
        errorMsg[errorMsgSize - 1] = '\0';
    }
}



int Alien_EntryPoint_getStartupData(const char        *appname,
                                    const char        *filename,
                                    Alien_StartupData *startupData)
{
    EntryPoint *entryPoint;

    entryPoint = entryPointFromName(appname);

    if (entryPoint)
    {
        startupData->initFn      = entryPoint->getInitFn();
        startupData->filename    = filename;
        startupData->suppressGui = (entryPoint->flags &
                                    EntryPoint_Flag_SuppressGui) != 0;
        return 1;
    }

    return 0;
}
