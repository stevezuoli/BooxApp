/**
 * Portable Preferences configuration module
 *
 * Copyright (C) Picsel, 2008. All Rights Reserved.
 *
 * @author Picsel Technologies Ltd
 *
 * $Id: preferences.c,v 1.19 2009/08/25 14:54:21 neilk Exp $
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "preferences.h"
#include "alien-file.h"
#include "picsel-config.h"
#include "alien-debug.h"

#include "picsel-language.h"
#include "picsel-locale.h"
#include "picsel-encoding.h"

#if defined(DEBUG)
#define PREF_DBUGF(x) printf x
#else /* DEBUG */
#define PREF_DBUGF(x)
#endif /* DEBUG */

/* Bitfield describing which fonts are active
 */
typedef enum
{
    Preferences_Font_Latin_Full    =1,
    Preferences_Font_Latin_Subset  =1<<1,
    Preferences_Font_JaJp          =1<<2,
    Preferences_Font_KoKr          =1<<3,
    Preferences_Font_ZhCn          =1<<4,
    Preferences_Font_ZhTw          =1<<5,
    Preferences_Font_ArAr          =1<<6,
    Preferences_Font_ThTh          =1<<7,
    Preferences_Font_BnIn          =1<<8,
    Preferences_Font_GuIn          =1<<9,
    Preferences_Font_DeVa          =1<<10,
    Preferences_Font_KnIn          =1<<11,
    Preferences_Font_PaIn          =1<<12,
    Preferences_Font_TaIn          =1<<13,
    Preferences_Font_TeIn          =1<<14,
    Preferences_Font_ViVn          =1<<15,
    Preferences_Font_Enable        =1<<16/* If 0 register default languages/fonts
                                          * If 1 register only languages/fonts
                                          * enabled in the preferences
                                          * configuration file
                                          */
}Preferences_Font;

/* Bitfield representing which fonts/languages are registered for
 * if the fileviewer.cfg file is not present
 */
enum
{
    Preferences_Font_Default_Fonts =
          Preferences_Font_Enable | Preferences_Font_Latin_Subset
};


typedef int  (*Preferences_FontEncodingRegisterFp)(Picsel_Context *picselContext);
typedef void (*Preferences_FontLanguageRegisterFp)(Picsel_Context *picselContext);
typedef void (*Preferences_FontRegisterFp)(Picsel_Context *picselContext);

/* Look up table mapping ALIEN_FONT to
 * font/encoding/language
 * registration functions
 */
static struct
{
    const char *const                   fontKeyName;        /* Alien configuration key from Preferences    */
    const Preferences_Font              fontBitField;       /* Bitfield representing fontKeyname           */
    Preferences_FontEncodingRegisterFp  encodingRegisterFp; /* Encoding registration function for language */
    Preferences_FontLanguageRegisterFp  languageRegisterFp; /* Language registration function for language */
    Preferences_FontRegisterFp          fontRegisterFp;     /* Font registration function for language     */
} configFontMappings[] =
{
    /* It is posible to reduce the size of the
     * application binary by removing font support for
     * languages that are not required.
     * To do this enable or disable the appropriate ALIEN_ language
     * defines in the alien make file.
     */
#ifdef ALIEN_EN_GB_FULL
    { "ALIEN_FONT_latin_full"   , Preferences_Font_Latin_Full   ,NULL,                          Picsel_Font_Register_latin,       Picsel_Font_Register_wgl4 },
#endif /* ALIEN_EN_GB_FULL */
#ifdef ALIEN_EN_GB_SUBSET
    { "ALIEN_FONT_latin_subset" , Preferences_Font_Latin_Subset ,NULL,                          Picsel_Font_Register_latin,       NULL                      },
#endif /* ALIEN_EN_GB_SUBSET */
#ifdef ALIEN_JA_JP
    { "ALIEN_FONT_ja-jp"        , Preferences_Font_JaJp         ,Picsel_Encoding_Register_jajp, Picsel_PdfLanguage_Register_jajp, Picsel_Font_Register_jajp },
#endif /* ALIEN_JA_JP */
#ifdef ALIEN_KO_KR
    { "ALIEN_FONT_ko-kr"        , Preferences_Font_KoKr         ,Picsel_Encoding_Register_kokr, Picsel_PdfLanguage_Register_kokr, Picsel_Font_Register_kokr },
#endif /* ALIEN_KO_KR */
#ifdef ALIEN_ZH_CN
    { "ALIEN_FONT_zh-cn"        , Preferences_Font_ZhCn         ,Picsel_Encoding_Register_zhcn, Picsel_PdfLanguage_Register_zhcn, Picsel_Font_Register_zhcn },
#endif /* ALIEN_ZH_CN */
#ifdef ALIEN_ZH_TW
    { "ALIEN_FONT_zh-tw"        , Preferences_Font_ZhTw         ,Picsel_Encoding_Register_zhtw, Picsel_PdfLanguage_Register_zhtw, Picsel_Font_Register_zhtw },
#endif /* ALIEN_ZH_TW */
#ifdef ALIEN_AR_AR
    { "ALIEN_FONT_ar-ar"        , Preferences_Font_ArAr         ,NULL,                          NULL,                             Picsel_Font_Register_arar },
#endif /* ALIEN_AR_AR */
#ifdef ALIEN_TH_TH
    { "ALIEN_FONT_th-th"        , Preferences_Font_ThTh         ,NULL,                          NULL,                             Picsel_Font_Register_thth },
#endif /* ALIEN_TH_TH */
#ifdef ALIEN_BN_IN
    { "ALIEN_FONT_bn-in"        , Preferences_Font_BnIn         ,NULL,                          NULL,                             Picsel_Font_Register_bnin },
#endif /* ALIEN_BN_IN */
#ifdef ALIEN_GU_IN
    { "ALIEN_FONT_gu-in"        , Preferences_Font_GuIn         ,NULL,                          NULL,                             Picsel_Font_Register_guin },
#endif /* ALIEN_GU_IN */
#ifdef ALIEN_DE_VA
    { "ALIEN_FONT_de-va"        , Preferences_Font_DeVa         ,NULL,                          NULL,                             Picsel_Font_Register_deva },
#endif /* ALIEN_DE_VA */
#ifdef ALIEN_KN_IN
    { "ALIEN_FONT_kn-in"        , Preferences_Font_KnIn         ,NULL,                          NULL,                             Picsel_Font_Register_knin },
#endif /* ALIEN_KN_IN */
#ifdef ALIEN_PA_IN
    { "ALIEN_FONT_pa-in"        , Preferences_Font_PaIn         ,NULL,                          NULL,                             Picsel_Font_Register_pain },
#endif /* ALIEN_PA_IN */
#ifdef ALIEN_TA_IN
    { "ALIEN_FONT_ta-in"        , Preferences_Font_TaIn         ,NULL,                          NULL,                             Picsel_Font_Register_tain },
#endif /* ALIEN_TA_IN */
#ifdef ALIEN_TE_IN
    { "ALIEN_FONT_te-in"        , Preferences_Font_TeIn         ,NULL,                          NULL,                             Picsel_Font_Register_tein },
#endif /* ALIEN_TE_IN */
#ifdef ALIEN_VI_VN
    { "ALIEN_FONT_vi-vn"        , Preferences_Font_ViVn         ,NULL,                          NULL,                             Picsel_Font_Register_vivn }
#endif /* ALIEN_VI_VN */
};


