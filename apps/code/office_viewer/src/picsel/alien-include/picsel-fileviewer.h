/**
 * Page View Control
 *
 * The functions in this file are offered by the Picsel library for
 * use by the Alien application. If these features are not required
 * by the application, it is not necessary to call these.
 *
 * $Id: picsel-fileviewer.h,v 1.18 2010/01/14 14:07:02 neilk Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @addtogroup TgvViewControl
 * @{
 */

#ifndef PICSEL_FILEVIEWER_H
#define PICSEL_FILEVIEWER_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Page turning options requested when calling PicselFileviewer_turnPage().
 */
typedef enum PicselPageTurn
{

    /**
     * Turn to first page of the document.
     */
    PicselPageTurn_First = (1<<16),

    /**
     * Turn to last page of the document.
     */
    PicselPageTurn_Last,

    /**
     * Turn to next page of the document.
     */
    PicselPageTurn_Next,

    /**
     * Turn to previous page of the document.
     */
    PicselPageTurn_Previous
}
PicselPageTurn;

/**
 * Turn page within the document. This can be one page forward or backward
 * relative to the current page, or it can be an absolute jump to the first
 * or last page. The Alien application can turn page forward/backward within
 * a valid range of pages. The number of valid pages depends on the current
 * document and varies during loading.
 *
 * @product   Document Viewing products, such as @ref TgvFileViewer.
 *
 * @see To go instead to a numbered page, see PicselFileviewer_gotoPage().
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] turn          Destination page to turn to.
 *                          See @ref PicselPageTurn.
 *
 * @return                  The queue status, normally 1.
 *                          See @ref TgvAsync_Queue.
 */
int PicselFileviewer_turnPage(Picsel_Context *picselContext,
                              PicselPageTurn  turn);

/**
 * Goto and view the specified page in the document. The Alien application
 * can go to any page of the loaded document within a valid range of pages.
 * The number of valid pages depends on the current document and varies
 * during loading.
 *
 * @product   Document Viewing products, such as @ref TgvFileViewer.
 *
 * @see To go instead one page forward or back, or to view the last page,
 * see PicselFileviewer_turnPage().
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] pageNumber    Destination page, numbered starting at 1 for
 *                          the first page, and then 2,3,4 etc. The page
 *                          number cannot be 0 or negative value.
 *                          The Picsel library reports the maximum valid
 *                          page number to the Alien application by
 *                          calling AlienScreen_pagesChanged().
 *
 * @return                  The queue status, normally 1.
 *                          See @ref TgvAsync_Queue.
 */
int PicselFileviewer_gotoPage(Picsel_Context  *picselContext,
                              int              pageNumber);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_FILEVIEWER_H */
