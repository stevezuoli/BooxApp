/**
 * Linux alien implementation
 *
 * Copyright (C) Picsel, 2004-2008. All Rights Reserved.
 *
 * @author Picsel Technologies Ltd
 *
 * $Id: alien-file.c,v 1.3 2009/07/09 15:56:37 dpt Exp $
 */


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "alien-file.h"

#define COPYBUF_SIZE (4096)

#ifndef PATH_MAX
#define PATH_MAX 256
#endif /* !PATH_MAX */

typedef struct findEntry
{
    struct dirent **namelist;
    int             current;
    int             numEnts;
    char           *pattern;
    char           *lastFound;
}
findEntry;


/**
 * Get the next file from the directory enuemration
 *
 * @param[in,out] entries   structure containing enumeration state
 * @param[out]    found     address of the filename
 * @retval    PicselFile_findAllDone  no (more) entries to return
 * @retval    PicselFile_findOK       success, filename returned in found
 */
static PicselFile_findResult findNextInternal(findEntry   *entries,
                                              char       **found)
{
    struct stat buf;
    int         dtype;

    /* enumerate next file after current*/
    entries->current++;

    if (entries->current == entries->numEnts)
    {
        return PicselFile_findAllDone;
    }

    /*
     * Some implementations (iPAQ) don't fill in the d_type so if unknown
     * use stat on the file name to make a guess.  At some point will
     * need to cope with systems that don't have a d_type in the dirent
     * structure.
     */
    if (entries->namelist[entries->current]->d_type == DT_UNKNOWN)
    {
        stat(entries->namelist[entries->current]->d_name, &buf);
        /*
         * Have seen on iPAQ st_mode not being set correctly for reg files
         * so default to reg file type.
         */
        dtype = DT_REG;
        if (S_ISDIR(buf.st_mode))
        {
            dtype = DT_DIR;
        }

        entries->namelist[entries->current]->d_type = dtype;
    }


    if (found)
    {
        *found = entries->namelist[entries->current]->d_name;
    }

    return PicselFile_findOK;
}

/**
 * Copy information from a linux 'struct stat' to a Picsel AlienFileInfo
 *
 * The target structure has already been cleared by the caller.
 *
 * @param[in] fileInfo  address of a Picsel AlienFileInfo structure
 * @param[in] stats     address of a linux stat structure
 */
static void copyStats(AlienFileInfo *fileInfo, struct stat *stats)
{
    fileInfo->attrib = 0;
    if(stats->st_mode & S_IFDIR)
    {
        fileInfo->attrib |= AlienFileAttrib_Dir;
    }
    if((stats->st_mode & S_IWUSR) == 0)
    {
        fileInfo->attrib |= AlienFileAttrib_ReadOnly;
    }

    fileInfo->size.low = (unsigned int) stats->st_size;
    fileInfo->size.high = (unsigned int)
                    ((unsigned long long)stats->st_size >> 32);

    fileInfo->modificationDate = (unsigned long) stats->st_mtime;
    fileInfo->accessDate = (unsigned long) stats->st_atime;
    fileInfo->creationDate = (unsigned long) stats->st_ctime;
}


/** Shim to File mkdir function
 */
int AlienFile_mkdir(Alien_Context *ac, const char *name)
{
    int ret;
    struct stat info;

    ac = ac; /* Unused, shush compiler */

    /* Work around bug in TGV sending us extra leading /s */
    if ( (name[0] == '/') && (name[1] == '/') )
        name++;

    /* make new directory */
    ret=mkdir(name, S_IRUSR|S_IWUSR|S_IXUSR);

    if (ret==0)
    {
        /* Success */
        return 0;
    }

    if (errno!=EEXIST)
    {
        /* Failure */
        return -1;
    }

    /*
     * *something* exists using the file name we want, check if it is a
     * directory
     */
    ret = stat(name, &info);
    if (ret!=0)
    {
        /* Failure */
        return -1;
    }

    if (S_ISDIR(info.st_mode))
    {
        /* it is a directory */
        return 0;
    }
    else
    {
        return -1;
    }
}