/* Property key names prefixed with "ALIEN_"
 * are alien specific.
 */
static const char *alienPrefix = "ALIEN_";

/* Determines if savePrefFile() function also finalises
 * the preference module.
 */
typedef enum
{
    PrefFinalise_No,   /* Save but do not finalise     */
    PrefFinalise_Yes   /* Finalise in addition to save */
}PrefFinalise;

typedef enum
{
    PrefsModified_No,   /* Preferences unmodified         */
    PrefsModified_Yes   /* Preferences have been modified */
}PrefsModified;

/* Preferences context */
struct Preferences
{
    PreferencesNode               *firstPref;       /* linked list of preference nodes */
    char                          *preferencesFile; /* Preferences configuration file  */
    char                          *endOfLineFormat; /* Platform end of file convention */
    Preferences_Font               registerFonts;   /* Defines which fonts are active  */

    PrefsModified                  modified;        /* If PrefsModified_No there are no *
                                                     * changes to preferences since the *
                                                     * last save                        */
    int                            count;           /* The number of items in the list  */
    Preferences_AlienPrefCallBack  callback;        /* The alien preferences callback   */
};

#ifdef DEBUG
/**
 * Debug function: Prints all preference properties
 *
 * This function requires a working implementation of
 * printf()
 *
 * @param[in] prefs       Pointer to preferences context
 *
 * @return                Error
 *                          PrefError_PreferencesUninitialised
 *                          PrefError_UnknownPropertyType
 *                        Success
 *                          PrefError_NoError
 *
 */
PrefError printAllNodes(Preferences *prefs)
{
    PreferencesNode *hd;

    if (prefs == NULL)
        return PrefError_PreferencesUninitialised;
    if (prefs->firstPref == NULL)
        return PrefError_PreferencesUninitialised;

    hd = prefs->firstPref;

    do
    {
        switch (hd->propertyType)
        {
            case PrefKeyType_StringProperty:
                PREF_DBUGF(("Printing String node \n"));
                PREF_DBUGF(("\t propertyKey %s \n", hd->propertyKey));
                PREF_DBUGF(("\t propertyString %s \n", hd->propertyStr));
                break;

            case PrefKeyType_IntProperty:
                PREF_DBUGF(("Printing Int node \n"));
                PREF_DBUGF(("\t propertyKey %s \n", hd->propertyKey));
                PREF_DBUGF(("\t propertyInt %d \n", hd->propertyInt));
                break;

            case PrefKeyType_ColourProperty:
                PREF_DBUGF(("Printing colour node \n"));
                PREF_DBUGF(("\t propertyKey %s \n", hd->propertyKey));
                PREF_DBUGF(("\t propertyColour 0x%08lX \n", hd->propertyColour));

            case PrefKeyType_CommentProperty:
                PREF_DBUGF(("Printing Comment node \n"));
                PREF_DBUGF(("\t comment %s \n", hd->propertyStr));
                break;

            default:
                PREF_DBUGF(("Error: Uknown property type specified\n"));
                return PrefError_UnknownPropertyType;
        }
        hd = hd->next;
    }while (hd !=NULL);

    return PrefError_NoError;
}
#endif /* DEBUG */

/**
 *
 * Reads a line of text from a file into **linePtr.
 * If the buffer is not big enough for a complete line
 * it will be realloced to twice its size.
 *
 * If *linePtr is NULL *pos, *end, *n and *linePtr
 * will be initialised.
 *
 * If *linePtr is not NULL it is the callers responsibility
 * to perform the following initialisation before calling
 * getLine
 *
 *  *pos = 0
 *  *end = 0
 *  *n   = buffersize
 *
 * The caller is responsible for freeing *linePtr
 *
 * @param[in/out] linePtr    Pointer to destination buffer pointer
 * @param[in/out] n          Size of destination buffer in bytes
 * @param[in]     fd         AlienFile_open handle
 * @param[in]     ac         Alien Context
 * @param[in/out] pos        Pointer to the index of the start
 *                           of the buffer in use
 * @param[in/out] end        Pointer to the index of the end
 *                           of the buffer in use
 *
 * @return long              Success
 *                             The number of characters processed
 *                             or 0 for EOF
 *                           Error
 *                             -1 If read fails or out of memory
 */
static long getLine (char          **linePtr,
                     long           *n,
                     int             fd,
                     Alien_Context  *ac,
                     long           *pos,
                     long           *end)
{
    long  read=0;
    long  bytesRead=0;
    long  i;
    char *x;

    char *newBuffer;

    if (*linePtr == NULL)
    {
        *linePtr = (char *)malloc(Line_Buffer_Size);

        if (*linePtr == NULL)
            return -1;

        *n = Line_Buffer_Size;
        *pos = 0;
        *end = 0;
    }

    if (*end == -1)
        return 0; /* The file has been processed */

    /* if position isn't zero then
     * we've read excess into the buffer
     */
    if (*pos != 0 )
    {
        memmove(*linePtr,
                *linePtr + *pos,
                *end-*pos);

        *end -= *pos;
        *pos = 0; /* process from the start again */
    }

    do
    {
        read =  AlienFile_read(ac,
                               fd,
                               (*linePtr)+*end,
                               (*n)-*end-1);

        if (read == -1)
            return -1;

        *end += read;

        bytesRead += read;

        /* check for new line character */
        x = *linePtr;
        for (i=*pos; i<*end; i++)
        {
            if (*x == '\n')
            {
                *x = '\0';

                *pos = i + 1;
                return i + 1; /* return the number  *
                               * of processed chars */
            }
            x++;
        }

        /* If we've get this far with no new line
         * and no more bytes to read this is the
         * final line in the file.
         */
        if (read == 0)
        {
            long result = *end - *pos;
            *(*linePtr + *end) = '\0';

            *end = -1; /* signal we're finished */
            return result;
        }


        /* The line is longer than the current buffer */
        newBuffer = (char *)realloc(*linePtr, (*n)*2 );
        if (newBuffer == NULL)
        {
            *pos = *n;
            return -1;
        }
        else
        {
            *linePtr = newBuffer;
            *n = (*n)*2;
        }
    }while(1);
}


