/*
    Author	: Zarjazz
    Email	: zarjazz@quakenet.org
    Copyright (C) 2008  Vincent Sweeney
    ----
    Read the LICENSE file for more details
*/

#include <QString>
#include <QFile>
#include <QDir>
#include <QProcess>

#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

#include "archive_file_stream.h"

/*
 * Statics
 */

#ifdef __cplusplus
extern "C" {
#endif 

static int rarCallback(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2)
{
    QByteArray *data = (QByteArray *) UserData;
    
    switch (msg)
    {
    case UCM_CHANGEVOLUME:
        break;
    case UCM_PROCESSDATA:
        data->append(QByteArray((char *) P1, P2));
        break;
    case UCM_NEEDPASSWORD:
        return -1;
    }

    return 0;
}

static bool initLZMA = false;

#ifdef _LZMA_IN_CB

#define kBufferSize (1 << 12)
Byte g_Buffer[kBufferSize];

static SZ_RESULT szCallback_Read(void *object, void **buffer,
        size_t maxRequiredSize, size_t *processedSize)
{
    szCallbackData *cb = (szCallbackData *) object;
    QDataStream stream(cb->file);

    size_t processedSizeLoc;

    if (maxRequiredSize > kBufferSize)
        maxRequiredSize = kBufferSize;
    processedSizeLoc = stream.readRawData((char *) g_Buffer, maxRequiredSize);
    *buffer = g_Buffer;
    if (processedSize != 0)
        *processedSize = processedSizeLoc;
    return SZ_OK;
}

#else

static SZ_RESULT szCallback_Read(void *object, void *buffer, size_t size,
        size_t *processedSize)
{
    szCallbackData *cb = (szCallbackData *) object;
    QDataStream stream((cb->file));

    size_t processedSizeLoc = stream.readRawData((char *) buffer, size);
    if (processedSize != 0)
        *processedSize = processedSizeLoc;
    return SZ_OK;
}

#endif

static SZ_RESULT szCallback_Seek(void *object, CFileSize pos)
{
    szCallbackData *cb = (szCallbackData *) object;

    bool res = cb->file->seek(pos);
    if (res == true)
        return SZ_OK;
    return SZE_FAIL;
}

#ifdef __cplusplus
}
#endif