/** Shim to File write function
 */
long AlienFile_write(Alien_Context *ac,
                     int fd, const void *pbuf, unsigned long count)
{
    ac = ac; /* Unused, shush compiler */

    /* N.B. write returns -1 on error */
    return (write(fd, pbuf, count));
}


/** Shim to File rename function
 */
int AlienFile_rename(Alien_Context *ac,const char *name, const char *newname)
{
    ac = ac; /* Unused, shush compiler */

    /* Work around bug in TGV sending us extra leading /s */
    if ( (name[0] == '/') && (name[1] == '/') )
        name++;

    /* Work around bug in TGV sending us extra leading /s */
    if ( (newname[0] == '/') && (newname[1] == '/') )
        newname++;

    /* N.B. rename returns -1 on error */
    return (rename(name,newname));
}

/** Shim to File copy function
 */
int AlienFile_copy(Alien_Context *ac,
                   const char    *name,
                   const char    *newname)
{
    char    *tmpbuf;
    FILE    *in = NULL;
    FILE    *out = NULL;
    size_t   lump, done;

    ac = ac; /* Unused, shush compiler */

    /* Work around bug in TGV sending us extra leading /s */
    if ( (name[0] == '/') && (name[1] == '/') )
        name++;

    /* Work around bug in TGV sending us extra leading /s */
    if ( (newname[0] == '/') && (newname[1] == '/') )
        newname++;

    tmpbuf = malloc(COPYBUF_SIZE);
    if ( NULL == tmpbuf )
        return -1;

    in = fopen(name,"rb");
    if ( NULL == in )
        goto error;

    out = fopen(newname,"wb");
    if ( NULL == out )
        goto error;

    for(;;)
    {
        lump = fread( tmpbuf, 1, COPYBUF_SIZE, in );
        if ( !lump )
            break;
        done = fwrite( tmpbuf, 1, lump, out );
        if ( done < lump )
            goto error;
    }

    fclose( in );
    fclose( out );
    free( tmpbuf );

    return 0;

error:
    if ( in )
        fclose( in );
    if ( out )
        fclose( out );
    free( tmpbuf );

    remove( newname );  /* Remove any possible partial file */

    return -1;
}

/** Shim to File delete function
 */
int AlienFile_delete(Alien_Context *ac,const char *path)
{
    ac = ac; /* Unused, shush compiler */

    /* Work around bug in TGV sending us extra leading /s */
    if ( (path[0] == '/') && (path[1] == '/') )
        path++;

    /* N.B. remove returns -1 on error */
    return (remove(path));
}

/** Shim to File close function
 */
int AlienFile_close(Alien_Context *ac,int fd)
{
    ac = ac; /* Unused, shush compiler */

    /* N.B. close returns -1 on error */
    return (close(fd));
}

/** Shim to File initialise function
 *
 *  Function should perform any necessary initialisation
 */
void AlienFile_initialise(Alien_Context *ac)
{
    ac = ac; /* Unused, shush compiler */
}


/** Shim to File finalise function
 *
 *  Function should perform any necessary finalisation
 */
void AlienFile_finalise(Alien_Context *ac)
{
    ac = ac; /* Unused, shush compiler */
}


/** Shim to File open function
 */
