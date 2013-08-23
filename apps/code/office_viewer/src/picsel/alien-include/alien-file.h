/**
 * Persistent file access
 *
 * The functions in this file must be implemented by the Alien application
 * before linking with the TGV library. Many of them are optional; for
 * features which are not required by the application, stub functions may
 * be provided which compile but simply return errors.
 *
 * @file
 * $Id: alien-file.h,v 1.49 2009/08/05 16:26:55 neilk Exp $
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @defgroup TgvFiles File Access
 * @ingroup TgvSystem
 *
 * Filing System access, allowing the Picsel library to read and write
 * persistent files from device storage.
 *
 * @anchor TgvFileImplementationCoverage
 * @par Implementation Coverage
 *
 * When building your first application using the Picsel library, it may be
 * best to initially implement the Picsel File Viewer.
 * You can implementing only those functions required by the File Viewer. The
 * other functions must be provided as stubs.  This will familiarize you
 * with the build process, the threading models and the architecture of
 * TGV.  See @ref TgvDeveloping_an_Application.
 * Once the File Viewer is working, the other functions required for
 * the product you intend to build can be implemented.
 * See @ref TgvDeveloping_an_Application.
 *
 * The following AlienFile* functions should be implemented for File
 * Viewer:
 *
 * - AlienFile_close()
 * - AlienFile_finalise()
 * - AlienFile_getFileInfo() but initially only the size field need be set
 * - AlienFile_initialise()
 * - AlienFile_open()
 * - AlienFile_read()
 * - AlienFile_seek()
 *
 * The remaining AlienFile* functions are required for some products,
 * but for default configuration of Picsel File Viewer it is safe to provide
 * stubs which return the values shown in the table below.
 *
 *  - AlienFile_copy(),                return -1
 *  - AlienFile_delete(),              return -1
 *  - AlienFile_exists(),              return -1
 *  - AlienFile_findAgain(),           return  @ref PicselFile_findImpossible
 *  - AlienFile_findAllDone(),         (no  return value)
 *  - AlienFile_findDone(),            (no  return value)
 *  - AlienFile_findFirst(),           return   @ref PicselFile_findImpossible
 *  - AlienFile_findNext(),            return   @ref PicselFile_findImpossible
 *  - AlienFile_flush(),               return 0
 *  - AlienFile_getFileInfoByPath(),   return -1
 *  - AlienFile_getLastEnumInfo(),     return @ref PicselFile_findImpossible
 *  - AlienFile_getMetaData(),         return @ref AlienFileMetaResult_NotSupported
 *  - AlienFile_getMetaDataByPath(),   return @ref AlienFileMetaResult_NotSupported
 *  - AlienFile_getLastEnumMetaData(), return @ref AlienFileMetaResult_NotSupported
 *  - AlienFile_getMimeType(),         (no return value)
 *  - AlienFile_mkdir(),               return -1
 *  - AlienFile_rename(),              return -1
 *  - AlienFile_truncate(),            return -1
 *  - AlienFile_write(),               return -1
 *
 * Some Picsel products need to know when files or folders have changed.
 * For these you must implement the functions described
 * in @ref TgvFileSystemChanges.
 *
 * @section AlienFile_PathNames Path Names
 *
 * Any pathname passed to the Picsel library must be supplied as a UTF-8 string.
 * Use the slash character, '/', to separate directories from each other
 * and from the filename.
 *
 * Absolute paths must begin with a leading '/'.
 * There will be no trailing '/' in pathnames passed to or from the
 * Alien application, except for the (ePAGE) root directory, which has the path "/".
 *
 * The Alien application is responsible for translating paths understood
 * by the Picsel library to those understood by the platform OS.
 *
 * The Alien application is responsible for any required character encoding
 * conversions.
 *
 * @section TgvEnumeratingFolders Enumerating the Contents of Folders
 *
 * Some Picsel products need to enumerate the contents of folders.  To
 * allow this you must implement the following functions
 *
 * - AlienFile_findFirst()
 * - AlienFile_findNext()
 * - AlienFile_getLastEnumInfo()
 * - AlienFile_findAgain()
 * - AlienFile_findDone()
 * - AlienFile_findAllDone()
 * - AlienFile_getLastEnumInfo()
 *
 *
 * First, the Picsel library will call AlienFile_findFirst() with the path
 * of the folder to enumerate.  Then, it will keep calling AlienFile_findNext()
 * until @ref PicselFile_findAllDone is returned. After the Picsel library
 * has finished with each entry, it will call AlienFile_findDone().
 * Finally, it will call AlienFile_findAllDone().
 *
 * @dotfile alien-file-find.dot
 *
 * Below is a typical sequence of calls to enumerate a folder containing
 * one file, called @c file.txt, and one folder, called @c folder.
 *
 * @dotfile alien-file-find.msc
 */

#ifndef ALIEN_FILE_H
#define ALIEN_FILE_H