/**
 *
 * Writes a line of text to a file.
 *
 * Automatically appends the end of line string
 * to the file.
 *
 * @param line            Pointer to source buffer
 *                        to file
 * @param fp              AlienFile_open handle
 * @param endOfLineFormat End of line format convention.
 *                        "\n\r" for DOS
 *                        "\n"  for Unix
 * @param ac              Alien Context
 *
 * @return long           The number of bytes written
 */
static long putLine(char *line, int fp,
                    char *endOfLineFormat,
                    Alien_Context *ac)
{
    long written = 0;

    written = AlienFile_write(ac,
                              fp,
                              (void *)line,
                              strlen(line));
    if (written == -1)
        return written;

    if (*endOfLineFormat != '\0')
    {
        written = AlienFile_write(ac,
                                  fp,
                                  endOfLineFormat,
                                  strlen(endOfLineFormat));
    }
    return written;
}

/**
 * Removes '\n' and '\r' from a buffer
 *
 * @param dst             Pointer to destination buffer
 *
 * @return void
 */
static void stripCrLf(char *dst)
{
    char *ln = strchr(dst,'\n');
    char *cr = strchr(dst,'\r');

    if (ln != NULL)
        *ln = '\0';
    if (cr != NULL)
        *cr = '\0';
}

/**
 * Generates two strings split by a delimiter character
 *
 * If the delimiter is not present in will
 * be copied to first and second will be initialised to
 * '/0'
 *
 * @param delimiter       Seperator character
 * @param in              Source buffer
 * @param first           Destination buffer for
 *                        left side of split string
 * @param second          Destination buffer for
 *                        right side of split string
 *
 * @return                void
 *
 * @warning               First and second buffers
 *                        should be at least
 *                        equal in size to the in
 *                        buffer
 */
static void splitStr(char *delimiter, char *in,
                     char *first, char *second)
{
    int i;
    int delimitIdx;
    int length;
    char *split = strstr(in,delimiter);

    if (split == NULL)
    {
        strcpy(first,in);
        strcpy(second,"");
        return;
    }

    length = strlen(in);

    delimitIdx = split - in; /* delimiter index */

    {
        char *a = first;
        char *b = second;
        for (i=0; i< delimitIdx; i++)
        {
            *a++ = *(in+i);
        }
        *a = '\0';

        for (i=(delimitIdx+1); i< length; i++)
        {
            *b++ = *(in+i);
        }
        *b = '\0';
    }
    return;
}

/**
 * Checks if a property value is a string
 *
 * if a value is enclosed in "" it is assumed
 * the property is a string. If so the "" is removed
 *
 * @param propVal         Property value to check
 *
 * @return                0 - if propVal is not a string
 *                        1 - if propVal is a string
 */
static int validateStr(char *propVal)
{
    /* if a value is enclosed in "" then
     * it is assumed to be a string and
     * converted in place
     */
    char *start;
    char *end;

    start = strstr(propVal,"\"");
    if (start == NULL)
        return 0;       /* can't be a string */

    end = strstr(start+1,"\"");
    if (end == NULL)
        return 0;       /* can't be a string */

    end--;
    start++;

    do
    {
        *propVal++ = *start++;
    }while (start <= end);
    *propVal = '\0';

    return 1;
}

/**
 * Appends a preference node to linked list
 *
 * @param prefs           Pointer to preference context
 * @param newPref         Pointer to node to add
 *
 * @return                Error
 *                          PrefError_PreferencesUninitialised
 *                          PrefError_OutOfMemory
 *                        Success
 *                          PrefError_NoError
 *
 */
static PrefError addNode(Preferences *prefs, PreferencesNode *newPref)
{
    PreferencesNode *hd;
    PreferencesNode *node;

    assert(newPref != NULL);

    if (prefs == NULL)
        return PrefError_PreferencesUninitialised;

    node = (PreferencesNode *) malloc(sizeof(PreferencesNode));

    if (node == NULL)
        return PrefError_OutOfMemory;

    hd = prefs->firstPref;

    memcpy(node,newPref,sizeof(PreferencesNode));

    if (prefs->firstPref == NULL)
    {
        prefs->firstPref = node;
    }
    else
    {
        do
        {
            if (hd->next == NULL)
            {
                break;
            }
            else
            {
                hd = hd->next;
            }
        }while(1);

        hd->next = node;
    }
    if (node->propertyType != PrefKeyType_CommentProperty)
        prefs->count++;
    return PrefError_NoError;
}

/**
 *
 * Checks if a given font is enabled
 *
 * @param[in] activeFonts  Bitfield of enabled fonts
 * @param[in] checkActive  Font_Preference bitfield
 *
 * @return    int          1 - font is enabled
 *                         0 - font is not enabled
 */
static int isFontEnabled(Preferences_Font activeFonts,
                         Preferences_Font checkActive)
{
    activeFonts &= checkActive;

    if (activeFonts)
        return 1;
    else
        return 0;
}


/**
 * Parses config file configuration data.
 * Calls callback for Alien preference properties
 *
 * @param prefs           Pointer to preference context
 * @param cfgFileName     Preferences config file name
 * @param ac              Alien Context
 *
 * @return                Error
 *                          PrefError_ConfigFileOpenFailed
 *                          PrefError_OutOfMemory
 *                        Success
 *                          PrefError_NoError
 *
 */