int AlienFile_open(Alien_Context *ac, const char *name, int flags)
{
    int oflags;
    int ret;
    int fd;
    struct stat buf;

    ac = ac; /* Unused, shush compiler */

    /* Work around bug in TGV sending us extra leading /s */
    if ( (name[0] == '/') && (name[1] == '/') )
        name++;

    if (flags & PicselFileFlagReadOnly)
    {
        oflags = O_RDONLY;
    }
    else if (flags & PicselFileFlagWriteOnly)
    {
        oflags = O_WRONLY;
    }
    else
    {
        oflags = O_RDWR;
    }

    if (flags & PicselFileFlagCreate)
    {
        oflags |= O_CREAT;
    }
    if (flags & PicselFileFlagTruncate)
    {
        oflags |= O_TRUNC;
    }

    fd = open(name, oflags, S_IRUSR|S_IWUSR);

    if (fd == -1)
    {
        return PicselInvalidFile;
    }

    ret = fstat(fd, &buf);
    if (ret == -1)
    {
        printf("alien-file.c: stat of %s failed : %s\n",
               name, strerror(errno));
        close(fd);
        return PicselInvalidFile;
    }

    if (!S_ISREG(buf.st_mode))
    {
        printf("alien-file.c: %s is not a regular file\n", name);
        close(fd);
        return PicselInvalidFile;
    }

    return fd;
}

/** Shim to File read function
 */
long AlienFile_read(Alien_Context *ac,int fd, void *pbuf, unsigned long count)
{
    ac = ac; /* Unused, shush compiler */

    /* N.B. read returns -1 on error */
    return (read(fd, pbuf, count));
}

/** Shim to File seek function
 */
int AlienFile_seek(Alien_Context        *ac,
                   int                   fd,
                   PicselFileOffset     *pos,
                   PicselFileSeekOrigin  origin)
{
    int   whence;
    int   offset;
    off_t newPos;

    ac = ac; /* Unused, shush compiler */

    /* Linux only has a 32bit seek (that we're using),
     * so if the high word is set for an absolute seek,
     * we cannot function.
     */
    if (pos->high != 0 &&
        origin == PicselFileSeekSet)
        return -1;

    switch(origin)
    {
        case PicselFileSeekSet:
            whence = SEEK_SET;
        break;
        case PicselFileSeekCur:
            whence = SEEK_CUR;
        break;
        case PicselFileSeekEnd:
            whence = SEEK_END;
        break;
        default:
            return -1;
        break;
    }

    offset = 0;
    if (pos)
    {
        offset = pos->low;
    }

    newPos = lseek(fd, offset, whence);
    if (newPos == (off_t)-1)
        return -1;

    pos->high = 0;
    pos->low  = newPos;

    if (newPos < 0 || newPos > INT_MAX)
        return INT_MAX;

    return newPos;
}


/** Shim to File findFirst function
 *
 * The Alien function should attempt to find the first file (if any)
 * which matches the given name.  The find* functions may use the opaque
 * pointer which to store state between calls.  The value set (if any)
 * by the findFirst function will be passed in to the findNext and
 * findAgain functions, and finally passed to the findAllDone function
 * for any necessary tidying up.
 *
 * In common with the other aliens, this has been modified to expect a
 * directory rather than a filename.
 */
PicselFile_findResult AlienFile_findFirst(Alien_Context *ac,
                                          const char    *searchName,
                                          void         **findObj,
                                          char         **found)
{
    findEntry *entries;

    ac = ac; /* Unused, shush compiler */

    entries = (findEntry *)calloc(1, sizeof(findEntry));
    *findObj = entries;
    if (entries == NULL)
    {
        return PicselFile_findImpossible;  /**< Insufficient resources */
    }

    assert(searchName);

    /* Work around bug in TGV sending us extra leading /s */
    if ( (searchName[0] == '/') && (searchName[1] == '/') )
        searchName++;

    entries->pattern = (char *)malloc(sizeof(char) * strlen(searchName) + 1);
    if (entries->pattern == NULL)
    {
        free(entries);
        *findObj = NULL;
        return PicselFile_findImpossible;
    }

    strcpy(entries->pattern, searchName);

    /* N.B. scandir will ensure we have a directory */

    /*
     * Scan the given directory
     */
    entries->numEnts = scandir(entries->pattern,
                               &entries->namelist, NULL, alphasort);

    if (entries->numEnts == -1)
    {
        /* an error occured in scandir() */
        free(entries->pattern);
        free(entries);
        *findObj = NULL;
        return PicselFile_findNotADirectory;
    }
    else if (entries->numEnts == 0)
    {
        return PicselFile_findAllDone;
    }

    /*
     * Find first file
     */
    entries->current = -1;
    return findNextInternal(entries, found);
}


