/**
 * Portable Prefences configuration module
 *
 * This set of utility functions provide the interface to the
 * portable Preference module.
 *
 * Note that in order to make use of these functions, an alien
 * must have the following capabilities:
 *
 * 1) It must have working copies of certain standard C library
 *    routines:
 *       strstr()
 *       strlen()
 *       strcmp()
 *       strcpy()
 *       strncpy()
 *       strcat()
 *       strncat()
 *       sscanf()
 *       sprintf()
 *       strchr()
 *       malloc()
 *       realloc()
 *       free()
 *       memcpy()
 *
 * 2) It must have working implementations of
 *      AlienFile_open()
 *      AlienFile_close()
 *      AlienFile_read()
 *      AlienFile_write()
 *
 * Copyright (C) Picsel, 2008. All Rights Reserved.
 *
 * $Id: preferences.h,v 1.14 2009/08/25 14:54:21 neilk Exp $
 *
 * @file
 * @author Picsel Technologies Ltd
 *
 */

/**
 * @defgroup Preferences_API Interface for configuration data
 * @{
 */

#ifndef ALIEN_PREFERENCES_H
#define ALIEN_PREFERENCES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "alien-types.h"

/**
 * Opaque type for preferences structure
 */
typedef struct Preferences Preferences;

enum
{
    /* Maximum allowable configuration line length.
     * This should also be big enough to hold a
     * decimal ASCII representation of the
     * largest/smallest 64 bit signed integer.
     */
    Line_Buffer_Size=256
};

/* Enumeration for the different preferences key types */
typedef enum
{
    PrefKeyType_StringProperty,
    PrefKeyType_IntProperty,
    PrefKeyType_ColourProperty,
    PrefKeyType_CommentProperty
}PrefKeyType;


typedef enum
{
    KeyOwner_Alien, /* Alien specific property  */
    KeyOwner_Picsel /* Picsel specific property */
}KeyOwner;

/* Possible Preference error codes */
typedef enum
{
    PrefError_NoError,
    PrefError_OutOfMemory,
    PrefError_UnknownPropertyType,
    PrefError_ConfigFileOpenFailed,
    PrefError_ConfigFileWriteFailed,
    PrefError_ConfigFileMalformed,
    PrefError_BadParameter,
    PrefError_InvalidNode,
    PrefError_PreferencesUninitialised
}PrefError;


/* Preferences_font error codes */
typedef enum
{
    PrefFontError_NoError,
    PrefFontError_FontNotAvailable
}PrefFontError;


/* Preferences node */
typedef struct PreferencesNode
{
    char                   *propertyKey;           /* Property key name or comment     */
    int                     propertyInt;           /* Property integer value           */
    unsigned long           propertyColour;        /* Property to define RGBA colour   */
    char                   *propertyStr;           /* Property string value            */
    char                   *initialPropertyStr;    /* String value initially loaded    */
    int                     initialPropertyInt;    /* Integer value initially loaded   */
    unsigned long           initialPropertyColour; /* Colour value initially loaded    */
    int                     propertyType;          /* String,integer,colour or comment */
    int                     persist;               /* If updated value should be       */
                                                   /* saved on exit                    */
    KeyOwner                owner;                 /* If owned by Alien or Picsel      */
    struct PreferencesNode *next;                  /* Pointer to next Preferences node */
}
PreferencesNode;

/* Alien call back definition */
typedef void (*Preferences_AlienPrefCallBack)(
                               PreferencesNode *prefNode);

/**
 * Initialises preference module
 *
 * If no callback is required callback can be set to NULL
 *
 * @param[in/out] prefs               Pointer to preference context pointer
 * @param[in]     cfgFileName         Preferences config file name
 * @param[in]     endOfLineformat     Platform end of line convention
 *                                    eg "\r\n" for DOS or "\n" for Unix
 * @param[in]     callback            Alien preference callback
 * @param[in/out] prefFontInitStatus  errors returned from language registration
 * @param[in]     ac                  Alien Context
 *
 * @return                        Error
 *                                  PrefError_ConfigFileOpenFailed
 *                                  PrefError_OutOfMemory
 *                                Success
 *                                  PrefError_NoError
 *
 */
