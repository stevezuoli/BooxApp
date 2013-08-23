/**
 * Configuration options supplied to Picsel from Alien modules.
 *
 * The functions in this file must be implemented by the Alien application
 * before linking with the TGV library.
 *
 * @file
 * $Id: alien-config.h,v 1.35 2009/03/27 13:20:52 jonathanc Exp $
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
/**
 * @defgroup TgvConfigureCore Core Configuration
 * @ingroup TgvCore
 *
 * These functions provide an interface to allow configuration of the Picsel
 * library.
 *
 * Some functions will call into the Alien application, from the Picsel
 * library, at the appropriate stages during initialisation to allow the
 * alien application to configure the Picsel library.
 * The remaining functions allow the Alien application to configure the
 * Picsel library with the aim of tailoring the behaviour to the needs of the
 * Alien application, with respect to:
 * - Available functionality e.g. call Picsel_Agent_Word() from
 *   AlienConfig_initialiseAgents() to enable word document support.
 * - Feature behaviour e.g. set @ref PicselConfig_allowEmptyFiles via
 *   PicselConfig_setInt() from AlienConfig_ready() to enable viewing of
 *   empty files.
 * - Locale settings e.g. call PicselLocale_set() from AlienConfig_ready() to
 *   set the locale.
 * - Picsel library base configuration e.g. set
 *   @ref PicselConfig_settingsPath via PicselConfig_setString() from
 *   AlienConfig_ready() to configure a storage path.
 * - Performance e.g. set @ref PicselConfig_imageCache via
 *   PicselConfig_setInt() from AlienConfig_ready() to configure the size of
 *   the image cache.
 * - Product specific configuration options, described in pages linked from
 *   @ref TgvConfigureCore.
 *
 * Some of these functions are required during initialisation by all Picsel
 * products. Please see @ref TgvInitialisation and @ref TgvDeveloping_App_Flow for
 * further information on the initialisation of the Picsel library.
 *
 * The options are set using PicselConfig_setInt() or
 * PicselConfig_setString().
 * Not all options are valid in all products. Many will be ignored if
 * changed after the Picsel library has been initialised, and so should be
 * called from AlienConfig_ready().
 *
 * See @ref TgvConfigGeneral
 *
 * @{
 */

#ifndef ALIEN_CONFIG_H
#define ALIEN_CONFIG_H

#include "alien-types.h"
#include "picsel-app.h"
#include "picsel-config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Provides an opportunity for the Alien application to configure the
 * behaviour of the Picsel library.
 *
 * Configuration properties can typically be set with calls to
 * PicselConfig_setInt(), while other configuration options are set via
 * functions calls such as PicselApp_setTimeSlice().  Language and character
 * sets should also be configured here
 * e.g. Picsel_PdfLanguage_Register_jajp().
 *
 * Many options will be ignored if changed after the Picsel library has been
 * initialised, and so should be called from here, see the documentation for
 * the individual option for further information.
 *
 * Initialisation of document agents should not occur inside this function,
 * they should reside inside either AlienConfig_initialiseAgents() or
 * AlienPhotoLab_registerOptionalModules(). See @ref TgvDeveloping_App_Flow for
 * more information.
 *
 * This function is called by the Picsel library shortly after
 * PicselApp_start() is called. See @ref TgvInitialisation and
 * @ref TgvDeveloping_App_Flow.
 *
 * @param[in] alienContext  The @ref Alien_Context which was provided by the
 *                          Alien application when it called
 *                          PicselApp_start().
 *
 * @product Required by all Picsel products.
 * @see     PicselConfig_setInt(), @ref TgvInitialisation,
 *          @ref TgvDeveloping_App_Flow
 */
void AlienConfig_ready(Alien_Context *alienContext);