#include "alien-types.h"
#include "picsel-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup TgvVirtualFiles Supporting Concurrent File Access
 * @ingroup TgvFiles
 *
 *
 * By default, the Picsel library expects that the Alien application will
 * support having an unlimited number of file descriptors open simultaneously.
 *
 * If the platform OS is unable to support this, the Picsel library is able
 * to arrange its file access such that only one file is ever open at a time.
 *
 * To open a file the Picsel library will call AlienFile_open() with
 * @ref PicselFileFlagVirtual set in @p flags.  If the platform OS can only
 * have one file open at a time, AlienFile_open() must return
 * @ref PicselVirtualFile, without actually opening the file.
 *
 * When the Picsel library actually needs to access the file, it will
 * call AlienFile_open(), without @ref PicselFileFlagVirtual set.  This time the
 * implementation should open the file and return the genuine file
 * descriptor.  The Picsel library will use this in calls to AlienFile_read(),
 * AlienFile_write() etc. and will call AlienFile_close() to close the file
 * before requesting another genuine file descriptor.
 *
 * The diagram below shows a typical sequence of calls when the Picsel
 * library is opening two files, one for reading and the other for writing.
 *
 *  @dotfile alien-file-virtual-files.msc
 *
 * @warning There is a performance hit from using virtual file descriptors.
 */

/**
 * @defgroup TgvDrmFiles DRM File Handling
 * @ingroup TgvFiles
 *
 * To handle files controlled by Digital Rights Management(DRM),
 * special flags are used.
 *
 * The Picsel library will pass a bitwise or of the relevant @c PicselFileFlagDrm*
 * flags as the @c flags parameter of AlienFile_open() to inform the Alien
 * Application how the file being opened will be used.  The Alien Application
 * must use the platform's native DRM checking facilities to verify that
 * the Picsel library has the appropriate DRM license, or to consume such a
 * license if one is available.  If no such license is available
 * AlienFile_open() must return @ref PicselDrmFile.
 *
 */

/** @defgroup TgvFileFlags File Opening Flags.
 *  @ingroup TgvFiles
 *
 *  Flags for use in AlienFile_open().
 *
 *
 */

/**
 * @ingroup TgvFileFlags
 * Create the file if it does not exist.
 *
 * If this flag is set, and the file being opened does not exist, then it
 * should be created. Flag has no effect if the file does exist.
 *
 * This flag may be set together with @ref PicselFileFlagTruncate.
 */
#define PicselFileFlagCreate    (1<<0)

/**
 * @ingroup TgvFileFlags
 * Open the file read-only.
 *
 * If this flag is set, then the file should be opened in read only mode.
 *
 * An error should be returned from AlienFile_open() if the file does not
 * exist (assuming neither PicselFileFlagCreate or PicselFileFlagTruncate are
 * set).
 *
 * If neither PicselFileFlagReadOnly nor @ref PicselFileFlagWriteOnly are set, then
 * the file should be opened in combined read/write mode.
 *
 * Picsel will never set both PicselFileFlagReadOnly and
 * PicselFileFlagWriteOnly together.
 */
#define PicselFileFlagReadOnly  (1<<1)

/**
 * @ingroup TgvFileFlags
 * Truncate the file to zero bytes.
 *
 * If this flag is set, and the file already exists, it should be truncated to
 * zero bytes long. If the file doesn't exist, and @ref PicselFileFlagCreate
 * is set, it should be created.
 *
 * This flag may be set together with @ref PicselFileFlagCreate.
 */
#define PicselFileFlagTruncate  (1<<2)

/**
 * @ingroup TgvFileFlags
 * Open the file write-only.
 *
 * If this flag is set, then the file should be opened in write only mode.
 *
 * An error should be returned from AlienFile_open() if the file does not
 * exist (assuming neither PicselFileFlagCreate or PicselFileFlagTruncate are
 * set).
 *
 * If neither PicselFileFlagReadOnly or PicselFileFlagWriteOnly are set, then
 * the file should be opened in combined read/write mode.
 *
 * Picsel will never set both PicselFileFlagReadOnly and
 * PicselFileFlagWriteOnly together.
 */
#define PicselFileFlagWriteOnly (1<<3)


/**
 * @ingroup TgvFileFlags
 * A virtual file will be used, see @ref TgvVirtualFiles.
 */
#define PicselFileFlagVirtual   (1<<4)


/** The contents of the file, including audio and video, will be played.
 *  @ingroup Tgv TgvDrmFiles
 */
#define PicselFileFlagDrmPlay       (1<<8)

/** The contents of the file will be displayed on-screen.
 *  @ingroup TgvDrmFiles
 */
#define PicselFileFlagDrmDisplay    (1<<9)

/** The file will be executed.
 *  @ingroup TgvDrmFiles
 */
#define PicselFileFlagDrmExecute    (1<<10)

/** The file will be printed.
 *  @ingroup TgvDrmFiles
 */
#define PicselFileFlagDrmPrint      (1<<11)

/** The contents of the file will be forwarded to another person.
 *  @ingroup TgvDrmFiles
 */