/** Shim to File findNext function
 */
PicselFile_findResult  AlienFile_findNext(Alien_Context *ac,
                                          const char    *searchName,
                                          void          *findObj,
                                          char         **found)
{
    findEntry *entries = (findEntry *)findObj;

    ac = ac; /* Unused, shush compiler */
    searchName = searchName; /* Unused, shush compiler */

    if ( entries->numEnts <= 0 || entries->current == entries->numEnts)
    {
        return PicselFile_findAllDone;
    }

    return findNextInternal(entries, found);
}


/** Shim to File findAgain function
 *
 * Just return the result of a previous find first/next operation.
 */
PicselFile_findResult AlienFile_findAgain(Alien_Context *ac,
                                          const char    *searchName,
                                          void          *findObj,
                                          char         **found)
{
    findEntry *entries = (findEntry *)findObj;

    ac = ac; /* Unused, shush compiler */
    searchName = searchName; /* Unused, shush compiler */

    if ( entries->numEnts <= 0)
    {
        return PicselFile_findAllDone;
    }

    *found = entries->namelist[entries->current]->d_name;

    return PicselFile_findOK;
}


/** Shim to File findDone function
 *
 * Caller has finished with the result of a find first/next call.
 */
void AlienFile_findDone(Alien_Context *ac, void *findObj, const char *found)
{
    /* Nothing to do as tidied up when findObj freed.  This assumes
     * we don't mind having the memory tied up during this time. Otherwise
     * we can change to free individual entries when no longer required.
     */

    ac = ac; /* Unused, shush compiler */
    findObj = findObj; /* Unused, shush compiler */
    found = found; /* Unused, shush compiler */
}

/** Shim to File findFirst/Next done function
 */
void AlienFile_findAllDone(Alien_Context *ac, void *findObj)
{
    findEntry *entries = (findEntry *)findObj;
    int i;

    ac = ac; /* Unused, shush compiler */

    if (entries)
    {
        for (i = 0 ; i < entries->numEnts ; i++)
        {
            free(entries->namelist[i]);
        }
        free(entries->namelist);
        free(entries->pattern);
        free(entries);
    }
}

/** Shim to File getLastEnumInfo
 */
PicselFile_findResult
AlienFile_getLastEnumInfo(Alien_Context *ac,
                          void          *findObj,
                          unsigned int   op,
                          AlienFileInfo *foundInfo)
{
    char      *path = NULL;
    findEntry *entries = (findEntry *)findObj;

    /* Unused, shush compiler */
    ac = ac;
    op = op;

    assert(NULL != findObj);
    assert(NULL != foundInfo);

    if (entries->current >= entries->numEnts)
    {
        return PicselFile_findImpossible;
    }

    assert(NULL != entries->namelist[entries->current]->d_name);
    assert(NULL != entries->pattern);

    /* allocate space for path including separator and terminating nul */
    path = malloc(strlen(entries->namelist[entries->current]->d_name) +
                  strlen(entries->pattern) + 2);
    if (NULL == path)
    {
        return PicselFile_findImpossible;
    }
    strcpy(path, entries->pattern);
    strcat(path, "/");
    strcat(path, entries->namelist[entries->current]->d_name);

    if (0 != AlienFile_getFileInfoByPath(ac, path, foundInfo))
    {
        free(path);
        return PicselFile_findImpossible;
    }
    else
    {
        free(path);
        return PicselFile_findOK;
    }
}

/** Shim to File flush function
 */
int AlienFile_flush(Alien_Context *ac, int fd)
{
    ac = ac; /* Unused, shush compiler */

    /* N.B. fdatasync returns -1 on error */
    return (fdatasync(fd));
}

/** Shim to File truncate
 */