PrefError Preferences_init(Preferences  **prefs,
                           char          *cfgFileName,
                           char          *endOfLineFormat,
                           Preferences_AlienPrefCallBack callback,
                           PrefFontError *prefFontInitStatus,
                           Alien_Context *ac);

/**
 * Finalises preference module
 *
 * @param[in/out] prefs   Pointer to preference context pointer
 * @param[in]     ac      Alien Context
 *
 * @return                Error
 *                          PrefError_ConfigFileOpenFailed
 *                          PrefError_OutOfMemory
 *                        Success
 *                          PrefError_NoError - on success
 *
 */
PrefError Preferences_finalise(Preferences  **prefs,
                               Alien_Context *ac);



/**
 * Saves all persistent preference values
 *
 * @param[in] prefs       Pointer to preference context pointer
 * @param[in] ac          Alien Context
 *
 * @return                Error
 *                          PrefError_ConfigFileWriteFailed
 *                          PrefError_PreferencesUninitialised
 *                        Success
 *                          PrefError_NoError
 *
 */
PrefError Preferences_saveAll(Preferences  **prefs,
                              Alien_Context *ac);

/**
 * Sets a preference integer property
 *
 * This function is a wrapper for PicselConfig_setInt()
 *
 * @param[in] pc          Picsel Context
 * @param[in] prefs       Pointer to preferences context
 * @param[in] key         Property key
 * @param[in] value       Value to set
 *
 * @return                Error
 *                          0 if event request fails
 *                        Success
 *                          1
 *
 */
int Preferences_setInt(void        *pc,
                       Preferences *prefs,
                       const char  *key,
                       int          value);

/**
 * Sets a preference string property
 *
 * This function is a wrapper for PicselConfig_setStr()
 *
 * @param[in] pc          Picsel Context
 * @param[in] prefs       Pointer to preferences context
 * @param[in] key         Property key
 * @param[in] str         Value to set
 *
 * @return                Error
 *                          0 if event request fails
 *                        Success
 *                          1
 *
 */
int Preferences_setStr(void        *pc,
                       Preferences *prefs,
                       const char  *key,
                       const char  *str);

/**
 * Sets a preference colour property
 *
 * This function is a wrapper for PicselConfig_setInt()
 *
 * @param[in] pc          Picsel Context
 * @param[in] prefs       Pointer to preferences context
 * @param[in] key         Property key
 * @param[in] value       Value to set
 *
 * @return                Error
 *                          0 if event request fails
 *                        Success
 *                          1
 *
 */
int Preferences_setColour(void         *pc,
                          Preferences  *prefs,
                          const char   *key,
                          unsigned long value);

/**
 * Retrieves a preference integer property
 *
 * @param[in] prefs       Pointer to preferences context
 * @param[in] key         Property key name
 *
 * @return                Error
 *                          NULL if key not set
 *                        Success
 *                          pointer to the retrieved key
 *
 * @warning               Only properties set or loaded by this module
 *                        can be retrieved with Preferences_getStr()
 *                        or Preferences_getInt()
 *
 */
int* Preferences_getInt(Preferences *prefs,
                        const char  *key);

/**
 * Retrieves a preference string property
 *
 * @param[in] prefs       Pointer to preferences context
 * @param[in] key         Property key name
 *
 * @return                Error
 *                          NULL if key not set
 *                        Success
 *                          pointer to the retrieved key
 *
 * @warning               Only properties set or loaded by this module
 *                        can be retrieved with Preferences_getStr()
 *                        or Preferences_getInt()
 *
 */
char* Preferences_getStr(Preferences *prefs,
                         const char  *key);

/**
 * Retrieves a preference colour property
 *
 * @param[in] prefs       Pointer to preferences context
 * @param[in] key         Property key name
 *
 * @return                Error
 *                          NULL if key not set
 *                        Success
 *                          pointer to the retrieved key
 *
 * @warning               Only properties set or loaded by this module
 *                        can be retrieved with Preferences_getColour()
 */