#define PicselFileFlagDrmForward    (1<<12)


/**
 * @addtogroup TgvFiles
 * @{
 */


/**
 * Origins for AlienFile_seek() */
typedef enum PicselFileSeekOrigin
{
   /** From start   */
    PicselFileSeekSet = 65539,

    /** From current */
    PicselFileSeekCur,

    /** From end     */
    PicselFileSeekEnd
}
PicselFileSeekOrigin;

/** File size                                                             */
typedef struct PicselFileSize
{
    unsigned int low;                   /**< Low part                     */
    unsigned int high;                  /**< High part                    */
}
PicselFileSize;

/** Offsets within a file, for use in AlienFile_seek()                    */
typedef struct PicselFileOffset
{
    unsigned int low;                   /**< Low part                     */
    int high;                           /**< High part                    */
}
PicselFileOffset;

/**
 * @}
 */ /* End addtogroup TgvFiles */


/** Illegal file handle value, returned by AlienFile_open().
  * @ingroup TgvFileFlags */
#define PicselInvalidFile (-1)


/** Virtual file handle value, returned by AlienFile_open()
  * when the platform OS can only support opening one file
  * at a time. @ingroup TgvVirtualFiles */
#define PicselVirtualFile (-2)

/** @ingroup TgvDrmFiles
  * Return %code from AlienFile_open() when DRM rights are unavailable.  */
#define PicselDrmFile     (-3)

/**
 *  @addtogroup TgvFiles
 *  @{
 */

/** Return codes from AlienFile_findFirst(), AlienFile_findNext(),
    AlienFile_findAgain()   */
typedef enum PicselFile_findResult
{
    PicselFile_findOK = 65539,            /**< Something found            */
    PicselFile_findNotADirectory,         /**< No directory to find in    */
    PicselFile_findAllDone,               /**< Nothing left to find       */
    PicselFile_findImpossible             /**< Insufficient resources     */
}
PicselFile_findResult;

/** Bitmasks in AlienFile_getLastEnumInfo()                               */
typedef enum AlienFileOp
{
    AlienFileOp_Size            = (1 << 0),
    AlienFileOp_ModificationDate= (1 << 1),
    AlienFileOp_AccessDate      = (1 << 2),
    AlienFileOp_CreationDate    = (1 << 3),
    AlienFileOp_Attrib          = (1 << 4),
    AlienFileOp_All = AlienFileOp_Size | AlienFileOp_ModificationDate |
                      AlienFileOp_AccessDate | AlienFileOp_CreationDate |
                      AlienFileOp_Attrib,
    AlienFileOp_ForceSize       = (1 << 16) /**< Force the enum to be
                                                 contained in a 32 bit int */
}
AlienFileOp;

/**
 * Structure filled in by AlienFile_getLastEnumInfo(), AlienFile_getFileInfo()
 * and AlienFile_getFileInfoByPath().
 *
 * All dates contain the elapsed time in seconds since
 * January 1, 1970 00:00:00 GMT.
 *
 * Any dates not supported by the platform should be set to 0 by
 * the Alien application.
 *
 *
 */
typedef struct AlienFileInfo
{
    PicselFileSize size;            /**< file size                         */
    unsigned long  modificationDate;/**< modification time of the file     */
    unsigned long  accessDate;      /**< time file was accessed only
                                         for reading                       */
    unsigned long  creationDate;    /**< creation time of the file         */
    unsigned long  attrib;          /**< bitmask of file attributes,
                                         see @ref AlienFileAttrib   */
}
AlienFileInfo;

/** Possible item flags                                                    */
typedef enum AlienFileAttrib
{
    AlienFileAttrib_Hidden    = (1 << 0),
    AlienFileAttrib_Dir       = (1 << 1),
    AlienFileAttrib_ReadOnly  = (1 << 2),
    AlienFileAttrib_System    = (1 << 3),
    AlienFileAttrib_ForceSize = (1 << 16)  /**< Force the enum to be
                                                contained in a 32 bit int */
}
AlienFileAttrib;

/** AlienFileGpsInfo values                                               */
typedef enum PicselFileGpsInfo
{
    PicselFileGpsInfo_DoesNotExist      = 0,
    PicselFileGpsInfo_IsEditable        = 1,
    PicselFileGpsInfo_IsNotEditable     = 2,
    PicselFileGpsInfo_ForceSize         = (1 << 16)  /**< Force the enum
                                                          to be contained
                                                          in a 32 bit int */
}
PicselFileGpsInfo;

/**
 * File meta-data operations, passed to AlienFile_getMetaDataByPath()
 * in @ref AlienFileMetaInfo.op
 *
 */
