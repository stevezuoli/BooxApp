/*
    Author	: Zarjazz
    Email	: zarjazz@quakenet.org
    Copyright (C) 2008  Vincent Sweeney
    ----
    Read the LICENSE file for more details
*/

#ifndef COMIC_IMAGE_STREAM_H_
#define COMIC_IMAGE_STREAM_H_

#ifdef WIN32
  #include <zlib/zlib.h>
  #include <windows.h>
#else
  #include <zlib.h>
#endif

#include "compression/minizip/unzip.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "compression/7z/7zIn.h"
#include "compression/7z/7zExtract.h"
#include "compression/7z/7zCrc.h"

// This is how 7z passes an argument to the callback functions - fugly
typedef struct _szCallbackData
{
    ISzInStream InStream;
    QFile *file;
} szCallbackData;

#ifdef __cplusplus
}
#endif

#include "compression/unrar/raros.hpp"
#include "compression/unrar/dll.hpp"

namespace compression
{

/*
 *  This class is a decompression class for archive file. Supports archive
 *  formats (cbz|zip, cbr|rar, cb7|7z). File filter can be set to get the
 *  target files.
 */

class ArchiveFileStream
{
public:
    ArchiveFileStream();
    ~ArchiveFileStream();

    enum IStreamType {
        IStream_None,
        IStream_File,
        IStream_ZIP,
        IStream_RAR,
        IStream_7Z
    };

    inline bool isArchive() {
        return (type == IStream_ZIP || type == IStream_RAR || type
                == IStream_7Z);
    }
    inline bool isEmpty()	{ return (type == IStream_None); }

    inline const QString &	errstr()  { return errStr; }
    inline const QString 	curFile() { return curFileName; }
    inline const QString 	curPath() { return absolute_path_name_; }
    inline const QString	cwd() { return curDir.path(); }
    inline const QByteArray & rawdata() { return rawData; }

    bool open(const QString &);
    void close();
    int fileIndex();    // get the index of current file
    bool first();
    bool last();
    bool next();
    bool prev();
    bool page(int page);
    bool loadData();

    const QString suggestPrev() { return suggest(false); }
    const QString suggestNext() { return suggest(true); }
    QStringList getPageList();

    void clrFilter();
    void setFilter(QRegExp &rx);

private:
    // only use this method to set curFileName
    void setCurrentFileName(QStringList &file_list, int index);

private:
    IStreamType type;
    QString errStr;

    QDir curDir;
    QString absolute_path_name_;    ///< absolute path of the archive file (with archive file name)
    QString archive_file_name_;     ///< the file name of the archive (without path)
    QString curFileName;    ///< file name for decompression, it could be
                            /// archive_file_name_ or name of internal file in archive

    int file_index_;
    QRegExp *file_filter_;  ///< filter of files in archive, e.g. pattern "\\.( png | jpg )$"
    QByteArray rawData;

    QStringList arcList;

    QStringList dirList(QDir &qdir, QRegExp *filter, QDir::Filters ftype =
            QDir::Files);
    QStringList zipList(unzFile file);
    QStringList rarList(HANDLE file, RAROpenArchiveDataEx *flags,
            RARHeaderDataEx *header);
    QStringList szList (CArchiveDatabaseEx *db);

    const QString suggest(bool forward);

    // ZIP File
    unzFile pZipFile;

    // RAR File
    HANDLE rarFile;
    QString rarPassword;

    struct RARHeaderDataEx rarHeader;
    struct RAROpenArchiveDataEx rarFlags;

    void rarClose();
    HANDLE rarOpen(const QString &filename, RAROpenArchiveDataEx *flags,
            RARHeaderDataEx *header, int mode);

    // 7Z File
    QFile *szFile;
    CArchiveDatabaseEx szDB;
    szCallbackData szCBData;
    ISzAlloc allocImp;
    ISzAlloc allocTempImp;

    UInt32 szBlockIndex;
    Byte *szOutBuffer;
    size_t szOutBufferSize;
};

}   // namespace comic_reader

#endif
