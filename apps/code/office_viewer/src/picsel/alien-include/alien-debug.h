/**
 * Debugging output commands for Testing
 *
 * The functions in this file must be implemented by the Alien application
 * before linking with the TGV library.
 *
 * @file
 * $Id: alien-debug.h,v 1.9 2008/12/11 16:34:18 roger Exp $
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
/**
 * @defgroup TgvTest Testing and Debugging
 * @ingroup TgvSystem
 *
 * It is unusual for computer software to work correctly first time,
 * because it is written by humans. Therefore the Picsel TGV environment
 * provides a number of hooks to assist in testing and debugging, both
 * within Picsel code and in the Alien application.
 *
 * @{
 */

#ifndef ALIEN_DEBUG_H
#define ALIEN_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Logs or displays a status message for debugging. This is used by the
 * Picsel library to indicate its operation. The function should be
 * implemented to allow a software engineer to review all of the messages
 * in sequence, after the application has run. This is typically done on
 * a device by saving them to a log file, which can be viewed or uploaded
 * later. Picsel may request these to assist in resolving support requests.
 * A newline may be indicated with \n
 *
 * @verbatim \n @endverbatim
 *
 * @param string    A text message to output from the Picsel library
 */
void AlienDebug_output(const char *string);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_DEBUG_H */