typedef enum AlienFileMetaOp
{
    /**
     * A name to be used for display instead of a filename.
     * Returns a UTF-8 string. <HR>
     */
    AlienFileMetaOp_DisplayName       = 0x000,

    /**
     * The native filename, where this does not match
     * the filename specified (for example symbolic links).
     * Returns a UTF-8 string. <HR>
     */
    AlienFileMetaOp_NativeFilename    = 0x001,

    /**
     * A string indicating the copyright owner.
     * Returns a UTF-8 string.   <HR>
     */
    AlienFileMetaOp_CopyrightOwner    = 0x002,

    /**
     * Whether the file is permitted to be attached to an email.
     * Returns an unsigned long, with value 1 if file can be attached,
     * or 0 if file cannot be attached.    <HR>
     */
    AlienFileMetaOp_PermitEmailAttach = 0x003,

    /*---------------------------------------------------------------*/

    /**
     * The time at which the DRM restrictions will prevent
     * playing of the file.
     * Returns the time as an unsigned long representing the number
     * of seconds since January 1, 1970 00:00:00 GMT.   <HR>
     */
    AlienFileMetaOp_DrmPlayExpiryTime = 0x100,

    /**
     * The number of seconds left for playing a file due to the DRM
     * restrictions of the file. May not match with the DrmPlayExpiryTime.
     * Returns a number of seconds as an unsigned long.   <HR>
     */
    AlienFileMetaOp_DrmPlayTimeLeft   = 0x101,

    /**
     * The number of plays remaining before the DRM restrictions prevent
     * playing of the file.
     * Returns a number of plays as an unsigned long.  <HR>
     */
    AlienFileMetaOp_DrmPlayCountLimit = 0x102,

    /**
     * Whether the data contained within the file is protected from being
     * copied.
     * Returns an unsigned long, with value 0 if file can be copied,
     * or 1 if copy should not be allowed.  <HR>
     */
    AlienFileMetaOp_DrmCopyProtect    = 0x103,

    /**
     * Whether the data contained within the file is allowed to be
     * forwarded.
     * Returns an unsigned long, with value 0 if file can be forwarded,
     * or 1 if forwarding should not be allowed. <HR>
     */
    AlienFileMetaOp_DrmForward        = 0x104,

    /**
     * Whether the data contained within the file is protected from being
     * copied.
     * Returns an unsigned long, with value 0 if file can be displayed,
     * or 1 if display should not be allowed. <HR>
     */
    AlienFileMetaOp_DrmDisplay        = 0x105,

    /*---------------------------------------------------------------*/

    /**
     * Whether the data contains GPS information in the file or not.
     * Returns an unsigned long, as defined in @ref PicselFileGpsInfo. <HR>
     */
    AlienFileMetaOp_GpsInformation    = 0x200,

    /*---------------------------------------------------------------*/

    /**
     * The play time for a movie file, in seconds.
     * Returns the number of seconds as an unsigned long. <HR>
     */
    AlienFileMetaOp_PlayTime          = 0x300,

    /*---------------------------------------------------------------*/

    /**
     * Whether the file is protected, requiring additional user prompts before
     * performing edits or deletes.
     * Returns an unsigned long, with value 1 if file is protected, and 0 if
     * the file is not protected. <HR>
     */
    AlienFileMetaOp_Protected        = 0x400,

    /**
     * Force the enum to be contained in a 32 bit int.
     */
    AlienFileMetaOp_ForceSize         = (1 << 16)
}
AlienFileMetaOp;

/**
 * Return codes from AlienFile_getMetaDataByPath() and
 * AlienFile_getMetaData() and in @ref AlienFileMetaInfo.result
 * for each individual meta-data request.
 */
typedef enum AlienFileMetaResult
{
    /**
     * Function: Success.
     * Struct:   Success.   <HR>
     */
    AlienFileMetaResult_Success      = 0,

    /**
     * Function: Failure to fetch any meta-data.
     * Struct:   Failure fetching this meta-data. (The function
     *           would have returned success.)  <HR>
     */
    AlienFileMetaResult_Failure      = 1,

    /**
     * Function: Fetching any meta-data is not supported.
     * Struct:   Fetching this meta-data is not supported.  <HR>
     */
    AlienFileMetaResult_NotSupported = 2,

    /**
     * Struct:   This meta-data has no value to be returned.  <HR>
     */
    AlienFileMetaResult_NoValue      = 3,

    /**
     * Dummy value to force the enum to use 32 bits.
     */
    AlienFileMetaResult_ForceSize    = (1 << 16)
}
AlienFileMetaResult;

/**
 * A structure used to request multiple meta-data items from the
 * alien, in the functions AlienFile_getMetaData() and
 * AlienFile_getMetaDataByPath().
 */
typedef struct AlienFileMetaInfo
{
    AlienFileMetaOp     op;        /**< The operation to perform. <HR>*/

    AlienFileMetaResult result;    /**< One of the AlienFileMetaResult
                                    *   codes indicating the success of
                                    *   the operation or the reason for
                                    *   failure. <HR>*/

    void               *buffer;    /**< Buffer provided by the alien
                                    *   containing the data requested.<HR> */

    int                 bufferLen; /**< Length of buffer provided by the
                                    *   alien. */
}
AlienFileMetaInfo;

