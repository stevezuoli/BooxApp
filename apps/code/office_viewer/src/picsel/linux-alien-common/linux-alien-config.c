/**
 *
 * Linux implementation of Alien config functions.
 *
 * Copyright (C) Picsel, 2005-2007. All Rights Reserved.
 *
 * $Id: linux-alien-config.c,v 1.9 2009/04/03 16:00:06 frank Exp $
 *
 * @file
 * @author Picsel Technologies Ltd
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


#include "picsel-entrypoint.h"

#include "alien-context.h"
#include "alien-config.h"
#include "preferences.h"
#include "linux-alien-config-fv.h"
#include "linux-alien-config.h"

#include "picsel-encoding.h"
#include "picsel-locale.h"
#include "picsel-language.h"

enum
{
    MaxPathLen = 256
};

void LinuxAlienConfig_setLocale(Alien_Context *ac)
{
    const char                      *locale;

    locale=getenv("LANG");
    if (locale != NULL && strlen(locale) <= 5)
    {
        char locale2[6];
        strncpy(locale2, locale, 5);
        locale2[5]='\x0';
        if (locale2[2]=='_')
          locale2[2]='-';

        PicselLocale_set(ac->picselContext,
                         locale2);
    }
    else
    {
        (void)PicselLocale_set(ac->picselContext, "en-gb");
    }
}

void LinuxAlienConfig_registerFonts(Alien_Context *ac)
{
    Preferences_fontRegister(&ac->preferences,ac->picselContext);
}

int AlienConfig_getPropertiesPath(Alien_Context* ac, char **path)
{
    char *propertiesPath;
    char *rootDir;
    int   rootDirLen = 0;
    char *createDir;
    int   epageDirectory = 1;
    char  rootDirBuf[MaxPathLen];

    ac = ac;

    rootDir = getenv("EPAGE_DIRECTORY");

    if (rootDir != NULL)
    {
        rootDirLen = strlen(rootDir);
    }

    if (rootDirLen == 0)
    {
        /* No EPAGE_DIRECTORY set, so fall back to current
         * working directory */
        rootDir = getcwd(rootDirBuf, sizeof(rootDirBuf));
        if ( rootDir == NULL )
        {
            return 1;
        }
        rootDirLen = strlen(rootDir);
        epageDirectory = 0;
    }

    createDir = malloc(rootDirLen + sizeof("/tmp/"));
    if (createDir != NULL)
    {
        sprintf(createDir, "%s/tmp/", rootDir);

        mkdir(createDir, 0755);

        if (epageDirectory == 1)
        {
            /* If the EPAGE_DIRECTORY property was set, it means we handle
             * files relative to it internally */
            free(createDir);
            propertiesPath = strdup("/tmp/");
        }
        else
        {
            /* Otherwise we can just use the one we created */
            propertiesPath = createDir;
        }
    }
    else
    {
        return 1;
    }

    *path = propertiesPath;
    return 0;
}

void AlienConfig_getPropertiesPathDone(Alien_Context* ac, char *path)
{
    ac = ac;
    free(path);
}

void AlienConfig_setInputType( Alien_Context *ac, PicselInputType type )
{
    assert( ac != NULL );

    ac->inputType = type;
    printf("Switching to %s input mode\n",
           ac->inputType == InputType_Key ? "key" : "command" );
}
