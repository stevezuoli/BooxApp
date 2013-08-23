/**
 *
 * Linux implementation of Alien config functions.
 *
 * Copyright (C) Picsel, 2005. All Rights Reserved.
 *
 * $Id: linux-alien-config-fv.c,v 1.4.2.1 2010/07/27 09:42:35 alans Exp $
 *
 * @file
 * @author Picsel Technologies Ltd
 *
 */


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "alien-context.h"
#include "alien-config.h"
#include "picsel-locale.h"
#include "picsel-agent.h"
#include "picsel-encoding.h"
#include "picsel-language.h"

#include "picsel-config-fileviewer.h"
#include "picsel-config.h"

#include "linux-alien-config.h"

void Fv_AlienConfig_ready(Alien_Context *ac)
{
    char *path = NULL;
    int   ret;

    LinuxAlienConfig_setLocale(ac);
    LinuxAlienConfig_registerFonts(ac);

    ret = AlienConfig_getPropertiesPath(ac, &path);
    if ( ret != 0 )
    {
        assert(path == NULL);
        printf("Out of memory\n");
        abort();
    }
    /* To setup persistent cacheing, cookies, history and
     * bookmarks, set:
     * PicselConfig_settingsPath to a writeable directory here */
    assert(path != NULL);
    PicselConfig_setString( ac->picselContext,
                            PicselConfig_settingsPath,
                            path);
    AlienConfig_getPropertiesPathDone(ac, path);
}

/*---------------------------------------------------------------------*/
/*---------------- FileViewer specific functions ----------------------*/
/*---------------------------------------------------------------------*/

int AlienConfig_initialiseAgents(            Picsel_Context *pc,
                                 /*@unused@*/Alien_Context  *ac)
{
    /* Here is an example of how to register agents. */
    /*if (!Picsel_Agent_Html(pc))
        return 0;

    if (!Picsel_Agent_Word(pc))
        return 0;
    */

    if (!Picsel_Agent_RegisterAll(pc))
    {
        return 0;
    }

    return 1;
}