/**
 * Function should perform any necessary initialisation.
 *
 * @param[in] alienContext     See PicselApp_start().
 */
void AlienFile_initialise(Alien_Context *alienContext);

/**
 * Function should perform any necessary finalisation.
 *
 * @param[in] alienContext     See PicselApp_start().
 */
void AlienFile_finalise(Alien_Context *alienContext);

/**
 * Create a directory.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] name          UTF-8 encoded path name of the directory to
 *                          create. This function must indicate success if
 *                          the requested directory already exists.
 *
 * @retval 0         Success
 * @retval -1        Error, or not supported; see @ref TgvFileImplementationCoverage "Implementation Coverage".
 */
int AlienFile_mkdir(Alien_Context *alienContext, const Picsel_Utf8 *name);


/**
 * Open a file for reading or writing data.
 *
 * It is opened for the purpose specified by @p flags. Picsel always
 * expects files to be opened in "binary" mode, so that any 8-bit byte
 * can be loaded or saved.
 *
 * All paths passed from the Picsel library use '/' as the
 * directory separator, so special handling will be required if this is not the
 * directory separator for the platform OS.
 * See @ref AlienFile_PathNames "Path Names".
 *
 * Equivalent to the POSIX @c open() function.
 *
 * @implement If the platform OS is unable to multiple open files, you
 * must return @ref PicselVirtualFile, see @ref TgvVirtualFiles.
 * @n @n
 * Some files are controlled by Digital Rights Management(DRM), see
 * @ref TgvDrmFiles
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] name          UTF-8 encoded filename of file to open.
 * @param[in] flags         Bitwise OR of open flags, see @ref TgvFileFlags,
 *                          @ref TgvDrmFiles.
 *
 * @retval  (>=0)              The file descriptor.
 * @retval  PicselVirtualFile  A virtual file descriptor is to be used, see
 *                             @ref TgvVirtualFiles.
 * @retval  PicselInvalidFile  On error.
 */
int AlienFile_open(Alien_Context     *alienContext,
                   const Picsel_Utf8 *name,
                   int                flags);

/**
 * Close a file.
 *
 * Equivalent to the POSIX @c close() function.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] fd            File descriptor
 *
 * @retval 0         Success
 * @retval -1        Failure
 */
int AlienFile_close(Alien_Context *alienContext, int fd);

/**
 * File read.
 *
 * Equivalent to the POSIX @c read() function.
 *
 * @pre The file has been opened for reading with AlienFile_open(),
 *      see @ref TgvFileFlags.
 *
 * @param[in]  alienContext  See PicselApp_start().
 * @param[in]  fd            File descriptor
 * @param[out] pbuf          Pointer to buffer to read into
 * @param[in]  count         Number of bytes to read
 *
 * @return           Number of bytes read or -1 for error.
 */
long AlienFile_read(Alien_Context *alienContext,
                    int            fd,
                    void          *pbuf,
                    unsigned long  count);

/**
 * File write.
 *
 * Equivalent to the POSIX @c write() function.
 *
 * @pre The file has been opened for writing with AlienFile_open(),
 *      see @ref TgvFileFlags.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] fd            File descriptor
 * @param[in] pbuf          Pointer to buffer to write from
 * @param[in] count         Number of bytes to write
 *
 * @return           Number of bytes written or
 *                   -1 for error or not supported;
 *                   see @ref TgvFileImplementationCoverage "Implementation Coverage".
 */
long AlienFile_write(Alien_Context *alienContext,
                     int            fd,
                     const void    *pbuf,
                     unsigned long  count);

/**
 * Flush the contents of the file.
 *
 * Equivalent to the POSIX @c flush() function.
 *
 * @pre The file has been opened for writing with AlienFile_open(),
 *      see @ref TgvFileFlags.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] fd            File descriptor
 *
 * @retval 0         Success
 * @retval -1        Error, or not supported; see @ref
 *         TgvFileImplementationCoverage "Implementation Coverage".
 */
int AlienFile_flush(Alien_Context *alienContext, int fd);

/**
 * File seek.
 *
 * @pre The file has been opened  with AlienFile_open().
 *
 * @param[in]     alienContext  See PicselApp_start().
 * @param[in]     fd            File descriptor
 * @param[in,out] offset        Offset from origin
 * @param[in]     origin        Seek origin: set, cur, end
 *
 * @return       The offset of the new position from the beginning of the file.
 *               Returns -1 to indicate an error
 *               The offset parameter should be updated to the new offset
 *               from the beginning of the file.
 */
int AlienFile_seek(Alien_Context        *alienContext,
                   int                   fd,
                   PicselFileOffset     *offset,
                   PicselFileSeekOrigin  origin);