static PrefError parse (Preferences   *prefs,
                        const char    *cfgFileName,
                        Alien_Context *ac)
{
    char *line = NULL;                   /* line read from preference file */
    char  propLine[Line_Buffer_Size];    /* property part including value  */
    char  persist[Line_Buffer_Size];     /* persistence string             */

    long  lineSize  = Line_Buffer_Size;  /* size of line buffer            */
                                         /* including '\0' terminator      */

    long  position=0;                    /* current positiion within       */
                                         /* the line buffer                */
    long  end=0;                         /* end of the line buffer in use  */

    if (cfgFileName != NULL)
    {
        /* options from config file */
        int  inFile;
        long read;

        inFile = AlienFile_open (ac,
                                 cfgFileName,
                                 PicselFileFlagReadOnly);

        if (inFile == PicselInvalidFile)
        {
            PREF_DBUGF(("Error: Preference file cannot be opened\n"));

            return PrefError_ConfigFileOpenFailed;
        }
        do
        {
            char propKey[Line_Buffer_Size];
            char propValue[Line_Buffer_Size];
            PreferencesNode prefNode;
            int isString;

            read = getLine(&line,
                           &lineSize,
                            inFile,
                            ac,
                           &position,
                           &end);

            if (read == -1)
            {
                PREF_DBUGF(("Warning: Preferences line malformed : %s\n",line));
                free(line);
                return PrefError_ConfigFileMalformed;
            }

            if (read != 0)
            {
                /* remove any /r/n */
                stripCrLf(line);

                prefNode.propertyKey        = NULL;
                prefNode.next               = NULL;
                prefNode.persist            = 0;
                prefNode.propertyInt        = 0;
                prefNode.propertyStr        = NULL;
                prefNode.propertyColour     = 0;

                prefNode.initialPropertyStr = NULL;

                if (strstr(line,"#") != NULL) /* retain comment lines */
                {

                    prefNode.propertyStr = strdup(line);
                    if (prefNode.propertyStr == NULL)
                    {
                        return PrefError_OutOfMemory;
                    }

                    prefNode.propertyType = PrefKeyType_CommentProperty;
                    prefNode.persist = 0;
                }
                else
                {
                    splitStr(",",
                             line,
                            &propLine[0],
                            &persist[0]);

                    if (strstr(&persist[0],"persist") != NULL)
                    {
                        prefNode.persist = 1;
                    }

                    splitStr("=",
                             &propLine[0],
                             &propKey[0],
                             &propValue[0]);

                    /* if the "=" delimiter can't be found
                     * we cannot process this line
                     */
                    if (propValue[0] == '\0')
                    {
                        PREF_DBUGF(("Warning: Config line %s malformed. It will be ignored\n",
                                     &propLine[0]));
                        continue;
                    }

                    prefNode.propertyKey = strdup(&propKey[0]);
                    if (prefNode.propertyKey == NULL)
                    {
                        free(prefNode.propertyStr);
                        return PrefError_OutOfMemory;
                    }
                    else
                    {
                        if (strstr(&propKey[0],alienPrefix))
                            prefNode.owner = KeyOwner_Alien;
                        else
                            prefNode.owner = KeyOwner_Picsel;
                    }
                    /* Check for "" around a property and
                     * remove it.
                     */
                    isString = validateStr(&propValue[0]);
                    if (isString)
                    {
                        prefNode.propertyStr = strdup(&propValue[0]);
                        if (prefNode.propertyStr == NULL)
                        {
                            free(prefNode.propertyKey);
                            free(line);
                            return PrefError_OutOfMemory;
                        }
                        prefNode.initialPropertyStr = strdup(&propValue[0]);
                        if (prefNode.initialPropertyStr == NULL)
                        {
                            free(prefNode.propertyKey);
                            free(prefNode.propertyStr);
                            free(line);
                            return PrefError_OutOfMemory;
                        }
                        /* string only */
                        prefNode.propertyType = PrefKeyType_StringProperty;
                    }
                    else
                    {
                        /* If "0X" or "0x" is found the value
                         * is type colour (in hex)
                         */
                        if ((strstr(line,"0X") != NULL) || (strstr(line,"0x") != NULL))
                        {
                            if (sscanf(&propValue[0],"%lX",
                                &prefNode.propertyColour) ==1)
                            {
                                prefNode.propertyType = PrefKeyType_ColourProperty;
                                prefNode.initialPropertyColour = prefNode.propertyColour;
                            }
                            else
                            {
                                continue;
                            }
                        }
                        else
                        {
                            /* int only */
                            if (sscanf(&propValue[0],"%d", &prefNode.propertyInt) ==1)
                            {
                                prefNode.propertyType = PrefKeyType_IntProperty;
                                prefNode.initialPropertyInt = prefNode.propertyInt;
                            }
                            else
                            {
                                continue;
                            }
                        }
                    }
                }
                /* store in list */
                addNode(prefs,&prefNode);
                if (prefNode.owner == KeyOwner_Alien)
                {
                    if (prefs->callback != NULL)
                        prefs->callback(&prefNode);
                }
            }
        }while(read != 0);
        AlienFile_close(ac, inFile);
    }
    free(line);
    return PrefError_NoError;
}


/**
 * Releases memory used by preferences nodes.
 * This is a clean up function
 *
 * @param prefs           Preference context pointer
 *
 * @return                void
 */
static void deleteAllNodes(Preferences *prefs)
{
    PreferencesNode *hd;
    PreferencesNode *current;

    if (prefs == NULL)
        return;
    if (prefs->firstPref == NULL)
        return;

    hd = prefs->firstPref;

    do
    {
        switch (hd->propertyType)
        {
            case PrefKeyType_StringProperty:
                free(hd->propertyKey);
                free(hd->propertyStr);
                free(hd->initialPropertyStr);
                break;

            case PrefKeyType_IntProperty:
            case PrefKeyType_ColourProperty: /* Fall through */
                free(hd->propertyKey);
                break;

            case PrefKeyType_CommentProperty:
                free(hd->propertyStr);
                break;

            default:
                break;
        }
        current = hd;
        hd = hd->next;
        free(current);
    }while (hd !=NULL);

    prefs->firstPref = NULL;
}

/**
 * Searches preference nodes for a matching property key
 *
 * @param prefs           Preference context pointer
 * @param propKeySearch   property key to search for
 *
 * @return                NULL if search failed
 *                        or first matching node found
 *
 */
static PreferencesNode *findPropertyNode(Preferences *prefs,
                                         const char  *propKeySearch)
{
    PreferencesNode *node;

    if (prefs == NULL)
        return NULL;

    if (prefs->firstPref == NULL)
        return NULL;

    node = prefs->firstPref;

    do
    {
        switch (node->propertyType)
        {
            case PrefKeyType_StringProperty:
            case PrefKeyType_IntProperty:
            case PrefKeyType_ColourProperty: /* Fall through */
                if (strcmp(propKeySearch,node->propertyKey) == 0)
                    return node;
                break;

            default:
                break;
        }
        node = node->next;

    }while (node !=NULL);

    return node;
}

/**
 *
 * Saves preference file with optional
 * finalise
 *
 * @param[in/out] prefs  Preference context pointer
 * @param[in]     ac     Alien context
 *
 * @return
 *                       Error
 *                          PrefError_PreferencesUninitialised
 *                          PrefError_ConfigFileWriteFailed
 *
 *                       Success
 *                          PrefError_NoError
 */
