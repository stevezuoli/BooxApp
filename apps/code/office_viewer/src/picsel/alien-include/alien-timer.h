/**
 * Timer functions.
 *
 * The functions in this file must be implemented by the Alien
 * application integrator before linking with the TGV library.
 *
 * @file
 * $Id: alien-timer.h,v 1.26 2009/01/09 14:32:01 roger Exp $
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
#ifndef ALIEN_TIMER_H
#define ALIEN_TIMER_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Return the number of milliseconds from a fixed point in time.
 *
 * This time must be accurate to 20 milliseconds or better. The function is
 * used in a thread switch mechanism and for other internal purposes where
 * precise time counter is required.
 *
 * @param[in] alienContext  See PicselApp_start()
 * @return number of milliseconds
 *
 * @implement The implementation of this function should execute in less
 * then 10 microseconds. Failure to meet this performance target will
 * result in non-optimal performance of the Picsel system. Should you be
 * unable to meet this target in the Alien implementation please discuss
 * the matter with your Picsel representative.
 *
 * @ingroup TgvSystemTimer
 */
unsigned long AlienTimer_getUptime(Alien_Context *alienContext);


/**
 * @defgroup TgvTimeOfDay Time of Day
 * @ingroup TgvSystem
 *
 * Time of Day enquiries for the Device platform. It is used
 * for control of threading in the Picsel library and also
 * for product specific situations where the current time is
 * required.
 *
 * See @ref TgvSystemTimer
 *
 * @{
 */

/**
 * Return the current date and time.
 *
 * This time should be real and in a local time zone.
 *
 * The offset of local time from UTC must be notified to the Picsel
 * library using PicselLocale_setUtcDifference().
 *
 * @param[in]  alienContext  See PicselApp_start()
 * @param[out] sec           Seconds past the minute (0-59)
 * @param[out] min           Minutes past the hour (0-59)
 * @param[out] hour          Hour of the day (0-23)
 * @param[out] mday          Day of the month (1-31)
 * @param[out] mon           Month of the year (0-11)
 * @param[out] year          Years since 1900
 * @param[out] wday          Numbers of days since Sunday
 *                           (0-6; 0=Sun, 1=Mon, 6=Sat)
 * @param[out] yday          Number of days since 1st January (0-365)
 */
void    AlienTimer_getPlatformTime(Alien_Context *alienContext,
                                   int           *sec,
                                   int           *min,
                                   int           *hour,
                                   int           *mday,
                                   int           *mon,
                                   int           *year,
                                   int           *wday,
                                   int           *yday);

/**
 * Return the Daylight Savings Time offset in minutes.
 * The function is called by the Picsel library to ascertain whether
 * a date is during local Daylight Savings Time.
 *
 * @param[in] alienContext  See PicselApp_start()
 * @param[in] dateTime      Local date and time in seconds since Jan 1 1970
 *
 * @retval    dstOffset  Daylight Savings Time offset in minutes if date
 *                       during DST,
 * @retval    0          if the date is not during DST,
 * @retval   -1          if not implemented.
 *
 * @implement  If the country observes DST then clocks are typically
 * adjusted forward one hour near the start of spring and are adjusted
 * backward in autumn. This function should returns 60 if the date belongs
 * to the DST period. The actual periods involved vary between years and
 * between countries. Some countries do not have Daylight Savings Time
 * (like Japan), so 0 should always be returned for such locales.
 * If this function is implemented then the returned value is never
 * negative.
 *
 */
int AlienTimer_isDst(Alien_Context *alienContext,
                     long           dateTime);

/** @} */ /* end of doxygen group TgvTimeOfDay */

/**
 * @defgroup TgvSystemTimer Millisecond Timers
 * @ingroup TgvSystem
 *
 * See @ref TgvTimeOfDay
 * &nbsp;
 * @{
 */

/**
 * Request a timer event. Picsel usually has a constant chain of timers
 * outstanding, so after one expires, another is requested.
 *
 * Call PicselApp_timerExpiry() to signal the timer's expiry.
 *
 * @param[in]  alienContext See PicselApp_start()
 * @param[out] reference    Unique reference to the timer in use which is
 *                          returned by the Alien application.
 * @param[in]  ms           Timer interval in milliseconds
 *
 * @note The same timer reference will be used to request timers of
 * different intervals while an application is running.  e.g. when the app
 * is busy opening a document, a very short interval will be requested; when
 * the app is animating content, an interval of hundreds of even thousands
 * of milliseconds will be requested; when the app is idle and waiting for
 * user input an interval of MAX_INT may be requested.
 * @note The Picsel library will only ever have one timer active at a time.
 */

void AlienTimer_request(Alien_Context  *alienContext,
                        unsigned long  *reference,
                        unsigned long   ms);


/**
 * Cancel an outstanding timer request.
 *
 * Picsel is likely to need to cancel timers, for example to respond to
 * real-time user input, during a slow graphical animation. If timers cannot
 * be cancelled, the Picsel product will continue to work but will be much
 * less efficient, and may in some cases become unacceptable. If a short
 * timer were ignored because a long timer was pending and its cancellation
 * was ignored, the user experience would be poor.
 *
 * @param[in] alienContext  See PicselApp_start()
 * @param[in] reference     Unique reference to the timer in use.
 *
 * @retval 1                Confirms that timer cancellation is supported,
 *                          but the function does not return whether this
 *                          timer was actually cancelled.
 * @retval 0                If timers cannot be cancelled, this function
 *                          should return 0. However, performance will be
 *                          degraded; this is not recommended.
 * @see AlienTimer_request()
 */
int AlienTimer_cancel(Alien_Context *alienContext,
                      unsigned long *reference);


/**
 * Get high resolution performance timer for profiling, in microseconds
 *
 * The function is similar to AlienTimer_getUptime() which returns
 * time in milliseconds. The AlienTimer_getPerformanceTimeUs()
 * returns finer time in microseconds. The microsecond's counter can
 * overflow very quick. To avoid it a 64 bit value is used which is
 * splitted in to two 32 bit parts @c usLowPart and @c usHighPart.
 *
 * @param[in] alienContext  See PicselApp_start()
 * @param[out] usLowPart    On Exit: if function is supported, least-significant
 *                          part of time in microseconds;
 *                          not set, otherwise
 * @param[out] usHighPart   On Exit: if function is supported, most-significant
 *                          part of time in microseconds;
 *                          not set, otherwise
 *
 * @retval                  1 if platform supports this function,
 * @retval                  0 otherwise
 *
 * @implement It is optional for the Alien application to support
 * microseconds timers. If the Device platform supports this
 * the function must return 1 otherwise it returns 0.
 */
int AlienTimer_getPerformanceTimeUs(Alien_Context *alienContext,
                                    unsigned int  *usLowPart,
                                    unsigned int  *usHighPart);


/** @} */ /* end doxygen group TgvSystemTimer */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_TIMER_H */