/**
 * Find the first entry in the folder @p searchName.
 *
 * If the function fails (i.e. returns @ref PicselFile_findNotADirectory,
 * or @ref PicselFile_findImpossible ) there is no need to call AlienFile_findAllDone().
 *
 * @see @ref TgvEnumeratingFolders.
 *
 * @implement The Alien function should attempt to find the first file (if any)
 * which matches the given name.  The find* functions may use the opaque
 * pointer to store state between calls.  The value set (if any)
 * by the AlienFile_findFirst() function will be passed in to the AlienFile_findNext(),
 * AlienFile_findAgain(), AlienFile_getLastEnumInfo() and AlienFile_findDone()
 * functions, and finally passed  to the AlienFile_findAllDone() function
 * to allow resources to be freed.
 * @n @n
 * If the function returns  @ref PicselFile_findNotADirectory,
 * or @ref PicselFile_findImpossible, you must release any resources
 * allocated before returning. No call AlienFile_findAllDone() will be made.
 *
 *
 * @param[in]  alienContext  See PicselApp_start().
 * @param[in]  searchName    UTF-8 encoded pathname to search on
 * @param[out] findObj       Stores a pointer to information required
 *                           in future calls to AlienFile_findNext(),
 *                           AlienFile_findAgain().  Must be passed
 *                           to AlienFile_findAllDone() to allow
 *                           the Alien application to free up resources.
 * @param[out] found         Pointer to UTF-8 encoded filename to return.
 *                           Caller will pass NULL if not interested in the
 *                           result.
 *
 * @retval  PicselFile_findOK             Found an entry and returned its name
 *                                        in @p found.
 * @retval  PicselFile_findNotADirectory  searchName is not a directory
 * @retval  PicselFile_findAllDone        all files in the folder have been found
 * @retval  PicselFile_findImpossible     Error, or not supported;
 *                                        see @ref TgvFileImplementationCoverage
 *                                        "Implementation Coverage".
 *
 */
PicselFile_findResult AlienFile_findFirst(Alien_Context     *alienContext,
                                          const Picsel_Utf8 *searchName,
                                          void             **findObj,
                                          Picsel_Utf8      **found);

/**
 * Find the next entry in the folder @p searchName.
 *
 * @see @ref TgvEnumeratingFolders.
 *
 * @param[in]  alienContext See PicselApp_start().
 * @param[in]  searchName   UTF-8 encoded filename to search on
 * @param[in]  findObj      Pointer previously set by AlienFile_findFirst().
 * @param[out] found        Pointer to UTF-8 encoded filename to return.
 *                          NULL means don't actually store result
 *                          Function should store a pointer to the result
 *                          here (if NULL is not supplied)
 *
 * @retval  PicselFile_findOK          found an entry and returned name
 * @retval  PicselFile_findAllDone     all files in the folder have been found
 * @retval  PicselFile_findImpossible  Error, or not supported;
 *                                     see @ref TgvFileImplementationCoverage
 *                                     "Implementation Coverage".
 *
 */
PicselFile_findResult  AlienFile_findNext(Alien_Context     *alienContext,
                                          const Picsel_Utf8 *searchName,
                                          void              *findObj,
                                          Picsel_Utf8      **found);

/**
 * Repeat a previous search.
 *
 * Just return the result of a previous find first/next operation.
 *
 *  @see @ref TgvEnumeratingFolders.
 *
 * @param[in]  alienContext  See PicselApp_start().
 * @param[in]  searchName    UTF-8 encoded filename to search on.
 * @param[in]  findObj       Pointer previously set by AlienFile_findFirst().
 * @param[out] found         Pointer to filename to return.
 *                           Call will pass NULL if not interested in the
 *                           result.  The Alien application will not
 *                           free this string until it is passed to
 *                           AlienFile_findDone().
 *
 * @retval  PicselFile_findOK         Found an entry and returned name.
 * @retval  PicselFile_findAllDone    All files in the folder have been found
 * @retval  PicselFile_findImpossible Error, or not supported;
 *                                    see @ref TgvFileImplementationCoverage
 *                                    "Implementation Coverage".
 */
PicselFile_findResult AlienFile_findAgain(Alien_Context     *alienContext,
                                          const Picsel_Utf8 *searchName,
                                          void              *findObj,
                                          Picsel_Utf8      **found);

/**
 * Caller has finished with the result of a find first/next call.
 *
 *  @see @ref TgvEnumeratingFolders.
 *
 * @param[in] alienContext See PicselApp_start().
 * @param[in] findObj      Pointer previously set by AlienFile_findFirst().
 * @param[in] found        Pointer to UTF-8 encoded filename.
 */
void AlienFile_findDone(Alien_Context     *alienContext,
                        void              *findObj,
                        const Picsel_Utf8 *found);

/**
 * Frees resources used by AlienFile_findFirst() and
 * AlienFile_findNext().
 *
 *  @see @ref TgvEnumeratingFolders.
 *
 * @param[in] alienContext See PicselApp_start().
 * @param[in] findObj      Pointer previously set by AlienFile_findFirst().
 */
void AlienFile_findAllDone(Alien_Context *alienContext, void *findObj);

