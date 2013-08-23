/**
 * Linux implementation of Alien config functions.
 *
 * Copyright (C) Picsel, 2005-2008. All Rights Reserved.
 *
 * $Id: linux-alien-config.h,v 1.1 2008/10/10 06:50:53 maxim Exp $
 *
 * @file
 * @author Picsel Technologies Ltd
 */

#ifndef LINUX_ALIEN_CONFIG_H
#define LINUX_ALIEN_CONFIG_H

#include "alien-types.h"

#if defined(__cplusplus)
extern "C" {
#endif /* (__cplusplus) */

/**
 * Setup the local in the normal way
 *
 * @param ac Alien context
 */
void LinuxAlienConfig_setLocale(Alien_Context *ac);

/**
 * Register generic fonts
 *
 * Registers fonts, encodings, pdf language packs for the language this build
 * was done for.
 *
 * @param ac Alien context
 */
void LinuxAlienConfig_registerFonts(Alien_Context *ac);

#if defined(__cplusplus)
}
#endif /* (__cplusplus) */

#endif /* !LINUX_ALIEN_CONFIG_H */
