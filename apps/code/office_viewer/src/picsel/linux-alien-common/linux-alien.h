/**
 *
 * Common definitions for linux alien implementation
 *
 * Copyright (C) Picsel, 2004. All Rights Reserved.
 *
 * $Id: linux-alien.h,v 1.2 2008/11/19 08:15:59 maxim Exp $
 *
 * @author Picsel Technologies Ltd
 * @file
 *
 */

#ifndef LINUX_ALIEN_H
#define LINUX_ALIEN_H

#include "alien-request.h"
#include "alien-context.h"

#if defined(__cplusplus)
extern "C" {
#endif /* (__cplusplus) */

/**
 * Static Timer ID
 */
#define TIMER_ID        1

/**
 * Data structure to store data about a user request.
 */
typedef struct AlienUserRequest_Request
{
    Alien_Context             *ac;            /**< Alien context */
    PicselUserRequest_Request *picselRequest; /**< Request data  */
} AlienUserRequest_Request;

/**
 * Returns the difference between two 'struct timeval's in milliseconds
 */
int difftimeval(struct timeval curTime, struct timeval reference);

/**
 * Reset the resource usage counters, used for measuring application
 * performance
 */
void LinuxAlien_resetResourceUsage(void);

/**
 * Display the resource usage counters
 */
void LinuxAlien_displayResourceUsage(void);

/**
 * Indicate that a timer has expired
 */
void LinuxAlien_timerExpired(void);

#if defined(__cplusplus)
}
#endif /* (__cplusplus) */

#endif /* !LINUX_ALIEN_H */