unsigned long* Preferences_getColour(Preferences *prefs,
                                     const char  *key);

/**
 * Retrieves the number opreference items in
 * the preferences list. Comment nodes are not included
 * in the total.
 *
 * @param[in]  prefs       Pointer to preferences context
 * @param[out] count       Pointer to preferences counter
 *
 * @return                 Error
 *                            PrefError_BadParameter
 *                            PrefError_PreferencesUninitialised
 *                         Success
 *                            PrefError_NoError
 *
 */
PrefError Preferences_getCount(Preferences *prefs, int *count);

/**
 * Sets all preferences properties loaded by
 * Preferences_Init()
 *
 * This should be called from AlienConfig_ready()
 *
 * @param[in] pc          Picsel Context
 * @param[in] prefs       Pointer to preferences context
 *
 * @return                Error
 *                          PrefError_PreferencesUninitialised on error
 *                        Success
 *                          PrefError_NoError if successful
 *
 */
PrefError Preferences_read(void          *pc,
                           Preferences   *prefs);

/**
 * Sets all alien preferences properties loaded by
 * Preferences_Init()
 *
 * @param[in] pc          Picsel Context
 * @param[in] prefs       Pointer to preferences context
 *
 * @return                Error
 *                          PrefError_PreferencesUninitialised on error
 *                        Success
 *                          PrefError_NoError if successful
 *
 */
PrefError Preferences_readAlienKeys(void        *pc,
                                    Preferences *prefs);

/**
 * Inserts a preference integer property and marks it
 * as persistent
 *
 * This function also sets the PicselConfig_setInt()
 * property
 *
 * @param[in] pc          Picsel Context
 * @param[in] prefs       Pointer to preferences context
 * @param[in] key         Property key
 * @param[in] value       Value to set
 * @param[in] owner       KeyOwner_Alien or
 *                        KeyOwner_Picsel
 *
 * @return                Error
 *                          PrefError_PreferencesUninitialised
 *                          PrefError_OutOfMemory
 *                        Success
 *                          PrefError_NoError
 *
 */
PrefError Preferences_insertInt(void        *pc,
                                Preferences *prefs,
                                const char  *key,
                                int          value,
                                KeyOwner     owner);



/**
 * Inserts a preference string property and marks it
 * as persistent
 *
 * This function also sets the PicselConfig_setStr()
 * property
 *
 * @param[in] pc          Picsel Context
 * @param[in] prefs       Pointer to preferences context
 * @param[in] key         Property key
 * @param[in] value       Value to set
 * @param[in] owner       KeyOwner_Alien or
 *                        KeyOwner_Picsel
 *
 * @return                Error
 *                          PrefError_PreferencesUninitialised
 *                          PrefError_OutOfMemory
 *                        Success
 *                          PrefError_NoError
 */
PrefError Preferences_insertStr(void        *pc,
                                Preferences *prefs,
                                const char  *key,
                                const char  *value,
                                KeyOwner     owner);


/**
 * Inserts a preference colour property and marks it
 * as persistent
 *
 * This function also sets the PicselConfig_setInt()
 * property
 *
 * @param[in] pc          Picsel Context
 * @param[in] prefs       Pointer to preferences context
 * @param[in] key         Property key
 * @param[in] value       Value to set
 * @param[in] owner       KeyOwner_Alien or
 *                        KeyOwner_Picsel
 *
 * @return                Error
 *                          PrefError_PreferencesUninitialised
 *                          PrefError_OutOfMemory
 *                        Success
 *                          PrefError_NoError
 *
 */
PrefError Preferences_insertColour(void          *pc,
                                   Preferences   *prefs,
                                   const char    *key,
                                   unsigned long  value,
                                   KeyOwner       owner);

