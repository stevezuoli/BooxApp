/**
 *
 * Common definitions for linux alien file system notification module
 *
 * Copyright (C) Picsel, 2007. All Rights Reserved.
 *
 * $Id: alien-filesys-data.h,v 1.1 2008/10/10 06:50:53 maxim Exp $
 *
 * @author Picsel Technologies Ltd
 * @file
 *
 */

#ifndef ALIEN_FILESYS_DATA_H
#define ALIEN_FILESYS_DATA_H

#include "alien-context.h"

#if defined(__cplusplus)
extern "C" {
#endif /* (__cplusplus) */

/**
 * Finalise function for AlienFileSysData object.
 *
 * This function should be called to free all memory created for the
 * AlienFileSysData object and itself.
 */
void AlienFilesys_finalise( void );

#if defined(__cplusplus)
}
#endif /* (__cplusplus) */

#endif /* !ALIEN_FILESYS_DATA_H */