namespace compression
{

/*
 *  Constructors
 */

ArchiveFileStream::ArchiveFileStream()
    : file_index_(0)
{
    type = ArchiveFileStream::IStream_None;
    file_filter_ = NULL;
    pZipFile = NULL;
    rarFile = 0;
    szFile = NULL;

    // 7z Initialization
    if (!initLZMA)
    {
        initLZMA = true;
        // One-time 7z Initialization
        CrcGenerateTable();
    }

    szCBData.InStream.Read = szCallback_Read;
    szCBData.InStream.Seek = szCallback_Seek;
    szCBData.file = szFile;

    allocImp.Alloc = SzAlloc;
    allocImp.Free  = SzFree;

    allocTempImp.Alloc = SzAllocTemp;
    allocTempImp.Free  = SzFreeTemp;

    szBlockIndex = 0xFFFFFFFF;
    szOutBuffer = 0;
    szOutBufferSize = 0;
}

ArchiveFileStream::~ArchiveFileStream()
{
    clrFilter();
    close();
}

QStringList ArchiveFileStream::dirList(QDir &qdir, QRegExp *filter,
        QDir::Filters ftype)
{
    QStringList list;

    qdir.setFilter(ftype);
    qdir.setSorting(QDir::Name);

    if (filter)
        list = qdir.entryList().filter((*filter));
    else
        list = qdir.entryList();

    return list;
}

QStringList ArchiveFileStream::zipList(unzFile file)
{
    QStringList list;
    
    unz_global_info gi;
    uLong i;
    int err;

    err = unzGetGlobalInfo(file, &gi);
    if (err != UNZ_OK) return QStringList();

    for (i = 0; i < gi.number_entry; i++)
    {
        unz_file_info file_info;
        char zipfile[512];

        if (i > 0)
        {
            err = unzGoToNextFile(file);
            if (err != UNZ_OK) break;
        }

        err = unzGetCurrentFileInfo(file, &file_info, zipfile, sizeof(zipfile),
                NULL, 0, NULL, 0);
        if (err != UNZ_OK) continue;

        if (file_info.uncompressed_size > 0)
            list.append(zipfile);
    }

    if (file_filter_) list = list.filter((*file_filter_));
    list.sort();

    return list;
}

QStringList ArchiveFileStream::rarList(HANDLE file,
        RAROpenArchiveDataEx *flags, RARHeaderDataEx *header)
{
    QStringList list;

    while (!RARReadHeaderEx(file, header))
    {
        if (RARProcessFile(file, RAR_SKIP, NULL, NULL)) break;
#ifdef UNICODE
        list.append(QString::fromWCharArray(header->FileNameW));
#else
        list.append(header->FileName);
#endif
    }

    if (file_filter_) list = list.filter((*file_filter_));
    list.sort();

    return list;
}

QStringList ArchiveFileStream::szList(CArchiveDatabaseEx *db)
{
    QStringList list;

    UInt32 i;
    for (i = 0; i < db->Database.NumFiles; i++)
    {
        CFileItem *f = db->Database.Files + i;
        list.append(f->Name);
    }

    if (file_filter_) list = list.filter((*file_filter_));
    list.sort();

    return list;
}

void ArchiveFileStream::close()
{
    absolute_path_name_.clear();
    curFileName.clear();
    rawData.clear();
    arcList.clear();

    if (pZipFile) unzCloseCurrentFile(pZipFile);
    pZipFile = NULL;

    rarClose();
    rarPassword.clear();

    if (szFile)
    {
        szFile->close();
        delete szFile;
        szFile = NULL;
        SzArDbExFree(&szDB, allocImp.Free);

    }

    if (szOutBuffer) allocImp.Free(szOutBuffer);

    szBlockIndex = 0xFFFFFFFF;
    szOutBuffer = 0;
    szOutBufferSize = 0;

    type = ArchiveFileStream::IStream_None;
}

bool ArchiveFileStream::open(const QString &path)
{
    // Reset any existing content
    errStr.clear();
    close();

    QFileInfo file(path);
    archive_file_name_ = file.fileName();
    curFileName = archive_file_name_;
    absolute_path_name_ = path;

    if (file.isDir())
    {
        curDir = QDir(path);

        type = ArchiveFileStream::IStream_File;
        return first();
    }
    else
        curDir = file.dir();


    if (curFileName.isEmpty())
    {
        errStr = "Missing Filename: " + path;
        return false;
    }

    QRegExp rx;

    rx.setCaseSensitivity(Qt::CaseInsensitive);
    rx.setPattern("\\.(zip|cbz)$");
    if (curFileName.contains(rx))
    {
        pZipFile = unzOpen(path.toLocal8Bit());
        if (!pZipFile)
        {
            errStr = "Unable to open ZIP File: " + path;
            return false;
        }

        arcList = zipList(pZipFile);
        if (arcList.isEmpty())
        {
            errStr = "No Valid Files in ZIP: " + path;
            close();
            return false;
        }

        type = ArchiveFileStream::IStream_ZIP;
        return first();
    }

    rx.setCaseSensitivity(Qt::CaseInsensitive);
    rx.setPattern("\\.(rar|cbr)$");
    if (curFileName.contains(rx))
    {
        rarFile = rarOpen(path, &rarFlags, &rarHeader, RAR_OM_LIST);
        if (!rarFile) return false;

        arcList = rarList(rarFile, &rarFlags, &rarHeader);
        if (arcList.isEmpty())
        {
            errStr = "No Valid Files in RAR: " + path;
            close();
            return false;
        }

        // We have to reopen .rar archives to read the actual data
        rarClose();

        type = ArchiveFileStream::IStream_RAR;
        return first();
    }

    rx.setCaseSensitivity(Qt::CaseInsensitive);
    rx.setPattern("\\.(7z|cb7)$");
    if (curFileName.contains(rx))
    {
        szFile = new QFile(path);
        if (!szFile->open(QFile::ReadOnly))
        {
            errStr = "Unable to open 7Z File: " + path;
            return false;
        }

        szCBData.file = szFile;
        SzArDbExInit(&szDB);
        SZ_RESULT res = SzArchiveOpen((ISzInStream *) &szCBData, &szDB,
                &allocImp, &allocTempImp);
        if (res != SZ_OK)
        {
            szFile->close();
            delete szFile;
            szFile = NULL;
            close();
            return false;
        }

        arcList = szList(&szDB);
        if (arcList.isEmpty())
        {
            errStr = "No Valid Files in 7Z: " + path;
            close();
            return false;
        }

        type = ArchiveFileStream::IStream_7Z;
        return first();
    }

    type = ArchiveFileStream::IStream_File;
    return loadData();
}

bool ArchiveFileStream::loadData()
{
    if (type == ArchiveFileStream::IStream_File)
    {
        QFile file(curDir.absolutePath() + "/" + curFileName);
        if (!file.open(QFile::ReadOnly))
        {
            errStr = "Unable to Open File: " + file.fileName();
 			return false;
        }

        rawData = file.readAll();
        if (rawData.isEmpty())
        {
            errStr = "Empty File: " + file.fileName();
 			return false;
        }
    }
    else if (type == ArchiveFileStream::IStream_ZIP)
    {
        unz_file_info file_info;
        char zipfile[512];
        int err;

        if (!pZipFile)
        {
            errStr = "ZIP File not Open: " + absolute_path_name_;
            return false;
        }

        err = unzLocateFile(pZipFile, curFileName.toLocal8Bit(), 0);
        if (err != UNZ_OK)
        {
            errStr = "ZIP - Unable to locate: " + curFileName;
            return false;
        }

        err = unzGetCurrentFileInfo(pZipFile, &file_info, zipfile,
                sizeof(zipfile), NULL, 0, NULL, 0);
        if (err != UNZ_OK || file_info.uncompressed_size == 0)
        {
            errStr = "ZIP - Unable get FileInfo: " + curFileName;
            return false;
        }

        err = unzOpenCurrentFile(pZipFile);
        if (err != UNZ_OK)
        {
            errStr = "ZIP - Unable to open file: " + curFileName;
            return false;
        }

        const int BUFSIZE = 8192;
        char *buf = new char[BUFSIZE];

        rawData.clear();
        do
        {
            err = unzReadCurrentFile(pZipFile, buf, BUFSIZE);
            if (err < 0)
            {
            	errStr = "ZIP - Error reading: [" + QString::number(
                        rawData.size()) + "] " + curFileName;
            	return false;
            }

            if (err > 0)
            	rawData.append( QByteArray(buf, err) );
        } while (err > 0);

        delete buf;

        err = unzCloseCurrentFile(pZipFile);
       		if (err != UNZ_OK)
        {
            errStr = "ZIP - Unable to close file: " + curFileName;
            return false;
        }

        if (((unsigned int)rawData.size()) != file_info.uncompressed_size)
        {
            errStr = "ZIP - Deflate Error: [" + QString::number(rawData.size())
                    + " | " + QString::number(file_info.uncompressed_size)
                    + "] " + curFileName;
            return false;
        }

        return true;
    }
    else if (type == ArchiveFileStream::IStream_RAR)
    {
        rarFile = rarOpen(absolute_path_name_, &rarFlags, &rarHeader,
                RAR_OM_EXTRACT);
        if (!rarFile) return false;

        size_t length = 0;
        while (!RARReadHeaderEx(rarFile, &rarHeader))
        {
#ifdef UNICODE
            if (curFileName == QString::fromWCharArray(rarHeader.FileNameW))
#else
            if (curFileName == rarHeader.FileName)
#endif
            {
            	length = rarHeader.UnpSize;
            	break;
            }

            if (RARProcessFile(rarFile, RAR_SKIP, NULL, NULL))
            {
            	rarClose();
            	errStr = "RAR - Error loading file: " + curFileName;
            	return false;
            }

        }

        if (!length)
        {
            rarClose();
            errStr = "RAR - Unable to locate: " + curFileName;
            return false;
        }

        rawData.clear();
        RARSetCallback(rarFile, (UNRARCALLBACK) rarCallback, (LPARAM) &rawData);
        int ret = RARProcessFile(rarFile, RAR_TEST, NULL, NULL);
        rarClose();

        if (ret)
        {
            errStr = "RAR - Error extracting file: " + curFileName;
            return false;
        }
    }
    else if (type == ArchiveFileStream::IStream_7Z)
    {
        if (!szFile)
        {
            errStr = "7Z File not Open: " + absolute_path_name_;
            return false;
        }

        for (UInt32 i = 0; i < szDB.Database.NumFiles; i++)
        {
            SZ_RESULT res;
            size_t offset;
            size_t outSizeProcessed;

            CFileItem *f = szDB.Database.Files + i;
            if (f->IsDirectory || curFileName != f->Name) continue;

            res = SzExtract((ISzInStream *) &szCBData, &szDB, i, 
            	&szBlockIndex, &szOutBuffer, &szOutBufferSize, 
            	&offset, &outSizeProcessed, &allocImp, &allocTempImp);

            if (res != SZ_OK)
            {
            	errStr = "7Z - Error extracting file: " + curFileName;
            	return false;
            }

            rawData = QByteArray::fromRawData((char *) (szOutBuffer + offset),
                    outSizeProcessed);
            break;
        }

        return true;
    }

    return true;
}

int ArchiveFileStream::fileIndex()
{
    return file_index_;
}

void ArchiveFileStream::setCurrentFileName(QStringList &file_list, int index)
{
    file_index_ = index;
    curFileName = file_list.at(index);
}

bool ArchiveFileStream::first()
{
    QStringList list;

    if (isArchive())
        list = arcList;
    else
        list = dirList(curDir, file_filter_);

    if (list.isEmpty())
    {
        errStr = "Image List Empty";
 		return false;
    }

    setCurrentFileName(list, 0);
    return loadData();
}

bool ArchiveFileStream::last()
{
    QStringList list;

    if (isArchive())
        list = arcList;
    else
        list = dirList(curDir, file_filter_);

    if (list.isEmpty())
    {
        errStr = "Image List Empty";
 		return false;
    }

    setCurrentFileName(list, list.size() - 1);
    return loadData();
}

bool ArchiveFileStream::next()
{
    QStringList list;

    if (isArchive())
        list = arcList;
    else
        list = dirList(curDir, file_filter_);

    if (list.isEmpty())
    {
        errStr = "Image List Empty";
 		return false;
    }

    int n = list.indexOf(curFileName);
    if (n < 0)
        return first();
    else if (n == list.size() - 1)
        return false;
    else
    {
        setCurrentFileName(list, n + 1);
    }

    return loadData();
}

bool ArchiveFileStream::prev()
{
    QStringList list;

    if (isArchive())
        list = arcList;
    else
        list = dirList(curDir, file_filter_);

    if (list.isEmpty())
    {
        errStr = "Image List Empty";
 		return false;
    }

    int n = list.indexOf(curFileName);
    if (n < 0)
        return first();
    else if (n)
    {
        setCurrentFileName(list, n - 1);
    }
    else
        return false;

    return loadData();
}

bool ArchiveFileStream::page(int page)
{
    QStringList list;

    if (isArchive())
        list = arcList;
    else
        list = dirList(curDir, file_filter_);

    if (list.isEmpty())
    {
        errStr = "Image List Empty";
 		return false;
    }

    if (page < 0 || page > list.size() - 1)
        return false;
    setCurrentFileName(list, page);

    return loadData();
}

QStringList ArchiveFileStream::getPageList()
{
    QStringList list;

    if (isArchive())
        list = arcList;
    else
        list = dirList(curDir, file_filter_);

    return list;
}

const QString ArchiveFileStream::suggest(bool forward)
{
    QString sPath, path;

    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    if (isArchive())
    {
        path = curDir.absolutePath();
        rx.setPattern("\\.(zip|rar|7z|cb[rz7])$");

        QStringList list = dirList(curDir, &rx);
        int index = list.indexOf(archive_file_name_);

        if (forward && index >= 0 && index < (list.size() - 1))
            sPath = path + "/" + list.at(index + 1);
        else if (!forward && index > 0)
            sPath = path + "/" + list.at(index - 1);
    }
    else
    {
        path = QDir::fromNativeSeparators(absolute_path_name_);
//		rx.setPattern("^[^\\.]");

        QStringList split = path.split("/", QString::SkipEmptyParts);
        QString cwd = split.takeLast();
        QDir upDir(split.join("/"));
        if (!split.isEmpty() && curDir != upDir)
        {
            QStringList list = dirList(upDir, NULL, QDir::Dirs
                    | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable);
            path = upDir.absolutePath();

            int index = list.indexOf(cwd);
            if (forward && index >= 0 && index < (list.size() - 1))
            	sPath = path + "/" + list.at(index + 1);
            else if (!forward && index > 0)
            	sPath = path + "/" + list.at(index - 1);
        }
    }

    return sPath;
}

void ArchiveFileStream::clrFilter()
{
    delete file_filter_;
    file_filter_ = NULL;
}

void ArchiveFileStream::setFilter(QRegExp &rx)
{
    delete file_filter_;
    file_filter_ = new QRegExp(rx);
}

void ArchiveFileStream::rarClose()
{
    if (rarFile)
    {
        RARCloseArchive(rarFile);
        rarFile = 0;
    }
}

HANDLE ArchiveFileStream::rarOpen(const QString &filename,
        RAROpenArchiveDataEx *flags, RARHeaderDataEx *header, int mode)
{
    HANDLE file;

    memset(flags, 0, sizeof(*flags));

#ifdef UNICODE
    flags->ArcNameW = new wchar_t[filename.length() + 1];
    filename.toWCharArray(flags->ArcNameW);
    flags->ArcNameW[filename.length()] = 0;
#else
    flags->ArcName = new char[filename.length() + 1];
    memcpy(flags->ArcName, filename.toLocal8Bit(), filename.length() + 1);
#endif
    flags->CmtBuf = NULL;
    flags->OpenMode = mode;

    file = RAROpenArchiveEx(flags);
    if (flags->OpenResult)
    {
        RARCloseArchive(file);
        return 0;
    }

    header->CmtBuf = NULL;

    if (!file)
    {
        errStr = "Unable to open RAR File: " + filename;
        return 0;
    }

    // prompt for password if required
    if (flags->Flags & 0x0080)
    {
        if (rarPassword.isEmpty())
        {
            bool ok;
            rarPassword = QInputDialog::getText(NULL,
                    "Archive is Password Protected.", "Enter Password:",
                    QLineEdit::Normal, NULL, &ok);

            if (!ok || rarPassword.isEmpty())
            {
            	RARCloseArchive(file);
            	errStr = "No Password Given.";
            	return 0;
            }
        }
        RARSetPassword(file, (char *) rarPassword.toLocal8Bit().constData());
    }

    return file;
}

}   // namespace comic_reader