/**
 * Retrieve details of the last file entry returned by
 * AlienFile_findNext()
 *
 * @see @ref TgvEnumeratingFolders.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] findObj       Opaque pointer for use by Alien find*  and
 *                          AlienFile_getLastEnumInfo() functions
 * @param[in] op            Requested information, a bitmask of
 *                          @ref AlienFileOp.
 * @param[out] foundInfo    Pointer to structure to fill in.
 *
 * @implement The function may fill in more fields than requested by @p op.
 * Unused or unsupported fields should be set to 0.
 *
 * @retval  PicselFile_findOK           Information is returned
 * @retval  PicselFile_findImpossible   Error, or not supported;
 *                                      see @ref TgvFileImplementationCoverage
 *                                      "Implementation Coverage".
 */
PicselFile_findResult AlienFile_getLastEnumInfo(Alien_Context *alienContext,
                                                void          *findObj,
                                                unsigned int   op,
                                                AlienFileInfo *foundInfo);


/**
 * Retrieve file information about the specified path.
 *
 * @param[in]  alienContext  See PicselApp_start().
 * @param[in]  path          UTF-8 encoded pathname.
 * @param[out] fileInfo      Pointer to AlienFileInfo structure to fill with
 *                           the file information.
 *
 * @retval  0   Success
 * @retval -1   Error, or not supported;
 *              see @ref TgvFileImplementationCoverage "Implementation Coverage".
 */
int AlienFile_getFileInfoByPath(Alien_Context     *alienContext,
                                const Picsel_Utf8 *path,
                                AlienFileInfo     *fileInfo);

/**
 * Retrieve file information about an open file.
 *
 * Some platforms may not support all fields in @p fileInfo.  All
 * platforms will support @ref AlienFileInfo.size.
 *
 * @implement The Alien may choose to keep this data cached until the file
 * is closed. This may need to be flushed if the file size or modification
 * date changes. All implementations must provide at least the size
 * information.  Unsupported fields must be set to 0.
 *
 *
 * @param[in]  alienContext  See PicselApp_start().
 * @param[in]  fd            File descriptor.
 * @param[out] fileInfo      Pointer to AlienFileInfo structure to fill with
 *                           the file information.
 *
 * @retval 0              Success.
 * @retval -1             Error.
 */
int AlienFile_getFileInfo(Alien_Context *alienContext,
                          int            fd,
                          AlienFileInfo *fileInfo);

/**
 * Fill in an array of meta-data information structures
 * about the specified path.
 *
 * This function will fill in each structure with the
 * requested information, by setting @ref AlienFileMetaInfo.buffer to point to
 * the value in memory allocated from its own store.
 * The Picsel application will call AlienFile_getMetaDataDone() to allow
 * this memory to be freed.
 *
 * The @ref AlienFileMetaInfo.result field of each meta-data structure is used
 * to return success or failure of fetching each individual item of meta-data.
 * Note that the function may return success, but the individual meta-data
 * items may each return failure independently.
 *
 * @param[in]     alienContext  Set by the host as part of PicselApp_start
 *                              command.
 * @param[in]     path          Pointer to UTF-8 encoded pathname
 * @param[in,out] metaArray     Pointer to an array of meta-data structures
 *                              to fill in.
 * @param[in]     nMeta         Number of meta-data items to fill in.
 *
 * @retval AlienFileMetaResult_Success       Success of the whole function
 * @retval AlienFileMetaResult_Failure       Failure of the whole function
 * @retval AlienFileMetaResult_NotSupported  Fetching meta-data not supported;
 *                                           see @ref TgvFileImplementationCoverage
 *                                           "Implementation Coverage".
 */
AlienFileMetaResult AlienFile_getMetaDataByPath(Alien_Context     *alienContext,
                                                const Picsel_Utf8 *path,
                                                AlienFileMetaInfo *metaArray,
                                                int                nMeta);

/**
 * Fill in an array of meta-data information structures
 * about the open file.
 *
 * As for AlienFile_getMetaDataByPath(), but operating on an open file.
 *
 * @param[in]     alienContext  Set by the host as part of PicselApp_start
 *                              command
 * @param[in]     fd            File descriptor
 * @param[in,out] metaArray     pointer to an array of meta-data structures
 *                              to fill in
 * @param[in]     nMeta         Number of meta-data items to fill in
 *
 * @retval AlienFileMetaResult_Success       Success of the whole function
 * @retval AlienFileMetaResult_Failure       Failure of the whole function
 * @retval AlienFileMetaResult_NotSupported  Fetching meta-data not supported;
 *                                           see @ref TgvFileImplementationCoverage
 *                                           "Implementation Coverage".
 */
AlienFileMetaResult AlienFile_getMetaData(Alien_Context     *alienContext,
                                          int                fd,
                                          AlienFileMetaInfo *metaArray,
                                          int                nMeta);