static PrefError savePrefFile(Preferences  **prefs,
                              Alien_Context *ac,
                              PrefFinalise   finalise)
{
    PreferencesNode *hd;
    PreferencesNode *current;
    Preferences     *prefContext;

    /* options from config file */
    int  inFile;

    char line[Line_Buffer_Size];
    char numericStr[Line_Buffer_Size];

    int  totalStrLen;

    const int strEqualLen = strlen("=\"\"");
    const int persistLen = strlen(",persist");

    if (*prefs == NULL)
       return PrefError_PreferencesUninitialised;

    prefContext = *prefs;

    if (prefContext->firstPref == NULL)
        return PrefError_PreferencesUninitialised;

    hd = prefContext->firstPref;

    /* Only save to disk if the preferences
     * have changed
     */
    if (prefContext->modified == PrefsModified_No)
    {
        if (finalise == PrefFinalise_Yes)
        {
            deleteAllNodes(prefContext);
            free(*prefs);
            *prefs = NULL;
        }
        return PrefError_NoError;
    }

    inFile = AlienFile_open(ac,
                            prefContext->preferencesFile,
                            PicselFileFlagWriteOnly|
                            PicselFileFlagCreate|
                            PicselFileFlagTruncate);

    if (inFile == PicselInvalidFile)
    {
        if (finalise == PrefFinalise_Yes)
            deleteAllNodes(prefContext);

        return PrefError_ConfigFileWriteFailed;
    }
    else
    {
        do
        {
            switch (hd->propertyType)
            {
                case PrefKeyType_StringProperty:
                    totalStrLen = strlen(hd->propertyKey);
                    if (hd->persist)
                        totalStrLen += strlen(hd->propertyStr);
                    else
                        totalStrLen += strlen(hd->initialPropertyStr);

                    totalStrLen += strEqualLen;
                    totalStrLen += persistLen;

                    if (totalStrLen > (Line_Buffer_Size-1) )
                    {
                        PREF_DBUGF(("Error: Config line for property %s too long",
                                    hd->propertyKey));
                        goto failure;
                    }

                    strcpy(&line[0],hd->propertyKey);

                    /* Surround property string with "" */
                    strcat(&line[0],"=\"");

                    if (hd->persist)
                        strcat(&line[0],hd->propertyStr);
                    else
                        strcat(&line[0],hd->initialPropertyStr);

                    strcat(&line[0],"\"");
                    break;

                case PrefKeyType_IntProperty:
                    if (hd->persist)
                        sprintf(&numericStr[0],"%d",hd->propertyInt);
                    else
                        sprintf(&numericStr[0],"%d",hd->initialPropertyInt);

                    totalStrLen = strlen(hd->propertyKey);
                    totalStrLen++; /* reserve space for "=" */
                    totalStrLen += strlen(&numericStr[0]);
                    totalStrLen += persistLen;

                    if (totalStrLen > (Line_Buffer_Size-1) )
                    {
                        PREF_DBUGF(("Error: Config line for property %s too long",
                                    hd->propertyKey));
                        goto failure;
                    }

                    strcpy(&line[0],hd->propertyKey);
                    strcat(&line[0],"=");
                    strcat(&line[0],&numericStr[0]);
                    break;

                case PrefKeyType_ColourProperty:
                    if (hd->persist)
                        sprintf(&numericStr[0],"0x%08lX",hd->propertyColour);
                    else
                        sprintf(&numericStr[0],"0x%08lX",hd->initialPropertyColour);

                    totalStrLen = strlen(hd->propertyKey);
                    totalStrLen++; /* reserve space for "=" */
                    totalStrLen += strlen(&numericStr[0]);
                    totalStrLen += persistLen;

                    if (totalStrLen > (Line_Buffer_Size-1) )
                    {
                        PREF_DBUGF(("Error: Config line for property %s too long",
                                     hd->propertyKey));
                        goto failure;
                    }

                    strcpy(&line[0],hd->propertyKey);
                    strcat(&line[0],"=");
                    strcat(&line[0],&numericStr[0]);
                    break;

                case PrefKeyType_CommentProperty:
                    strncpy(&line[0],hd->propertyStr,Line_Buffer_Size-1);
                    break;

                default:
                    break;
            }

            if (hd->persist)
                strncat(&line[0],",persist",
                        Line_Buffer_Size-1-strlen(&line[0]));

            /* FIXME: What to do if the write fails? */
            putLine(&line[0],
                     inFile,
                     prefContext->endOfLineFormat,
                     ac);
failure:
            if (finalise == PrefFinalise_Yes)
            {
                free(hd->propertyKey);
                free(hd->propertyStr);
                free(hd->initialPropertyStr);
            }
            current = hd;
            hd = hd->next;

            if (finalise == PrefFinalise_Yes)
                free(current);

      }while (hd !=NULL);
  }

  AlienFile_close(ac, inFile);
  if (finalise == PrefFinalise_Yes)
  {
    free(prefContext->endOfLineFormat);
    free(prefContext->preferencesFile);

    free(*prefs);
    *prefs = NULL;
  }
  return PrefError_NoError;
}

/**
 * returns the next non comment type node in the preferences list
 *
 * @param[in] node        Pointer to preference node
 *
 * @return                Next non comment node pointer or NULL
 *
 */
static PreferencesNode *getNextNonCommentNode(PreferencesNode *node)
{
    if (node == NULL)
        return NULL;

    do
    {
        switch (node->propertyType)
        {
            case PrefKeyType_StringProperty:
            case PrefKeyType_IntProperty:
            case PrefKeyType_ColourProperty:
                return node;

            default:
                break;
        }
        node = node->next;

    }while (node !=NULL);

    return node;
}

/**
 * Walks the list of preferences, either setting the picsel keys (by calling
 * Preferences_setX), or setting the alien keys (by calling the alien callback
 * function).
 *
 * @param[in] pc          Picsel Context
 * @param[in] prefs       Pointer to preferences context
 * @param[in] owner       The key type to set (Alien or Picsel)
 *
 * @return                Error
 *                          PrefError_PreferencesUninitialised
 *                          PrefError_OutOfMemory
 *                          PrefError_BadParameter
 *                        Success
 *                          PrefError_NoError
 */
static PrefError readPrefs(void        *pc,
                           Preferences *prefs,
                           KeyOwner     owner)
{
    PreferencesNode *hd;

    PrefsModified modified;

    if (prefs == NULL)
        return PrefError_PreferencesUninitialised;;

    if (prefs->firstPref == NULL)
    {
        return PrefError_PreferencesUninitialised;
    }

    /* For alien preferences, we need a valid callback function.  For
     * Picsel preferences, we need a valid Picsel_Context pointer */
    if ((owner == KeyOwner_Alien  && prefs->callback == NULL) ||
        (owner == KeyOwner_Picsel && pc == NULL))
    {
        return PrefError_BadParameter;
    }

    modified = prefs->modified;

    hd = prefs->firstPref;

    do
    {
        if (owner == hd->owner)
        {
            if (hd->owner == KeyOwner_Alien)
            {
                prefs->callback(hd);
            }
            else if (hd->owner == KeyOwner_Picsel)
            {
                switch (hd->propertyType)
                {
                    case PrefKeyType_StringProperty:
                        (void)Preferences_setStr(pc,
                                                 prefs,
                                                 hd->propertyKey,
                                                 hd->propertyStr);
                        break;

                    case PrefKeyType_IntProperty:
                        (void)Preferences_setInt(pc,
                                                 prefs,
                                                 hd->propertyKey,
                                                 hd->propertyInt);
                        break;

                    case PrefKeyType_ColourProperty:
                        (void)Preferences_setColour(pc,
                                                    prefs,
                                                    hd->propertyKey,
                                                    hd->propertyColour);

                    case PrefKeyType_CommentProperty: /* FALL THROUGH */
                    default:
                        break;
                }
            }
        }
        hd = hd->next;
    }while (hd !=NULL);

    /* Calling Preferences_Setx() implies preferences
     * have been modified. This is not the case when
     * the file is read for the first time.
     */
    prefs->modified = modified;

    return PrefError_NoError;
}

