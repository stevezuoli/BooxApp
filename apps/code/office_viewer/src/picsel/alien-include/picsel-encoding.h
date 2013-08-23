/**
 * Character Encoding Initialisation
 *
 * $Id: picsel-encoding.h,v 1.7 2008/12/11 16:34:21 roger Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @addtogroup TgvLanguageInit
 *
 * @{
 */

#ifndef PICSEL_ENCODING_H
#define PICSEL_ENCODING_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Enable support for Japanese character encodings, where required
 * by the document content.
 *
 * This function enables EUC-JP, Shift-JIS and ISO-2022-JP encodings.
 *
 * @pre This function should be called from AlienConfig_ready().
 *
 * @param[in] picselContext   Set by AlienEvent_setPicselContext().
 */
int Picsel_Encoding_Register_jajp(Picsel_Context *picselContext);

/**
 * Enable support for Korean character encodings, where required
 * by the document content.
 *
 * This function enables the Windows-949 encoding.
 *
 * @pre This function should be called from AlienConfig_ready().
 *
 * @param[in] picselContext   Set by AlienEvent_setPicselContext().
 */
int Picsel_Encoding_Register_kokr(Picsel_Context *picselContext);

/**
 * Enable support for Simplified Chinese character encodings, where
 * required by the document content.
 *
 * This function enables the GB18030 encoding.
 *
 * @pre This function should be called from AlienConfig_ready().
 *
 * @param[in] picselContext   Set by AlienEvent_setPicselContext().
 */
int Picsel_Encoding_Register_zhcn(Picsel_Context *picselContext);

/**
 * Enable support for Traditional Chinese character encodings, where
 * required by the document content.
 *
 * This function enables the Big5 encoding.
 *
 * @pre This function should be called from AlienConfig_ready().
 *
 * @param[in] picselContext   Set by AlienEvent_setPicselContext().
 */
int Picsel_Encoding_Register_zhtw(Picsel_Context *picselContext);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_ENCODING_H */