/**
 * Retrieve meta-data information about the last object enumerated.
 *
 * As for AlienFile_getMetaDataByPath(), but operating on the last
 * object enumerated.
 *
 * @param[in]     alienContext  Set by the host as part of PicselApp_start
 *                              command
 * @param[in]     findObj       Opaque pointer for use by Alien find* and
 *                              AlienFile_getLastEnumInfo() functions
 * @param[in,out] meta          pointer to an array of meta-data structures
 *                              to fill in
 * @param[in]     nMeta         number of meta-data items to fill in
 *
 * @retval AlienFileMetaResult_Success       Success of the whole function
 * @retval AlienFileMetaResult_Failure       Failure of the whole function
 * @retval AlienFileMetaResult_NotSupported  Fetching meta-data not supported;
 *                                           see @ref TgvFileImplementationCoverage
 *                                           "Implementation Coverage".
 */
AlienFileMetaResult AlienFile_getLastEnumMetaData(Alien_Context     *alienContext,
                                                  void              *findObj,
                                                  AlienFileMetaInfo *meta,
                                                  int                nMeta);


/**
 * Release memory allocated by the AlienFile_getMetaData* functions.
 *
 * @param[in]     alienContext  Set by the host as part of PicselApp_start
 *                              command
 * @param[in]     meta          pointer to an array of meta-data structures
 *                              to free data from
 * @param[in]     nMeta         number of meta-data items to free
 */
void AlienFile_getMetaDataDone(Alien_Context     *alienContext,
                               AlienFileMetaInfo *meta,
                               int                nMeta);

/**
 * Truncate a file.
 *
 * @pre The file has been opened for writing with AlienFile_open().
 * See @ref TgvFileFlags.
 *
 * If the offset specified is bigger than the file size the function fails.
 *
 * After the file is truncated, the file pointer is moved to the end
 * of the file.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] fd            File descriptor
 * @param[in] position      New size of the file
 *
 * @retval 0              Success
 * @retval -1             Error, or not supported;
 *                        see @ref TgvFileImplementationCoverage "Implementation Coverage".
 */
int AlienFile_truncate(Alien_Context        *alienContext,
                       int                   fd,
                       const PicselFileSize *position);

/**
 * Rename a file.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] name          Old UTF-8 encoded filename for file
 * @param[in] newName       New UTF-8 encoded filename for file
 *
 * @retval 0            Success
 * @retval -1           Error, or not supported;
 *                      see @ref TgvFileImplementationCoverage "Implementation Coverage".
 */
int AlienFile_rename(Alien_Context     *alienContext,
                     const Picsel_Utf8 *name,
                     const Picsel_Utf8 *newName);
/**
 * Copy a file.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] name          UTF-8 encoded filename for source file
 * @param[in] newName       UTF-8 encoded filename for new file
 *
 * @retval 0            Success
 * @retval -1           Error, or not supported;
 *                      see @ref TgvFileImplementationCoverage "Implementation Coverage".
 */
int AlienFile_copy(Alien_Context     *alienContext,
                   const Picsel_Utf8 *name,
                   const Picsel_Utf8 *newName);
/**
 * Delete a file.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] path          UTF-8 encoded path to file
 *
 * @retval 0            Success
 * @retval -1           Error, or not supported;
 *                      see @ref TgvFileImplementationCoverage "Implementation Coverage".
 */
int AlienFile_delete(Alien_Context *alienContext, const Picsel_Utf8 *path);

/**
 * Check if file or directory exists.
 *
 * @implement If the platform does not provide a file existence check, just
 * return -1.  The Picsel library will use AlienFile_open() to check
 * for the file's existence.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] path          UTF-8 encoded path to file
 *
 * @retval 1            File, or directory, exists.
 * @retval 0            File does not exist, or the path is invalid.
 * @retval -1           Error, or not supported;
 *                      see @ref TgvFileImplementationCoverage "Implementation Coverage".
 */
int AlienFile_exists(Alien_Context *alienContext, const Picsel_Utf8 *path);


/**
 * @}
 *//* End ingroup TgvFiles */


/**
 * @ingroup TgvFileTypes
 *
 * Get the MIME type, also known as the Internet Media type, for
 * the specified file extension.  Additional parameters, such as charset, are
 * excluded.
 *
 * This function will be called when the Picsel library encounters an
 * unrecognised file extension.  The Picsel library is aware of all standard
 * file extensions for its supported file types, so for most applications a
 * stub implementation will suffice.  However, if non-standard file
 * extensions are to be supported, this function must be implemented to
 * provide the appropriate MIME type (excluding any additional
 * parameters) for these new file extensions.
 *
 * Any memory allocated for the mimeType must not be deallocated until
 * application shutdown.
 *
 * @param[in]  alienContext   See PicselApp_start().
 * @param[in]  fileExtension  The file extension to retrieve the mimetype for.
 * @param[out] mimeType       Receives the MIME type for the
 *                            file extension.  NULL if the type is unknown,
 *                            or memory could not be allocated for the string.
 */
void AlienFile_getMimeType(Alien_Context       *alienContext,
                           const Picsel_Utf8   *fileExtension,
                           const char         **mimeType);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_FILE_H */