/**
 *
 * Initialises preference module
 *
 */
PrefError Preferences_init(Preferences  **prefs,
                           char          *cfgFileName,
                           char          *endOfLineFormat,
                           Preferences_AlienPrefCallBack callback,
                           PrefFontError *prefFontInitStatus,
                           Alien_Context *ac)
{
    Preferences *prefContext;
    PrefError    error;

    assert(cfgFileName != NULL);
    assert(endOfLineFormat != NULL);
    assert(callback != NULL);
    assert(prefFontInitStatus != NULL);

    if (prefs == NULL)
        return PrefError_ConfigFileOpenFailed;

    *prefs = (Preferences *)calloc(1,sizeof(Preferences));
    if (*prefs == NULL)
        return PrefError_OutOfMemory;

    prefContext = *prefs;
    prefContext->firstPref = NULL;
    prefContext->preferencesFile = strdup(cfgFileName);
    prefContext->endOfLineFormat = strdup(endOfLineFormat);
    prefContext->modified = PrefsModified_No;
    prefContext->count = 0;
    prefContext->callback = callback;

    prefContext->registerFonts = Preferences_Font_Enable;

    *prefFontInitStatus = PrefFontError_NoError;

    error = parse(prefContext,cfgFileName,ac);

    if (error == PrefError_ConfigFileOpenFailed)
    {
        /* If the preferences configuration file
         * cannot be opened initialisation has failed.
         * Freeing the preferences context allows for
         * font registration defaults.
         */
        free(prefContext->preferencesFile);
        free(prefContext->endOfLineFormat);
        free(*prefs);
        *prefs = NULL;
    }
    return error;
}

/**
 *
 * Finalises preference module
 *
 */
PrefError Preferences_finalise(Preferences  **prefs,
                               Alien_Context *ac)
{

    return savePrefFile(prefs,ac,PrefFinalise_Yes);
}

/**
 *
 * Saves all persistent preference values
 *
 */
PrefError Preferences_saveAll(Preferences  **prefs,
                              Alien_Context *ac)
{
    Preferences *prefContext;

    assert(prefs != NULL);

    if (*prefs == NULL)
        return PrefError_PreferencesUninitialised;

    prefContext = *prefs;
    if (prefContext == NULL)
        return PrefError_PreferencesUninitialised;

    if (prefContext->modified == PrefsModified_Yes)
    {
        if (savePrefFile(prefs,ac,PrefFinalise_No) ==
               PrefError_NoError)
        {
            prefContext->modified = PrefsModified_No;
        }
    }

    return PrefError_NoError;
}

/**
 *
 * Sets a preference integer property
 *
 * This function is a wrapper for PicselConfig_setInt()
 *
 */
int Preferences_setInt(void        *pc,
                       Preferences *prefs,
                       const char  *key,
                       int          value)
{
    PreferencesNode *node;

    int returnCode =
        PicselConfig_setInt(pc,
                            key,value);

    if (prefs == NULL)
        return returnCode;

    node = findPropertyNode(prefs,
                            key);

    if (node != NULL)
    {
        node->propertyInt = value;
        prefs->modified   = PrefsModified_Yes;
    }

    return returnCode;
}

/**
 *
 * Sets a preference string property
 *
 * This function is a wrapper for PicselConfig_setStr()
 *
 */
int Preferences_setStr(void        *pc,
                       Preferences *prefs,
                       const char  *key,
                       const char  *str)
{
    PreferencesNode *node;

    int returnCode =
        PicselConfig_setString(pc,
                               key,str);

    if (prefs == NULL)
        return returnCode;

    node = findPropertyNode(prefs,
                            key);

    if (node != NULL)
    {
        char *newStr = strdup(str);
        free(node->propertyStr);
        node->propertyStr = newStr;
        prefs->modified   = PrefsModified_Yes;
    }

    return returnCode;
}

/**
 *
 * Sets a preference colour property
 *
 * This function is a wrapper for PicselConfig_setInt()
 *
 */
int Preferences_setColour(void         *pc,
                          Preferences  *prefs,
                          const char   *key,
                          unsigned long value)
{
    PreferencesNode *node;

    int returnCode =
        PicselConfig_setInt(pc,
                            key,(int)value);

    if (prefs == NULL)
        return returnCode;

    node = findPropertyNode(prefs,
                            key);

    if (node != NULL)
    {
        node->propertyColour = value;
        prefs->modified   = PrefsModified_Yes;
    }

    return returnCode;
}

/**
 *
 * Retrieves a preference integer property
 *
 */
int* Preferences_getInt(Preferences *prefs,
                        const char  *key)
{
    PreferencesNode *node;

    if (prefs == NULL)
        return NULL;

    if (prefs->firstPref == NULL)
        return NULL;

    node = findPropertyNode(prefs,
                            key);

    if (node != NULL)
    {
        return &(node->propertyInt);
    }
    else
    {
        return NULL;
    }
}

/**
 *
 * Retrieves a preference string property
 *
 */
char* Preferences_getStr(Preferences *prefs,
                         const char  *key)
{
    PreferencesNode *node;

    if (prefs == NULL)
        return NULL;

    if (prefs->firstPref == NULL)
        return NULL;

    node = findPropertyNode(prefs,
                            key);

    if (node != NULL)
    {
        return node->propertyStr;
    }
    else
    {
        return NULL;
    }
}

/**
 *
 * Retrieves a preference colour property
 *
 */
unsigned long* Preferences_getColour(Preferences *prefs,
                                     const char  *key)
{
    PreferencesNode *node;

    if (prefs == NULL)
        return NULL;

    if (prefs->firstPref == NULL)
        return NULL;

    node = findPropertyNode(prefs,
                            key);

    if (node != NULL)
    {
        return &(node->propertyColour);
    }
    else
    {
        return NULL;
    }
}

/**
 * Retrieves the number of preference items in
 * the preferences list. Comment nodes are not included
 * in the total.
 */
