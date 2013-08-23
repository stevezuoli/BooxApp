/**
 * Locale Format and Time Initialisation
 *
 * $Id: picsel-locale.h,v 1.21.4.2 2010/08/18 07:36:04 gyunam Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @addtogroup TgvLanguageInit
 *
 * @{
 */

#ifndef PICSEL_LOCALE_H
#define PICSEL_LOCALE_H

#include "alien-types.h"
#include "alien-legacy.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Sets a locale
 *
 * A locale signifies a set of certain parameters that are customized for a
 * particular locality. The locale setting determines your application's
 * default encoding as well as the default number display format.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 * @param locale          The locale in RFC3066 format
 *
 * The format is in the form "en-gb", which is
 * Language=English, Dialect=British,
 * i.e. an ISO 639-1 Languge code
 *      followed by an ISO 3166-2 country code.
 *
 * The locale is case-insensitive.
 * A full list of supported locales follows. Please consult your
 * release notes/contract to determine which are available in your build.
 *    - @c ar-ar   Arabic
 *    - @c az-cyrl-az  Azeri, Cyrillic - Azerbaijan
 *    - @c az-latn-az  Azeri, Latin - Azerbaijan
 *    - @c bn-in   Begali - India
 *    - @c bg-bg   Bulgarian - Bulgaria
 *    - @c cs-cz   Czech - Czech Republic
 *    - @c da-dk   Danish - Denmark
 *    - @c de-de   German - Germany
 *    - @c el-gr   Greek - Greece
 *    - @c en-gb   English - United Kingdom
 *    - @c en-us   English - United States
 *    - @c es-co   Spanish - Colombia
 *    - @c es-do   Spanish - Dominican Republic
 *    - @c es-es   Spanish - Spain
 *    - @c et-ee   Estonian - Estonia
 *    - @c fi-fi   Finnish - Finland
 *    - @c fr-ca   French - Canada
 *    - @c fr-fr   French - France
 *    - @c gu-in   Gujarati - India
 *    - @c he-il   Hebrew - Israel
 *    - @c hi-in   Hindi - India
 *    - @c hr-hr   Croatian - Croatia
 *    - @c hu-hu   Hungarian - Hungary
 *    - @c it-it   Italian - Italy
 *    - @c ja-jp   Japanese - Japan
 *    - @c kk-kz   Kazakh - Kazakhstan
 *    - @c kn-in   Kannada - India
 *    - @c ko-kr   Korean - Korea
 *    - @c lt-lt   Lithuanian - Lithuania
 *    - @c lv-lv   Latvian - Latvia
 *    - @c ml-in   Malayalam - India
 *    - @c mr-in   Marathi - India
 *    - @c nl-nl   Dutch - The Netherlands
 *    - @c no-no   Norwegian
 *    - @c or-in   Oriya - India
 *    - @c pa-in   Punjabi - India
 *    - @c pl-pl   Polish - Poland
 *    - @c pt-br   Portuguese - Brazil
 *    - @c pt-pt   Portuguese - Portugal
 *    - @c ro-ro   Romanian - Romania
 *    - @c ru-ru   Russian - Russia
 *    - @c sk-sk   Slovak - Slovakia
 *    - @c sl-si   Slovenian - Slovenia
 *    - @c sv-se   Swedish - Sweden
 *    - @c te-in   Telugu - India
 *    - @c ta-in   Tamil - India
 *    - @c th-th   Thai - Thailand
 *    - @c tr-tr   Turkish - Turkey
 *    - @c uk-ua   Ukrainian - Ukraine
 *    - @c vi-vn   Vietnamese - Vietnam
 *    - @c zh-cn   Chinese - China
 *    - @c zh-tw   Chinese - Taiwan
 *
 * If neither this function nor PicselLocale_setFromLocaleId() is called, Picsel will
 * default to en-gb.  If the function fails to set a language, then the
 * currently set language will be used, (or default).
 *
 * This function should only be called when a document download/translation
 * is not in progress. To set the locale for the first document this function
 * should be called from AlienConfig_ready().
 * Any change in the locale settings will not affect the current document
 * unless it is reloaded.
 *
 * @note
 * - Some behaviour (for example, replacement of backslash with the Yen
 * character for the ja-jp locale) will not change if calls to
 * PicselLocale_set() are made after AlienConfig_ready() returns.
 * - The number display format prescribed by your locale selection
 * can be overridden using PicselLocale_setNumberConventions().
 * - Date display format and the local time zone have to be specified
 * separately using PicselLocale_setDateStrings() and
 * PicselLocale_setUtcDifference().
 *
 * @post The locale string is copied by Picsel and can be released after this call.
 *
 * @retval 1 if success, @retval 0 if failure.
 * Failure means that a valid locale string was not provided.
 *
 */
int PicselLocale_set(Picsel_Context *picselContext,
                     const char     *locale);

