/**
 * Language and Font Initialisation
 *
 * $Id: picsel-language.h,v 1.11 2008/12/11 16:34:21 roger Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @defgroup TgvLanguageInit Language and Font Initialisation
 * @ingroup TgvInitialisation
 *
 * Functions to enable access to character encodings, fonts, and
 * other language features. The Alien Application should call an
 * appropriate set of these, according to the world region where
 * the product will be shipped. These may then be used as required by
 * the Picsel library, according to requirements of the content.
 *
 * For example, supposing the Alien Application has called
 * Picsel_Font_Register_jajp() and Picsel_Font_Register_arar(), both
 * Japanese and Arabic fonts will be available to the Picsel library.
 * If a Japanese document is loaded into this application, the Picsel
 * library will display this in a Japanese font and not in an Arabic font,
 * given that the document reveals its encoding properly to the application.
 * On the other hand, if Picsel_Font_Register_jajp() was never called by
 * the application, the Japanese document will appear in garbled characters.
 *
 * Calls to these functions must be made from within AlienConfig_ready().
 * Calling each of these functions will increase the size of the linked
 * executable: see @ref TgvLink_Time_Configuration.
 *
 * @{
 */

#ifndef PICSEL_LANGUAGE_H
#define PICSEL_LANGUAGE_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Registers CMap files for Japanese language.
 *
 * Character code map files are used to accurately measure the size of letters
 * in PDF documents.  PDFs will load correctly without them but
 * letters or lines may overlap or have gaps that are too large.
 *
 * This function should be called from AlienConfig_ready().
 *
 * @pre You do not need to call this function unless you will also call
 * Picsel_Agent_Pdf().
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 *
 * @see Picsel_Font_Register_jajp(), Picsel_Encoding_Register_jajp()
 */
void Picsel_PdfLanguage_Register_jajp(Picsel_Context *picselContext);

/**
 * Registers CMap files for Korean language.
 *
 * Character code map files are used to accurately measure the size of letters
 * in PDF documents.  PDFs will load correctly without them but
 * letters or lines may overlap or have gaps that are too large.
 *
 * This function should be called from AlienConfig_ready().
 *
 * @pre You do not need to call this function unless you will also call
 * Picsel_Agent_Pdf().
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 *
 * @see Picsel_Font_Register_kokr(), Picsel_Encoding_Register_kokr()
 */
void Picsel_PdfLanguage_Register_kokr(Picsel_Context *picselContext);

/**
 * Registers CMap files for Simplified Chinese language.
 *
 * Character code map files are used to accurately measure the size of letters
 * in PDF documents.  PDFs will load correctly without them but
 * letters or lines may overlap or have gaps that are too large.
 *
 * This function should be called from AlienConfig_ready().
 *
 * @pre You do not need to call this function unless you will also call
 * Picsel_Agent_Pdf().
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 *
 * @see Picsel_Font_Register_zhcn(), Picsel_Encoding_Register_zhcn()
 */
void Picsel_PdfLanguage_Register_zhcn(Picsel_Context *picselContext);

/**
 * Registers CMap files for Traditional Chinese language.
 *
 * Character code map files are used to accurately measure the size of letters
 * in PDF documents.  PDFs will load correctly without them but
 * letters or lines may overlap or have gaps that are too large.
 *
 * This function should be called from AlienConfig_ready().
 *
 * @pre You do not need to call this function unless you will also call
 * Picsel_Agent_Pdf().
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 *
 * @see Picsel_Font_Register_zhtw(), Picsel_Encoding_Register_zhtw()
 */
void Picsel_PdfLanguage_Register_zhtw(Picsel_Context *picselContext);

/**
 * Registers the combined font files for the following languages:
 * Japanese, Korean, Chinese Traditional, Chinese Simplified.
 *
 * This function should be called from AlienConfig_ready() (along with
 * corresponding Picsel_PdfLanguage_Register_<lang> functions) for the
 * builds which include two or more of the above mentioned languages.
 * Using this combined font will have a smaller footprint compared to
 * registering fonts for even two of these languages individually.
 *
 * Individual Picsel_Font_Register_<lang> functions for these languages
 * (Picsel_PdfLanguage_Register_jajp(), Picsel_PdfLanguage_Register_kokr(),
 * Picsel_PdfLanguage_Register_zhcn(), Picsel_PdfLanguage_Register_zhtw())
 * should not be called together with this function.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 *
 * @see Picsel_Encoding_Register_jajp(), Picsel_Encoding_Register_kokr(),
 * Picsel_Encoding_Register_zhcn(), Picsel_Encoding_Register_zhtw()
 */
void Picsel_Font_Register_cjk(Picsel_Context *picselContext);