PrefError Preferences_getCount(Preferences *prefs, int *count)
{
    if (count == NULL)
        return PrefError_BadParameter;

    if (prefs == NULL)
        return PrefError_PreferencesUninitialised;
    else
        *count = prefs->count;

    return PrefError_NoError;
}

/**
 *
 * Sets all preferences properties loaded by
 * Preferences_init()
 *
 */
PrefError Preferences_read(void        *pc,
                           Preferences *prefs)
{
    return readPrefs(pc, prefs, KeyOwner_Picsel);
}

/**
 *
 * Sets all alien preferences properties loaded by
 * Preferences_init()
 *
 */
PrefError Preferences_readAlienKeys(void        *pc,
                                    Preferences *prefs)
{
    return readPrefs(pc, prefs, KeyOwner_Alien);
}

/**
 * Inserts a preference string property and marks it
 * as persistent. If the property already exists
 * it will be overwritten.
 *
 * This function also sets the PicselConfig_setInt()
 * property
 *
 */
PrefError Preferences_insertInt(void        *pc,
                                Preferences *prefs,
                                const char  *key,
                                int          value,
                                KeyOwner     owner)
{
    PrefError        error = PrefError_NoError;
    PreferencesNode *node;

    if (prefs == NULL)
        return PrefError_PreferencesUninitialised;

    node = findPropertyNode(prefs,
                            key);

    if (node != NULL)
    {
        node->propertyInt = value;
        node->persist = 1;
    }
    else
    {
        PreferencesNode newPref;
        newPref.propertyKey = strdup(key);
        newPref.propertyInt = value;

        newPref.propertyStr = NULL;
        newPref.initialPropertyStr = NULL;
        newPref.initialPropertyInt = value;
        newPref.propertyType = PrefKeyType_IntProperty;
        newPref.persist = 1;

        newPref.owner = owner;
        newPref.next = NULL;

        error = addNode(prefs, &newPref);
    }

    prefs->modified   = PrefsModified_Yes;

    if (pc != NULL)
        (void)PicselConfig_setInt(pc,
                                  key,
                                  value);
    return error;
}



/**
 * Inserts a preference string property and marks it
 * as persistent. If the property already exists
 * it will be overwritten.
 *
 * This function also sets the PicselConfig_setStr()
 * property
 *
 */
PrefError Preferences_insertStr(void        *pc,
                                Preferences *prefs,
                                const char  *key,
                                const char  *value,
                                KeyOwner     owner)
{
    PrefError        error = PrefError_NoError;
    PreferencesNode *node;

    if (prefs == NULL)
        return PrefError_PreferencesUninitialised;

    node  = findPropertyNode(prefs,key);

    if (node != NULL)
    {
        free(node->propertyStr);
        node->propertyStr = strdup(value);
        node->persist = 1;
    }
    else
    {
        PreferencesNode newPref;

        newPref.propertyKey = strdup(key);
        newPref.propertyStr = strdup(value);
        newPref.initialPropertyStr = strdup(value);
        newPref.initialPropertyInt = 0;
        newPref.propertyType = PrefKeyType_StringProperty;
        newPref.persist = 1;

        newPref.owner = owner;
        newPref.next = NULL;

        error = addNode(prefs, &newPref);
    }

    prefs->modified   = PrefsModified_Yes;

    if (pc != NULL)
        (void)PicselConfig_setString(pc,
                                     key,
                                     value);
    return error;
}

/**
 * Inserts a preference colour property and marks it
 * as persistent. If the property already exists
 * it will be overwritten.
 *
 * This function also sets the PicselConfig_setInt()
 * property
 *
 */
PrefError Preferences_insertColour(void       *pc,
                                Preferences   *prefs,
                                const char    *key,
                                unsigned long  value,
                                KeyOwner       owner)
{
    PrefError        error = PrefError_NoError;
    PreferencesNode *node;

    if (prefs == NULL)
        return PrefError_PreferencesUninitialised;

    node = findPropertyNode(prefs,
                            key);

    if (node != NULL)
    {
        node->propertyColour = value;
        node->persist = 1;
    }
    else
    {
        PreferencesNode newPref;
        newPref.propertyKey = strdup(key);
        newPref.propertyColour = value;

        newPref.propertyStr = NULL;
        newPref.initialPropertyStr = NULL;
        newPref.initialPropertyColour = value;
        newPref.propertyType = PrefKeyType_ColourProperty;
        newPref.persist = 1;

        newPref.owner = owner;
        newPref.next = NULL;

        error = addNode(prefs, &newPref);
    }

    prefs->modified   = PrefsModified_Yes;

    if (pc != NULL)
        (void)PicselConfig_setInt(pc,
                                  key,
                                  (int)value);
    return error;
}



/**
 * Initialises preference enumeration returning an opaque reference
 * to the first non comment preferences node.  Node attributes are
 * returned via the output parameters.
 *
 * Further enumeration is performed via Preferences_enumerateNext()
 *
 */
PrefError Preferences_enumerateStart(Preferences    *prefs,
                                     char          **propertyKey,
                                     void          **enumerationCtx,
                                     int           **intProperty,
                                     char          **strProperty,
                                     unsigned long **colourProperty,
                                     PrefKeyType    *keyType)
{
    PreferencesNode *node;

    assert(propertyKey != NULL);
    assert(enumerationCtx != NULL);

    assert(intProperty != NULL);
    assert(strProperty != NULL);
    assert(colourProperty != NULL);
    assert(keyType != NULL);

    if (prefs == NULL)
        return PrefError_PreferencesUninitialised;

    if ((propertyKey == NULL) ||
        (enumerationCtx == NULL) ||
        (intProperty == NULL) ||
        (strProperty == NULL) ||
        (colourProperty == NULL) ||
        (keyType == NULL))
    {
        return PrefError_BadParameter;
    }

    *intProperty    = NULL;
    *strProperty    = NULL;
    *colourProperty = NULL;

    node = prefs->firstPref;

    node = getNextNonCommentNode(node);

    if (node == NULL)
    {
        return PrefError_InvalidNode;
    }
    else
    {
       *enumerationCtx = (void *)node;
       *propertyKey = node->propertyKey;

        switch (node->propertyType)
        {
            case PrefKeyType_StringProperty:
                *strProperty = node->propertyStr;
                *keyType = PrefKeyType_StringProperty;
                break;

            case PrefKeyType_IntProperty:
                *intProperty = &node->propertyInt;
                *keyType = PrefKeyType_IntProperty;
                break;

            case PrefKeyType_ColourProperty:
                *colourProperty = &node->propertyColour;
                *keyType = PrefKeyType_ColourProperty;
                break;

            case PrefKeyType_CommentProperty:
            default:
                return PrefError_InvalidNode;
        }
    }
    return PrefError_NoError;
}