int AlienFile_truncate(Alien_Context          *ac,
                       int                     fd,
                       const PicselFileSize   *position)
{
    assert(position->high == 0);
    if (position->high != 0)
        return -1;

    ac = ac; /* Unused, shush compiler */

    /* N.B. ftruncate returns -1 on error */
    return (ftruncate(fd, position->low)) ? 0 : -1;
}

/**
 * AlienFile_getFileInfoByPath
 */
int AlienFile_getFileInfoByPath(Alien_Context *ac,
                                const char    *path,
                                AlienFileInfo *fileInfo)
{
    struct stat  stats;
    int          retval;

    assert(NULL != path);
    assert(NULL != fileInfo);

    ac = ac; /* Unused, shush compiler */

    /* Work around bug in TGV sending us extra leading /s */
    if ( (path[0] == '/') && (path[1] == '/') )
        path++;

    memset(fileInfo, 0, sizeof(*fileInfo));

    retval = stat(path, &stats);
    if (retval == 0)
    {
        copyStats(fileInfo, &stats);        /* copy info */
    }
    else
    {
        retval = -1;
    }

    return retval;
}

/**
 * AlienFile_getFileInfo
 */
int AlienFile_getFileInfo(Alien_Context *ac,
                          int            fd,
                          AlienFileInfo *fileInfo)
{
    struct stat stats;
    int         retval;

    assert(NULL != fileInfo);

    ac = ac; /* Unused, shush compiler */

    memset(fileInfo, 0, sizeof(*fileInfo));

    retval = fstat(fd, &stats);

    if (retval == 0)
    {
        copyStats(fileInfo, &stats);        /* copy info */
    }
    else
    {
        retval = -1;
    }

    return retval;
}


/**
 * File exists
 *
 * @param ac        Set by the host as part of PicselApp_start
 * @param path      UTF-8 encoded path to file
 *
 * @retval 1        file exists
 * @retval 0        file does not exist
 */
int AlienFile_exists(Alien_Context *ac, const char *path)
{
    struct stat *statData;
    int          ret;

    assert( path != NULL );

    ac = ac; /* Unused, shush compiler */

    /* Work around bug in TGV sending us extra leading /s */
    if ( (path[0] == '/') && (path[1] == '/') )
        path++;

    statData = malloc(sizeof(*statData));
    if (statData == NULL)
        return -1; /* Out of memory, so return error */

    ret  = stat(path, statData);
    if (ret == 0)
        ret = 1; /* File exists */
    else
        ret = 0; /* File not found (or some other error) */

    free(statData);
    return ret;
}


/*------------------------------------------------------------------------*/

/**
 * Retrieve metadata information about the specified path.
 *
 * This function is called by the Picsel application to read meta
 * data about a file. The alien should fill in the structure with
 * details, allocating memory from its own store as appropriate.
 * The Picsel application will call AlienFile_getMetaDataDone to
 * request that the memory allocated in this call be freed.
 */
AlienFileMetaResult AlienFile_getMetaDataByPath(Alien_Context     *ac,
                                                const char        *path,
                                                AlienFileMetaInfo *meta,
                                                int                nMeta)
{
    assert(ac != NULL);
    assert(path != NULL);
    assert(meta != NULL);
    assert(nMeta > 0);

    if (ac == NULL ||
        path == NULL ||
        meta == NULL ||
        nMeta <= 0)
    {
        /* Invalid parameters */
        return AlienFileMetaResult_Failure;
    }

    for (; nMeta>0; nMeta--, meta++)
    {
        switch (meta->op)
        {
            /* Example implementation to read a native file link */
            case AlienFileMetaOp_NativeFilename:
                {
                    static char destbuffer[PATH_MAX];

                    if (readlink(path, destbuffer, sizeof(destbuffer)) == -1)
                    {
                        /* An error occurred, so return -1 to indicate fault */
                        meta->result =AlienFileMetaResult_Failure;
                    }
                    else
                    {
                        meta->buffer = strdup(destbuffer);

                        if (meta->buffer != NULL)
                        {
                            meta->bufferLen = strlen(destbuffer) + 1;
                            meta->result = AlienFileMetaResult_Success;
                        }
                        else
                        {
                            meta->result = AlienFileMetaResult_Failure;
                        }
                    }
                }
                break;

            default:
                /* Not implemented, so return not supported */
                meta->result = AlienFileMetaResult_NotSupported;
                break;
        }
    }

    return AlienFileMetaResult_Success;
}