/**
 * Registers font files for Japanese language
 *
 * This function should be called from AlienConfig_ready() (along with
 * Picsel_PdfLanguage_Register_jajp() function) for the
 * builds which do not also include Korean, Simplified Chinese, or Traditional
 * Chinese language. (To register fonts for two or more of these languages,
 * Picsel_Font_Register_cjk() should be used instead .)
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 *
 * @see Picsel_Encoding_Register_jajp()
 */
void Picsel_Font_Register_jajp(Picsel_Context *picselContext);

/**
 * Registers font files for Korean language
 *
 * This function should be called from AlienConfig_ready() (along with
 * Picsel_PdfLanguage_Register_kokr() function) for the
 * builds which do not also include Japanese, Simplified Chinese, or Traditional
 * Chinese language. (To register fonts for two or more of these languages,
 * Picsel_Font_Register_cjk() should be used instead.)
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 *
 * @see Picsel_Encoding_Register_kokr()
 */
void Picsel_Font_Register_kokr(Picsel_Context *picselContext);

/**
 * Registers font files for Simplified Chinese
 *
 * This function should be called from AlienConfig_ready() (along with
 * Picsel_PdfLanguage_Register_zhcn() function) for the
 * builds which do not also include Japanese, Korean, or Traditional
 * Chinese language. (To register fonts for two or more of these languages,
 * Picsel_Font_Register_cjk() should be used instead.)
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 *
 * @see Picsel_Encoding_Register_zhcn()
 */
void Picsel_Font_Register_zhcn(Picsel_Context *picselContext);

/**
 * Registers font files for Traditional Chinese
 *
 * This function should be called from AlienConfig_ready() (along with
 * Picsel_PdfLanguage_Register_zhtw() function) for the
 * builds which do not include Japanese, Korean, or Simplified
 * Chinese language. (To register fonts for two or more of these languages,
 * Picsel_Font_Register_cjk() should be used instead.)
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 *
 * @see Picsel_Encoding_Register_zhtw()
 */
void Picsel_Font_Register_zhtw(Picsel_Context *picselContext);

/**
 * Registers font files for Latin languages.
 *
 * This function should be called from AlienConfig_ready() to register
 * fonts which are used by Latin languages.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_latin(Picsel_Context *picselContext);

/**
 * Registers font files for Arabic language
 *
 * This function should be called from AlienConfig_ready() to register
 * Linotype fonts used in Arabic language build.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_arar(Picsel_Context *picselContext);

/**
 * Registers font files for Thai language
 *
 * This function should be called from AlienConfig_ready() for all
 * builds which include Thai language.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_thth(Picsel_Context *picselContext);

/**
 * Registers font files for Bengali.
 *
 * This function should be called from AlienConfig_ready() for all
 * builds which include Bengali language.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_bnin(Picsel_Context *picselContext);

/**
 * Registers font files for Gujarati
 *
 * This function should be called from AlienConfig_ready() for all
 * builds which include Gujarati language.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_guin(Picsel_Context *picselContext);

/**
 * Registers font files for Hindi and Marathi.
 *
 * This function should be called from AlienConfig_ready() for all
 * builds which include Hindi or Marathi language.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_deva(Picsel_Context *picselContext);

/**
 * Registers font files for Kannada
 *
 * This function should be called from AlienConfig_ready() for all
 * builds which include Kannada language.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_knin(Picsel_Context *picselContext);

/**
 * Registers font files for Punjabi.
 *
 * This function should be called from AlienConfig_ready() for all
 * builds which include Punjabi language.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_pain(Picsel_Context *picselContext);

/**
 * Registers font files for Tamil
 *
 * This function should be called from AlienConfig_ready() for all
 * builds which include Tamil language.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_tain(Picsel_Context *picselContext);

/**
 * Registers font files for Telugu
 *
 * This function should be called from AlienConfig_ready() for all
 * builds which include Telugu language.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_tein(Picsel_Context *picselContext);

/**
 * Registers font files for Vietnamese
 *
 * This function should be called from AlienConfig_ready() for all
 * builds which include Vietnamese language.
 *
 * @pre Vietnamese fonts cannot be registered at the same time as the WGL4
 * fonts. See Picsel_Font_Register_wgl4().
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_vivn(Picsel_Context *picselContext);

/**
 * Registers WGL4 font files.
 *
 * Windows Glyph List 4 is a "PanEuropean" character set defined by Microsoft
 * which includes characters required by Western, Central, and Eastern European
 * writing systems as well as by Greek and Turkish.
 *
 * This function should be called from AlienConfig_ready().
 *
 * @pre WGL4 fonts cannot be registered at the same time as the Vietnamese
 * fonts. See Picsel_Font_Register_vivn().
 * @param picselContext   Set by AlienEvent_setPicselContext()
 */
void Picsel_Font_Register_wgl4(Picsel_Context *picselContext);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_LANGUAGE_H */