/**
 * Returns the atributes from the next non comment node in the
 * preferences list.
 *
 * Preferences_enumerateStart() should called first
 * to initialise enumeration.
 */
PrefError Preferences_enumerateNext(char          **propertyKey,
                                    void          **enumerationCtx,
                                    int           **intProperty,
                                    char          **strProperty,
                                    unsigned long **colourProperty,
                                    PrefKeyType    *keyType)
{
    PreferencesNode *node;

    assert(propertyKey != NULL);
    assert(enumerationCtx != NULL);

    assert(intProperty != NULL);
    assert(strProperty != NULL);
    assert(colourProperty != NULL);

    assert(keyType != NULL);

    if ((propertyKey == NULL) ||
        (enumerationCtx == NULL) ||
        (intProperty == NULL) ||
        (strProperty == NULL) ||
        (colourProperty == NULL) ||
        (keyType == NULL))
        return PrefError_BadParameter;

    *intProperty    = NULL;
    *strProperty    = NULL;
    *colourProperty = NULL;

    node = (PreferencesNode *)*enumerationCtx;
    node = node->next;

    *propertyKey = NULL;

    node = getNextNonCommentNode(node);

    if (node != NULL)
    {
        *enumerationCtx = (void *)node;
        *propertyKey = node->propertyKey;

        switch (node->propertyType)
        {
            case PrefKeyType_StringProperty:
                *strProperty = node->propertyStr;
                *keyType = PrefKeyType_StringProperty;
                break;

            case PrefKeyType_IntProperty:
                *intProperty = &node->propertyInt;
                *keyType = PrefKeyType_IntProperty;
                break;

            case PrefKeyType_ColourProperty:
                *colourProperty = &node->propertyColour;
                *keyType = PrefKeyType_ColourProperty;
                break;

            case PrefKeyType_CommentProperty:
            default:
                return PrefError_InvalidNode;
        }
    }
    return PrefError_NoError;
}

/**
 * Returns the key ownership of a node by property name
 */
PrefError Preferences_getOwnerType(Preferences *prefs,
                                   char        *propKeySearch,
                                   KeyOwner    *keyOwner)
{
    PreferencesNode *node;

    assert(propKeySearch != NULL);
    assert(keyOwner != NULL);

    if (prefs == NULL)
        return PrefError_PreferencesUninitialised;

    if ((propKeySearch == NULL) || (keyOwner == NULL))
        return PrefError_BadParameter;

    node = findPropertyNode(prefs,
                            propKeySearch);

    if (node != NULL)
    {
        /* The key exists */
        *keyOwner = node->owner;
        return PrefError_NoError;
    }
    else
    {
        /* The key does not exist */
        return PrefError_BadParameter;
    }
}

/**
 *  Returns pointers to node attributes with a given property name
 */
PrefError Preferences_getValueByName(Preferences    *prefs,
                                     char           *propKeySearch,
                                     int            *intProperty,
                                     char          **strProperty,
                                     unsigned long  *colourProperty,
                                     PrefKeyType    *keyType)
{
    PreferencesNode *node;

    assert(propKeySearch != NULL);
    assert(intProperty != NULL);
    assert(keyType != NULL);
    assert(strProperty != NULL);

    if (prefs == NULL)
        return PrefError_PreferencesUninitialised;

    if ((propKeySearch == NULL) ||
        (intProperty == NULL) ||
        (keyType == NULL) ||
        (strProperty == NULL))
    {
        return PrefError_BadParameter;
    }
    node = findPropertyNode(prefs,
                            propKeySearch);

    if (node != NULL)
    {
        switch (node->propertyType)
        {
            case PrefKeyType_StringProperty:
                *strProperty = node->propertyStr;
                *keyType = PrefKeyType_StringProperty;
                break;

            case PrefKeyType_IntProperty:
                *intProperty = node->propertyInt;
                *keyType = PrefKeyType_IntProperty;
                break;

            case PrefKeyType_ColourProperty:
                *colourProperty = node->propertyColour;
                *keyType = PrefKeyType_ColourProperty;
                break;

            default:
                break;
        }
    }
    return PrefError_NoError;
}

/**
 *
 * Sets the Preferences_Font bitfield to 1 for a given
 * font preference keyname if value is not 0.
 *
 */
void Preferences_fontSet(Preferences **prefs,
                         char *key,
                         int value,
                         PrefFontError *fontError)
{
    Preferences *prefContext;
    unsigned int i;

    assert(key!=NULL);

    if (prefs == NULL)
        return;

    prefContext = *prefs;

    if (isFontEnabled(prefContext->registerFonts,Preferences_Font_Enable))
    {
        if (*fontError == PrefFontError_NoError && value != 0)
            *fontError = PrefFontError_FontNotAvailable;

        for (i = 0; i < sizeof(configFontMappings) / sizeof(configFontMappings[0]); i++)
        {
            if (strcmp(key,configFontMappings[i].fontKeyName)==0)
            {
                if (value == 0)
                {
                    prefContext->registerFonts &= ~configFontMappings[i].fontBitField;
                    printf("Preferences unregistering font %s\n",configFontMappings[i].fontKeyName);
                }
                else
                {
                    prefContext->registerFonts |= configFontMappings[i].fontBitField;
                    printf("Preferences registering font %s\n",configFontMappings[i].fontKeyName);
                }

                *fontError =  PrefFontError_NoError;
            }
        }
    }
}

/**
 *
 * Enables either the font/laguages specified in
 * the preferences configuration file or
 * default font/languages
 *
 */
void Preferences_fontRegister(Preferences **prefs, void *pc)
{
    Preferences     *prefContext;
    unsigned int     i;
    Preferences_Font registerFonts;

    prefContext = *prefs;

    if (prefContext == NULL)
    {
        /* Enable the default set of language fonts.
         * Note: Vietnamese cannot be enabled at the
         * same time as latin and is disabled by default.
         */
        printf("Preferences: Enabling default set of fonts\n");
        registerFonts = Preferences_Font_Default_Fonts;
    }
    else
    {
        registerFonts = prefContext->registerFonts;
    }

    for (i = 0; i < sizeof(configFontMappings) / sizeof(configFontMappings[0]); i++)
    {
       if (isFontEnabled(registerFonts,
                         configFontMappings[i].fontBitField))
       {
            printf("Preferences: Registering language %s \n",
                    configFontMappings[i].fontKeyName);
            if  (configFontMappings[i].encodingRegisterFp != NULL)
            {
                configFontMappings[i].encodingRegisterFp(pc);
            }

            if  (configFontMappings[i].languageRegisterFp != NULL)
            {
                configFontMappings[i].languageRegisterFp(pc);
            }

            if  (configFontMappings[i].fontRegisterFp != NULL)
            {
                configFontMappings[i].fontRegisterFp(pc);
            }
       }
    }
}
