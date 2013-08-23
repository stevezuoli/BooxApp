/**
 * Selection region handling
 *
 * This file contains definitions and declarations needed for selection
 * handling.
 *
 * The contents of this file and the message described here should be
 * considered as being for internal use only. The intention is that this
 * will be refactored to be done a different way. Use at your own risk.
 *
 * $Id: picsel-selection.h,v 1.2 2009/12/04 13:32:14 malcolmh Exp $
 * @file picsel-selection.h
 */
/* Copyright (C) Picsel, 2005-2009. All Rights Reserved. */
/**
 * @defgroup TgvSelection Selection handling
 * @ingroup TgvFileViewer
 *
 * @{
 */

#ifndef PICSEL_SELECTION_H
#define PICSEL_SELECTION_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Additional information event types to @ref AlienInformation_Event
 */
enum
{
    /**
     * Notification sent to AlienEvent_information() in response to
     * PicselEdit_retrieveSelectionDetails(). The eventData will
     * point to an @ref AlienInformation_SelectionDetails structure.
     *
     * For internal use only - this should be refactored away in later
     * versions.
     */
    AlienInformation_SelectionRegion               = 0x1F000
};

/**
 * Bitfield of flags to describe the selection region.
 */
typedef enum PicselSelection_RegionInfo
{
    /** A selection is active */
    PicselSelection_RegionInfo_Active        = (1<<0),

    /** Set if the selection extends above the given start point */
    PicselSelection_RegionInfo_StartBelow    = (1<<1),

    /** Set if the selection extends to the left of the given start point */
    PicselSelection_RegionInfo_StartRight    = (1<<2),

    /** Set if the selection extends above the given end point */
    PicselSelection_RegionInfo_EndBelow      = (1<<3),

    /** Set if the selection extends to the left of the given end point. */
    PicselSelection_RegionInfo_EndRight      = (1<<4),

    /** Force size to be 32-bit */
    PicselSelection_RegionInfo_ForceEnumSize = (1<<16)
}
PicselSelection_RegionInfo;

/**
 * A structure passed as the data for an @ref AlienInformation_Selection
 * event. For internal use only.
 *
 * All int values are in Picsel internal units. See @ref
 * TgvCoordinate_Space_Zoom_Pan.
 */
typedef struct AlienInformation_SelectionRegionInfo
{
    /** View handle         */
    Picsel_View                *picselView;

    /** Bitfield of flags   */
    PicselSelection_RegionInfo  flags;

    /** Start point x coord */
    int                         startx;
    /** Start point y coord */
    int                         starty;

    /** End point x coord   */
    int                         endx;

    /** End point y coord   */
    int                         endy;
}
AlienInformation_SelectionRegionInfo;


/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_SELECTION_H */