/**
 * Initialises preference enumeration returning an opaque reference
 * to the first non comment preferences node.  Node attributes are
 * returned via the output parameters.
 *
 * Further enumeration is performed via Preferences_enumerateNext()
 *
 * @param[in]     prefs          Pointer to preferences context
 * @param[out]    propertyKey    Pointer to propertyKey pointer
 * @param[in/out] enumerationCtx Pointer to enumeration context pointer
 * @param[out]    intProperty    Pointer to integer property value pointer
 * @param[out]    strProperty    Pointer to string property value pointer
 * @param[out]    colourProperty Pointer to colour property value pointer
 * @param[out]    keyType        Pointer to preference key type
 *
 * @return                       Error
 *                                 PrefError_BadParameter
 *                                 PrefError_InvalidNode
 *                                 PrefError_PreferencesUninitialised
 *                               Success
 *                                 PrefError_NoError
 */
PrefError Preferences_enumerateStart(Preferences    *prefs,
                                     char          **propertyKey,
                                     void          **enumerationCtx,
                                     int           **intProperty,
                                     char          **strProperty,
                                     unsigned long **colourProperty,
                                     PrefKeyType    *keyType);

/**
 * Returns the attributes of the next non comment node
 * in the preference list.
 *
 * Preferences_enumerateStart() should called first
 * to initialise the enumeration.
 *
 * @param[out]    propertyKey    Pointer to propertyKey pointer
 * @param[in/out] enumerationCtx Pointer to enumeration context pointer
 * @param[out]    intProperty    Pointer to integer property value pointer
 * @param[out]    strProperty    Pointer to string property value pointer
 * @param[out]    colourProperty Pointer to colour property value pointer
 * @param[out]    keyType        Pointer to preference key type
 *
 * @return                Error
 *                           PrefError_BadParameter
 *                           PrefError_InvalidNode
 *                         Success
 *                           PrefError_NoError
 *
 */
PrefError Preferences_enumerateNext(char          **propertyKey,
                                    void          **enumerationCtx,
                                    int           **intProperty,
                                    char          **strProperty,
                                    unsigned long **colourProperty,
                                    PrefKeyType    *keyType);

/**
 * Returns the key ownership of a node by property name
 *
 * @param[in]  prefs          Pointer to preferences context
 * @param[in]  propKeySearch  Preference property search key
 * @param[out] KeyOwner       Pointer to keyOwner
 *
 * @return                    Error
 *                               PrefError_BadParameter
 *                               PrefError_PreferencesUninitialised
 *                            Success
 *                               PrefError_NoError
 */
PrefError Preferences_getOwnerType(Preferences *prefs,
                                   char        *propKeySearch,
                                   KeyOwner    *keyOwner);

/**
 *
 * Returns pointers to node attributes with a given property name
 *
 * @param[in]  prefs          Pointer to preferences context
 * @param[in]  propKeySearch  Preference property search key
 * @param[out] intProperty    Pointer to integer property value
 * @param[out] strProperty    Preference string property pinter
 * @param[out] colourProperty Preference colour property
 * @param[out] keyType        Pointer to keyType
 *
 * @return                    Error
 *                               PrefError_BadParameter
 *                               PrefError_PreferencesUninitialised
 *                            Success
 *                               PrefError_NoError
 */
PrefError Preferences_getValueByName(Preferences    *prefs,
                                     char           *propKeySearch,
                                     int            *intProperty,
                                     char          **strProperty,
                                     unsigned long  *colourProperty,
                                     PrefKeyType *keyType);

/**
 *
 * Sets the Preferences_Font bitfield to 1 for a given
 * preferences font keyname if value is not 0.
 *
 * @param[in]      prefs       Pointer to preference context pointer
 * @param[in]      key         Alien_FONT key string.
 *                             e.g ALIEN_FONT_latin
 * @param[in]      value
 ~ @param[in/out]  fontError   Pointer to variable which records
 *                             font initialisation status
 *
 * @return         void
 */
void Preferences_fontSet(Preferences **prefs,
                         char *key,
                         int value,
                         PrefFontError *fontError);

/**
 *
 * Enables support for fonts enabled in the Preference
 * configuration file.
 *
 * @param[in]      prefs       Pointer to preference context pointer
 * @param[in]      pc          Picsel Context
 *
 * @return         void
 */
void Preferences_fontRegister(Preferences **prefs, void *pc);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_PREFERENCES_H */