/**
 * Sets a locale from a Microsoft Locale Identifier
 *
 * @param picselContext     Set by AlienEvent_setPicselContext()
 * @param localeId          The Locale Identifier
 *
 * A Locale Identifier comprises 4 hexadecimal digits defining a main
 * language and sub-language.  This representation of the locale is used
 * mainly (but not exclusively) in Microsoft Windows/WinCE configurations.
 * Locale Identifier constants are listed by Microsoft, currently at:
 * <a href="http://msdn.microsoft.com/en-us/library/ms776260(VS.85).aspx">
 * "Locale Identifier Constants And Strings"</a>.
 *
 * A full list of supported Locale Identifiers follows. Please consult your
 * release notes/contract to determine which are available in your build.
 *   - @c 0x3801   Arabic
 *   - @c 0x082c   Azeri, Cyrillic - Azerbaijan
 *   - @c 0x042c   Azeri, Latin - Azerbaijan
 *   - @c 0x0445   Begali - India
 *   - @c 0x0402   Bulgarian - Bulgaria
 *   - @c 0x0405   Czech - Czech Republic
 *   - @c 0x0406   Danish - Denmark
 *   - @c 0x0407   German - Germany
 *   - @c 0x0408   Greek - Greece
 *   - @c 0x0809   English - United Kingdom
 *   - @c 0x0409   English - United States
 *   - @c 0x240a   Spanish - Colombia
 *   - @c 0x1c0a   Spanish - Dominican Republic
 *   - @c 0x0c0a   Spanish - Spain
 *   - @c 0x0425   Estonian - Estonia
 *   - @c 0x040b   Finnish - Finland
 *   - @c 0x0c0c   French - Canada
 *   - @c 0x040c   French - France
 *   - @c 0x0447   Gujarati - India
 *   - @c 0x040d   Hebrew - Israel
 *   - @c 0x0439   Hindi - India
 *   - @c 0x041a   Croatian - Croatia
 *   - @c 0x040e   Hungarian - Hungary
 *   - @c 0x0410   Italian - Italy
 *   - @c 0x0411   Japanese - Japan
 *   - @c 0x044b   Kannada - India
 *   - @c 0x043f   Kazakh - Kazakhstan
 *   - @c 0x0412   Korean - Korea
 *   - @c 0x0427   Lithuanian - Lithuania
 *   - @c 0x0426   Latvian - Latvia
 *   - @c 0x044c   Malayalam - India
 *   - @c 0x044e   Marathi - India
 *   - @c 0x0413   Dutch - The Netherlands
 *   - @c 0x0414   Norwegian
 *   - @c 0x0448   Oriya - India
 *   - @c 0x0446   Punjabi - India
 *   - @c 0x0415   Polish - Poland
 *   - @c 0x0416   Portuguese - Brazil
 *   - @c 0x0816   Portuguese - Portugal
 *   - @c 0x0418   Romanian - Romania
 *   - @c 0x0419   Russian - Russia
 *   - @c 0x041b   Slovak - Slovakia
 *   - @c 0x0424   Slovenian - Slovenia
 *   - @c 0x041d   Swedish - Sweden
 *   - @c 0x0449   Telugu - India
 *   - @c 0x044a   Tamil - India
 *   - @c 0x041e   Thai - Thailand
 *   - @c 0x041f   Turkish - Turkey
 *   - @c 0x0422   Ukrainian - Ukraine
 *   - @c 0x042a   Vietnamese - Vietnam
 *   - @c 0x0804   Chinese - China
 *   - @c 0x0404   Chinese - Taiwan
 *
 * If neither this function nor PicselLocale_set() is called, Picsel will
 * default to en-gb.  If the function fails to set a language, then the
 * currently set language will be used, (or default).
 * See PicselLocale_set() for further information.
 *
 * @retval 1 if success, @retval 0 if failure.
 * Failure means that a valid Locale Identifier was not provided.
 *
 */
int PicselLocale_setFromLocaleId(Picsel_Context      *picselContext,
                                 const unsigned int   localeId);
/**
 * Sets the difference between the local time zone and UTC in seconds.
 * This should include any daylight savings that are in effect.
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 * @param secondsDiff     Local time zone's difference from UTC (Coordinated Universal Time) in
 *                        seconds.  This should not include the Daylight
 *                        Savings Time offset.  In other words, it is the
 *                        difference between UTC and local winter time.
 *                        The value supplied as calculated follows the
 *                        standard used to declare time zones difference
 *                        from UTC; a positive value is used for zones further
 *                        east and a negative value for those further west.
 *                        @n Some examples:
 *                        @n Tokyo is UTC +9 hrs so the value supplied would be
 *                        9*60*60 = 32400.
 *                        @n Hawaii is UTC -10 hrs so the value supplied would be
 *                        -10*60*60 = -36000.

 * @param dst             Daylight Saving Time offset in minutes, if
 *                        Daylight Savings Time is applied in the target
 *                        country or region when this function is called.
 *                        In the UK and US this is usually 60 minutes.
 *                        If it is winter time, set this to 0.
 *
 * @retval 1 if success, @retval 0 if failure.
 * Failure means that you've not specified a valid value for @c secondsDiff.
 *
 */
