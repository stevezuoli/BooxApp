/**
 * Generic types used by the Picsel interface
 *
 * $Id: picsel-types.h,v 1.6 2008/12/11 16:34:23 roger Exp $
 * @file
 */
/* Copyright (C) Picsel, 2008. All Rights Reserved. */
/**
 * @addtogroup TgvCoreTypes
 * @{
 */

#ifndef PICSEL_TYPES_H
#define PICSEL_TYPES_H

/**
 * Type used for UTF8 format strings
 *
 * Generally all strings within the Picsel/Alien interface are UTF-8.
 *
 * (Some older interfaces use the 'char *' or 'unsigned char *' directly.)
 */
typedef char Picsel_Utf8;


/**
 * Type used for TGV API results.
 *
 * Many (but not all) Picsel* APIs will return this type.
 *
 * In most cases PicselCommand_Queued means that the API function has
 * successfully queued a command that will be completed asynchronously, not
 * that the operation has fully completed.
 *
 * PicselCommand_Failed indicates an immediate failure.
 *
 * See @ref TgvAsync_Queue for further explanation.
 */
typedef enum PicselCommand_Result
{
    PicselCommand_Queued    = 1, /**< The operation has started, and will
                                      complete or fail later, unless the
                                      function explicitly states otherwise.*/

    PicselCommand_Failed    = 0,  /**< The operation has failed to start. */


    PicselCommand_ForceSize = 1<<16 /**< This value is never returned. It is
                                         only here to force the enum to be
                                         32 bits in size.*/
}
PicselCommand_Result;


/**
 * @}
 */

#endif /* !PICSEL_TYPES_H */