/*------------------------------------------------------------------------*/

/**
 * Retrieve metadata information about the open file.
 *
 * (as for AlienFile_getMetaDataByPath, but operating on an open file)
 */
AlienFileMetaResult AlienFile_getMetaData(Alien_Context     *ac,
                                          int                fd,
                                          AlienFileMetaInfo *meta,
                                          int                nMeta)
{
    /* Unused, shush compiler */
    ac = ac;
    fd = fd;
    meta = meta;
    nMeta = nMeta;

    /*
     * Not implemented but since getMetaDataByPath is implemented it is
     * unsafe to return not supported here or we might ignore a DRM
     * limitation.
     */
    return AlienFileMetaResult_Failure;
}

/*------------------------------------------------------------------------*/

/**
 * Retrieve metadata information about the last object enumerated.
 *
 * (as for AlienFile_getMetaDataByPath, but operating on the last object
 * enumerated)
 */
AlienFileMetaResult AlienFile_getLastEnumMetaData(Alien_Context     *ac,
                                                  void              *findObj,
                                                  AlienFileMetaInfo *meta,
                                                  int                nMeta)
{
    /* Unused, shush compiler */
    ac = ac;
    findObj = findObj;
    meta = meta;
    nMeta = nMeta;

    /*
     * Not implemented but since getMetaDataByPath is implemented it is
     * unsafe to return not supported here or we might ignore a DRM
     * limitation.
     */
    return AlienFileMetaResult_Failure;
}

/*------------------------------------------------------------------------*/

/**
 * Release memory allocated by a metadata request.
 *
 * (allocated by :
 *    AlienFile_getMetaDataByPath,
 *    AlienFile_getMetaData
 *    AlienFile_getLastEnumMetaData)
 */
void AlienFile_getMetaDataDone(Alien_Context     *ac,
                               AlienFileMetaInfo *meta,
                               int                nMeta)
{
    assert(ac != NULL);
    assert(meta != NULL);
    assert(nMeta > 0);

    if (ac == NULL ||
        meta == NULL ||
        nMeta <= 0)
    {
        /* Invalid parameters */
        return;
    }

    for (; nMeta>0; nMeta--, meta++)
    {
        if ( meta->result == AlienFileMetaResult_Success )
        {
            switch (meta->op)
            {
                case AlienFileMetaOp_NativeFilename:
                    free(meta->buffer);
                    meta->buffer = NULL;
                    break;

                default:
                    break;
            }
        }
    }
}

/**
 * Get the MIME (1.0) Content-type (excluding any additional parameters) for
 * the specified file extension.
 *
 * This function will be called when the Picsel library encounters an
 * unrecognised file extension.  The Picsel library is aware of all standard
 * file extensions for its supported file types, so for most applications a
 * stub implementation will suffice.  However, if non-standard file
 * extensions are to be supported, this function must be implemented to
 * provide the appropriate MIME (1.0) Content-type (excluding any additional
 * parameters) for these new file extensions.
 *
 * Any memory allocated for the mimeType must not be deallocated until
 * application shutdown.
 *
 * @param[in]  alienContext   See PicselApp_start().
 * @param[in]  fileExtension  The file extension to retrieve the mimetype for.
 * @param[out] mimeType       Receives the MIME (1.0) Content-type for the
 *                            file extension.  NULL if the type is unknown.
 */
void AlienFile_getMimeType(Alien_Context       *alienContext,
                           const Picsel_Utf8   *fileExtension,
                           const char         **mimeType)
{
    /* Not implemented */

    /* Unused, shush compiler */
    alienContext  = alienContext;
    fileExtension = fileExtension;
    mimeType      = mimeType;
    return;
}