/**
 * Indicates the type of key input events expected by PicselApp_keyPress()
 * and PicselApp_keyRelease().
 *
 * This is called by the Picsel library to notify the Alien application of a
 * change in the expected type of the key input event. @ref PicselInputType
 * is an enum of the different key input event types, which indicate the
 * expected range of values of type @ref PicselKey.
 *
 * The default input type is @ref PicselInputType_Command.
 *
 * This function will be called when the Picsel library requires a change in
 * the input event type, e.g. if a Picsel product requires an input type of
 * @ref PicselInputType_Key, then this function will be called to configure
 * the input type. If multiple products share the same Alien application
 * (i.e. when another Picsel product is embedded into another, see
 * PicselApp_addEmbedded()), then it may be required to switch
 * between the different input types.
 *
 * @param[in] alienContext  The @ref Alien_Context which was provided by the
 *                          Alien application when it called
 *                          PicselApp_start().
 * @param[in] type          Type of input key event code
 *                          @ref PicselInputType.
 *
 * @post      PicselApp_keyPress() and PicselApp_keyRelease() will
 *            only expect key input events of the specified type.
 *
 * @product This change in event type will only potentially occur for CUI
 *          user interface products and applications.
 * @see     PicselApp_keyPress(), PicselApp_keyRelease(),
 *          @ref PicselKey, @ref PicselInputType
 **/
void AlienConfig_setInputType(Alien_Context   *alienContext,
                              PicselInputType  type);


/**
 * Provides an opportunity for the Alien application to initialise document
 * agents.
 *
 * The Alien application should initialise any document agents, such as
 * Picsel_Agent_Word(), as required for this product. Further information
 * available at @ref TgvInitialiseAgents.
 * See @ref TgvLink_Time_Configuration for information on the implications of
 * enabling features that are either unused or not required.
 *
 * This is used by all Picsel products that include a document viewing
 * capability. However, PLDK has a different architecture
 * for loading images incrementally, using
 * AlienPhotoLab_registerOptionalModules() at the same stage during
 * @ref TgvInitialisation instead.
 *
 * This function is called by the Picsel library shortly after
 * PicselApp_start() is called. See @ref TgvInitialisation and
 * @ref TgvDeveloping_App_Flow.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] alienContext  The @ref Alien_Context which was provided by the
 *                          Alien application when it called
 *                          PicselApp_start().
 *
 * @retval 1 If the function completed normally.
 * @retval 0 If the function failed. Behaviour of the Picsel library
 *           when it receives this is undefined.
 *
 * @product Required by all Picsel products (see @ref TgvProducts) except
 *          Photo Lab and Power Platform.
 * @see     @ref TgvInitialiseAgents, @ref TgvDeveloping_App_Flow,
 *          @ref TgvInitialisation
 */
int AlienConfig_initialiseAgents(Picsel_Context *picselContext,
                                 Alien_Context  *alienContext);


/**
 * Loads a configuration from a file.
 *
 * The function will allow configuration options to be altered, via the
 * configuration file, without having to rebuild the Alien application. This
 * configuration file will only be used for loading configuration options,
 * configuration options configure within an application will not be saved
 * out to this file.
 * The choices defined by PicselConfig_loadChoicesFile() are for
 * configuration of the TGV environment, exactly as if they had been set
 * one-by-one using PicselConfig_setInt() and PicselConfig_setString(). The
 * properties saved after AlienConfig_getPropertiesPath() has been called are
 * typically preferences and state for a CUI product, and are opaque to the
 * Alien application.
 *
 * This function will result in a file being loaded at every Picsel library
 * initialisation, this may affect Picsel libarary initialisation times.
 *
 * This function must only be called from inside AlienConfig_ready().
 * It is not intended for this function to be used in a products final build.
 *
 * The configuration file must contain lines of key=value pairs, with each
 * line terminated by a Unix line feed character. Each key must be a
 * configuration parameter string which is valid for PicselConfig_setInt()
 * or PicselConfig_setString(), such as @ref PicselConfig_focusedFillColour.
 * Values may not extend over multiple lines. The file must begin with a line
 * containing only @c [default]
 *
 * For example:
 *
 * @code
 * # choices file to change zoom amount to 110%
 * [default]
 * FV_zoomInAmount=110
 * FV_zoomInRepeatAmount=110
 * FV_zoomOutAmount=90
 * FV_zoomOutRepeatAmount=90
 * @endcode
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] choicesFile   Path to file containing the configuration
 *                          settings. May not be null.
 *
 * @retval 1 If function completed successfully.
 * @retval 0 If function failed to complete successfully.
 *
 * @product Optional for all Picsel products.
 */
