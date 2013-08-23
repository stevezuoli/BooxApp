#include "onyx/cms/media_info_manager.h"

#include "onyx/base/device.h"
#include "onyx/sys/sys_conf.h"
#include <QVector>

using namespace sys;

namespace cms
{

MediaInfoManager::MediaInfoManager()
{

}

MediaInfoManager::~MediaInfoManager()
{
}

bool MediaInfoManager::scanFoldersRecursively(QDir & dir,
                                              QStringList & result)
{
    QDir::Filters filters = QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot;
    QFileInfoList all = dir.entryInfoList(filters);
    for(QFileInfoList::iterator iter = all.begin(); iter != all.end(); ++iter)
    {
        if (iter->isFile())
        {
            result.push_back(iter->absoluteFilePath());
        }
        else if (iter->isDir())
        {
            QDir d(iter->absoluteFilePath());
            scanFoldersRecursively(d, result);
        }
    }
    return true;
}

QStringList MediaInfoManager::booksExtNames()
{
    QStringList temp;
    Service service;
    SystemConfig conf;
    conf.nabooReaderService(service);
    temp << service.mutable_extensions();
    conf.onyxReaderService(service);
    temp << service.mutable_extensions();
    conf.htmlReaderService(service);
    temp << service.extensions();
    conf.webBrowserService(service);
    temp << service.mutable_extensions();
    temp << "djv" << "djvu";
    return temp;
}

QStringList MediaInfoManager::musicExtNames()
{
    QStringList temp;
    Service service;
    SystemConfig conf;
    conf.musicService(service);
    temp << service.extensions();
    return temp;
}

QStringList MediaInfoManager::picturesExtNames()
{
    QList<QString> supported_formats;
    QList<QByteArray> list = QImageReader::supportedImageFormats();
    for(QList<QByteArray>::iterator it = list.begin(); it != list.end(); ++it)
    {
        QString ext(*it);
        ext = ext.toLower();
        supported_formats.push_back(ext);
    }
    return supported_formats;
}

QStringList MediaInfoManager::extNames(MediaType type)
{
    if (type == BOOKS)
    {
        return booksExtNames();
    }
    else if (type == PICTURES)
    {
        return picturesExtNames();
    }
    else if (type == MUSIC)
    {
        return musicExtNames();
    }
    return QStringList();
}

QString MediaInfoManager::internalStoragePath()
{
#ifndef WIN32
    return LIBRARY_ROOT;
#else
    return "G:\\sample";
#endif
}

QString MediaInfoManager::sdPath()
{
#ifndef WIN32
    return SDMMC_ROOT;
#else
    return "d:/";
#endif
}

/// scan both internal flash and sd card.
void MediaInfoManager::scan(bool scan_sd_card)
{
    QDir dir(internalStoragePath());
    QStringList result;
    scanFoldersRecursively(dir, result);

    if (scan_sd_card)
    {
        QDir sd(sdPath());
        scanFoldersRecursively(sd, result);
    }

    QStringList booksExts = booksExtNames();
    QStringList picsExts  = picturesExtNames();
    QStringList musicExts = musicExtNames();
    QStringList books, pics, music;
    foreach(QString path, result)
    {
        foreach(QString ext, booksExts)
        {
            if (path.endsWith(ext, Qt::CaseInsensitive))
            {
                books.push_back(path);
                continue;
            }
        }

        foreach(QString ext, picsExts)
        {
            if (path.endsWith(ext, Qt::CaseInsensitive))
            {
                pics.push_back(path);
                continue;
            }
        }

        foreach(QString ext, musicExts)
        {
            if (path.endsWith(ext, Qt::CaseInsensitive))
            {
                music.push_back(path);
            }
        }
    }

    MediaDB db;
    db.update(PICTURES, pics);
    db.update(BOOKS, books);
    db.update(MUSIC, music);
}

void MediaInfoManager::recurseCollect(const QString &sub_dir,
                                      const QStringList &name_filters,
                                      QStringList &path_list)
{
    QDir dir(sub_dir);
    QFileInfoList dirList = dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    for (QFileInfoList::iterator itr = dirList.begin();
                itr != dirList.end(); ++itr)
    {
        recurseCollect(itr->absoluteFilePath(), name_filters, path_list);
    }

    QDir::Filters filters = QDir::Files|QDir::NoDotAndDotDot|QDir::CaseSensitive;
    QFileInfoList target_files = dir.entryInfoList(name_filters, filters);
    for (QFileInfoList::iterator itr = target_files.begin();
            itr != target_files.end(); ++itr)
    {
        QString item = itr->absoluteFilePath();
        if (!path_list.contains(item))
        {
            path_list.push_back(item);
        }
    }
}

QStringList MediaInfoManager::getFullFilter(const QStringList &lower_filter)
{
    QStringList full_filter(lower_filter);
    for (int i = 0; i < lower_filter.size(); i++)
    {
        full_filter.push_back(lower_filter.at(i).toUpper());
    }
    return full_filter;
}

void MediaInfoManager::setFilterForBooks(QStringList &filter)
{
    // get extensions from service configuration
    QStringList temp;
    Service service;
    SystemConfig conf;
    conf.nabooReaderService(service);
    temp << service.mutable_extensions();
    conf.onyxReaderService(service);
    temp << service.mutable_extensions();
    conf.htmlReaderService(service);
    temp << "chm";
    conf.webBrowserService(service);
    temp << service.mutable_extensions();
    temp << "djv" << "djvu";

    int size = temp.size();
    for (int i=0; i<size; i++)
    {
        QString ext_item = temp.at(i);
        ext_item.prepend("*.");
        filter.push_back(ext_item);
    }
}

QStringList MediaInfoManager::mediaInfo(MediaType type)
{
    MediaDB db;
    return db.list(type);
}

void MediaInfoManager::mergeList(MediaDB &db, MediaType type,
        QStringList &new_list, bool is_sd_card)
{
    MediaInfoList origin = db.list(type);
    int size = origin.size();
    for (int i=0; i<size; i++)
    {
        QString prefix(LIBRARY_ROOT);
        if (is_sd_card)
        {
            prefix = SDMMC_ROOT;
        }

        if (!origin.at(i).startsWith(prefix))
        {
            new_list.push_front(origin.at(i));
        }
    }
}

void MediaInfoManager::update(bool is_sd_card)
{


    // collect pictures
    QStringList pictures_list;
    QStringList lower_filter("*.png");
    lower_filter << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif"
            << "*.tif" << "*.tiff";
    QStringList full_filter = getFullFilter(lower_filter);
    if (is_sd_card)
    {
        recurseCollect(SDMMC_ROOT, full_filter, pictures_list);
    }
    else
    {
        recurseCollect(LIBRARY_ROOT, full_filter, pictures_list);
    }

    {
        MediaDB db;
        mergeList(db, PICTURES, pictures_list, is_sd_card);
        db.update(PICTURES, pictures_list);
    }

    // collect books
    QStringList books_list;
    lower_filter.clear();
    setFilterForBooks(lower_filter);
    full_filter = getFullFilter(lower_filter);
    if (is_sd_card)
    {
        recurseCollect(SDMMC_ROOT, full_filter, books_list);
    }
    else
    {
        recurseCollect(LIBRARY_ROOT, full_filter, books_list);
    }

    {
        MediaDB db;
        mergeList(db, BOOKS, books_list, is_sd_card);
        db.update(BOOKS, books_list);
    }

    // collect music
    QStringList music_list;
    lower_filter.clear();
    lower_filter << "*.mp3" << "*.wav";
    full_filter = getFullFilter(lower_filter);
    if (is_sd_card)
    {
        recurseCollect(SDMMC_ROOT, full_filter, music_list);
    }
    else
    {
        recurseCollect(LIBRARY_ROOT, full_filter, music_list);
    }

    {
        MediaDB db;
        mergeList(db, MUSIC, music_list, is_sd_card);
        db.update(MUSIC, music_list);
    }
}

}   // namespace cms
