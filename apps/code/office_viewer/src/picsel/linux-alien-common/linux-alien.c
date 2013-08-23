/**
 * Linux alien implementation -- Functions common to all GNU/Linux alien apps
 *
 * Copyright (C) Picsel, 2004-2008. All Rights Reserved.
 *
 * @author Picsel Technologies Ltd
 *
 * $Id: linux-alien.c,v 1.10 2009/09/17 09:08:05 alans Exp $
 */


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "linux-alien.h"
#include "alien-context.h"
#include "alien-stack.h"

/* Picsel alien interface include files */
#include "alien-debug.h"
#include "alien-error.h"
#include "alien-memory.h"
#include "alien-notify.h"
#include "alien-timer.h"
#include "alien-request.h"

#include "picsel-locale.h"
#include "picsel-screen.h"

#ifdef DEBUG
#define DBUGF(s) printf s
#else /* DEBUG */
#define DBUGF(s)
#endif /* DEBUG */

/* Start times for measuring resource usages */
static struct timeval startUser;
static struct timeval startSys;
static struct timeval startTime;

/* -- Structures -- */

typedef struct
{
    unsigned long ref;
    unsigned long ms;
} timeParam;

const PicselPal_stackDirection AlienPal_stackDirection =
                               PicselPal_stackGrowsDown;
#ifdef __i386__
const int AlienPal_stackIndex = 4;
#else /* __i386__ */
const int AlienPal_stackIndex = 9;
#endif /* __i386__ */
const int AlienPal_discardedStack = 16;

int difftimeval(struct timeval curTime, struct timeval reference)
{
    int dsec = curTime.tv_sec - reference.tv_sec;
    int dmsec = (curTime.tv_usec - reference.tv_usec) / 1000;

    if (dmsec < 0)
    {
        dsec--;
        dmsec += 1000;
    }

    return dsec * 1000 + dmsec;
}

void LinuxAlien_displayResourceUsage(void)
{
    struct rusage  usage;
    int            err;
    struct timeval curr, diff;

    err = getrusage(RUSAGE_SELF, &usage);
    if (err == 0)
    {
        timersub(&usage.ru_utime, &startUser, &diff);
        DBUGF(("user time %ld.%.06ld\n", diff.tv_sec, diff.tv_usec));

        timersub(&usage.ru_stime, &startSys, &diff);
        DBUGF(("sys  time %ld.%.06ld\n", diff.tv_sec, diff.tv_usec));
    }
    else
    {
        DBUGF(("getrusage failed: %s\n", strerror(errno)));
    }

    err = gettimeofday(&curr, NULL);
    if (err==0)
    {
        timersub(&curr, &startTime, &diff);
        DBUGF(("wall time %ld.%.06ld\n", diff.tv_sec, diff.tv_usec));
    }
    else
    {
        DBUGF(("gettimeofday failed: %s\n", strerror(errno)));
    }
}

void LinuxAlien_resetResourceUsage(void)
{
    struct rusage  usage;
    int            err;

    err = getrusage(RUSAGE_SELF, &usage);
    if (err == 0)
    {
        startUser = usage.ru_utime;
        startSys  = usage.ru_stime;
    }
    else
    {
        DBUGF(("getrusage failed: %s\n", strerror(errno)));
    }

    err = gettimeofday(&startTime, NULL);
    if (err != 0)
    {
        DBUGF(("gettimeofday failed: %s\n", strerror(errno)));
    }
}

void AlienEvent_setPicselContext(Alien_Context  *ac, Picsel_Context *pc)
{
    assert(ac);

    ac->picselContext = pc;
}

///not implement here for onyx, as we can't get globalAlienContext
//directly
//Picsel_Context *AlienMemory_getPicselContext(void)
//{
//    return globalAlienContext->picselContext;
//}

/**
 * The return value will overflow every 49.7 days!
 */
unsigned long AlienTimer_getUptime(Alien_Context *ac)
{
    static long startTime = 0;
    long ct;

    ac = ac;

    {
        /* Use real time */
        struct timeval tv;

        if (gettimeofday(&tv, NULL))
            assert("AlienTimer_getUptime()[2]" == NULL);

        ct = (long)((long long)tv.tv_sec * 1000) + (long)tv.tv_usec / 1000;
    }

    if (startTime == 0)
    {
        startTime = ct;
        return 0;
    }

    return (unsigned long)(ct - startTime);
}


int AlienTimer_getPerformanceTimeUs(Alien_Context *ac,
                                    unsigned int              *usLowPart,
                                    unsigned int              *usHighPart)
{
    struct timeval curr;
    int            err;
    long long      us;

    ac = ac;

    err = gettimeofday(&curr, NULL);
    if (err)
        return 0;

    us = (long long)curr.tv_sec * 1000000 + (long long)curr.tv_usec;
    *usLowPart  = (unsigned int)us;
    *usHighPart = (unsigned int)(us>>32);
    return 1;
}


void    AlienTimer_getPlatformTime(Alien_Context *ac,
                                             int *sec,
                                             int *min,
                                             int *hour,
                                             int *mday,
                                             int *mon,
                                             int *year,
                                             int *wday,
                                             int *yday )
{
    time_t     currentTime;
    struct tm *timeDate;

    assert(ac != NULL);

    assert(sec  != NULL);
    assert(min  != NULL);
    assert(hour != NULL);
    assert(mday != NULL);
    assert(mon  != NULL);
    assert(year != NULL);
    assert(wday != NULL);
    assert(yday != NULL);

    time(&currentTime);
    timeDate = localtime(&currentTime);

    *sec  = (int)timeDate->tm_sec;
    *min  = (int)timeDate->tm_min;
    *hour = (int)timeDate->tm_hour;

    *mday = (int)timeDate->tm_mday;
    *mon  = (int)timeDate->tm_mon;
    *year = (int)timeDate->tm_year;
    *wday = (int)timeDate->tm_wday;
    *yday = (int)timeDate->tm_yday;
}

int AlienTimer_isDst(Alien_Context *ac,
                     long           dateTime)
{
    ac = ac;
    dateTime = dateTime;

    return -1;
}

void AlienUserRequest_requestTerminate(Alien_Context              *ac,
                                       PicselUserRequest_Request  *request)
{
    ac = ac;
    request = request;

    /* terminate current active request */
}