int PicselConfig_loadChoicesFile(Picsel_Context *picselContext,
                                 char           *choicesFile);


/**
 * Requests a path where a persistent properties file can be stored.
 *
 * The path given by this function should point to a writeable directory where
 * the Picsel library may store a persistent properties file.  The persistent
 * properties stored may be product specific, and will allow persistence of
 * state / configuration between successive application executions.
 *
 * The properties saved after AlienConfig_getPropertiesPath() has been called
 * are typically preferences and state for a CUI product, and are opaque to
 * the Alien application. The choices defined by
 * PicselConfig_loadChoicesFile() are for configuration of the TGV
 * environment, exactly as if they had been set one-by-one using
 * PicselConfig_setInt() and PicselConfig_setString().
 *
 * If a valid persistent properties path is given, then the persistent data
 * file will be written out when the application shuts down and, depending on
 * the Picsel product, may be updated at other times during runtime. The
 * persistent data file will be read in at application initialisation. If a
 * valid persistent properties path is not provided, or this function is not
 * called then there will be no persistent properties file created or loaded.
 *
 * Please consult you Picsel product integration guide for guidance on
 * whether the persistent property path is required, but for most products
 * this will be optional.
 *
 * A valid non-NULL @p path will result in a file being loaded every time the
 * Picsel library is initialised and finalised, this may affect Picsel
 * library initialisation and finalise times.
 *
 * The string provided by the @p path argument should not be freed until the
 * Picsel library calls AlienConfig_getPropertiesPathDone().
 *
 * @param[in] alienContext  The @ref Alien_Context which was provided by the
 *                          Alien application when it called
 *                          PicselApp_start().
 * @param[out] path         Where to store the path as a UTF-8 encoded
 *                          string.  Can be NULL if persistent data is not
 *                          required.  This string will allocated by the
 *                          Alien application.  The string must not be freed
 *                          until AlienConfig_getPropertiesPathDone() is
 *                          called.
 *
 * @retval 0 If function completed successfully.
 * @retval 1 If function failed to complete successfully or if the Alien
 *           application does not require properties to be persistent.
 *
 * @post    The allocated string @p path must remain valid until
 *          AlienConfig_getPropertiesPathDone() is called.
 * @product Typically only CUI user interface products and applications
 *          may require a persistent properties file.
 * @see     AlienConfig_getPropertiesPathDone()
 */
int AlienConfig_getPropertiesPath(Alien_Context* alienContext, char **path);


/**
 * Indicates when the properties path string is no longer required.
 *
 * Called when the properties @p path string passed back from
 * AlienConfig_getPropertiesPath() is no longer required, and any memory
 * allocated for it may be released.
 *
 * @param[in] alienContext  The @ref Alien_Context which was provided by the
 *                          Alien application when it called
 *                          PicselApp_start().
 * @param[in] path          The path string passed back from the previous
 *                          call to AlienConfig_getPropertiesPath().
 *
 * @pre     AlienConfig_getPropertiesPath() should have been called and the
 *          memory allocated for the string @p path passed back from that
 *          function should still be allocated.
 * @product Typically only CUI user interface products and applications
 *          require a persistent properties file.
 * @see AlienConfig_getPropertiesPath()
 */
void AlienConfig_getPropertiesPathDone(Alien_Context *alienContext,
                                       char          *path);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_CONFIG_H */