int PicselLocale_setUtcDifference(Picsel_Context *picselContext,
                                  int             secondsDiff,
                                  int             dst);


/**
 * Overrides the locale-specific number-format conventions.
 *
 * Field semantics are identical to their namesakes in the C89
 * function localeconv().
 *
 * @param picselContext   Set by AlienEvent_setPicselContext()
 * @param decimalPoint    The radix character used to format non-monetary
 *                        quantities, e.g., "," or ".".  Can be at most
 *                        4 characters, including nul byte.  This value is
 *                        required.
 * @param thousandsSep    The character used to separate groups of digits
 *                        before the decimal-point character, e.g., " " or
 *                        ",".  Can be at most 4 characters, including nul
 *                        byte.  May be NULL, in which case digits are not
 *                        grouped. Optional.
 * @param grouping        A string whose elements taken as one-byte integer
 *                        values indicate the size of each group of digits,
 *                        e.g., "\3\0", "\3\2\2\x7f".  See C89 localeconv()
 *                        documentation for more details.  Can be at most 8
 *                        characters.  May be NULL, in which case digits
 *                        are not grouped. Optional.
 *
 * @post The strings are copied by Picsel and can be released after this call.
 * @retval 1 if success, @retval 0 if failure.  Failure means that one or more of the
 * fields is too long, or that you've not specified a value for
 * the required parameter @c decimalPoint.
 *
 */
int PicselLocale_setNumberConventions(Picsel_Context *picselContext,
                                      const char     *decimalPoint,
                                      const char     *thousandsSep,
                                      const char     *grouping);

/**
 * Sets the strings used for date localisation.
 *
 * The format strings should consist of a string of simple C89 strftime()
 * format specifiers, namely %A, %a, %B, %b, %C, %d, %e, %H, %h, %I,
 * %M, %m, %p, %S, %Y, or %y.  It should not contain compound
 * specifiers, particularly %c, %X, or %x.  For details of the
 * meanings of these format specifiers, see the documentation on the
 * C89 function strftime().
 *
 * @param picselContext     Set by AlienEvent_setPicselContext()
 *
 * @param dateLongFormat    Format to use for long date strings.
 *                          By default: "%d %B %Y"
 * @param dateFormat        Format to use for date strings.
 *                          By default: "%d/%m/%Y"
 * @param timeFormat        Format to use for time strings.
 *                          By default: "%H/%M/%S"
 * @param dateAndTimeFormat Format to use for combined date and time strings.
 *                          By default: "%d/%m/%Y %H/%M/%S"
 * @param dayNames          Names of the days of the week
 *                          as an array of seven pointers to strings
 *                          starting with the locale's equivalent of "Sunday".
 *                          E.g., {"Sunday", "Monday", "Tuesday", ...}
 * @param dayNamesAbbr      Abbreviated names of the days of the week
 *                          as an array of seven pointers to strings
 *                          starting with the locale's equivalent of "Sun".
 *                          E.g., {"Sun", "Mon", "Tue",...}
 * @param monthNames        Names of the calendar months
 *                          as an array of twelve pointers to strings
 *                          starting  with the locale's equivalent of "January".
 *                          E.g., {"January", "February", "March", ...}
 * @param monthNamesAbbr    Abbreviated names of the calendar months
 *                          as an array of twelve pointers to strings
 *                          starting  with the locale's equivalent of "Jan".
 *                          E.g., {"Jan", "Feb", "Mar", ...}
 * @param ampmNames         Strings to signify pre-noon
 *                          and post-noon times when using a 12-hour clock
 *                          as an array of two pointers to strings.
 *                          E.g., {"am","pm"}.
 *
 * Strings can be UTF-8 encoded.  If parameters are NULL, default
 * values are used.  The default values for the day, month, and am/pm
 * strings are blank strings.

 * @post The strings are copied by Picsel and can be released after this call.
 *
 * @retval 1 if success, @retval 0 if failure.  Failure means that one of the
 * format strings was invalid.
 */
int PicselLocale_setDateStrings( Picsel_Context *picselContext,
                                 const char *dateLongFormat,
                                 const char *dateFormat,
                                 const char *timeFormat,
                                 const char *dateAndTimeFormat,
                                 const char * const dayNames[7],
                                 const char * const dayNamesAbbr[7],
                                 const char * const monthNames[12],
                                 const char * const monthNamesAbbr[12],
                                 const char * const ampmNames[2]);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_LOCALE_H */
