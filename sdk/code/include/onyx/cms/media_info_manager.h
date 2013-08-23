#ifndef CMS_MEDIA_INFO_MANAGER_H_
#define CMS_MEDIA_INFO_MANAGER_H_

#include <QtCore/QtCore>
#include "media_db.h"

namespace cms
{

class MediaInfoManager
{
public:
    MediaInfoManager();
    ~MediaInfoManager();

public:
    QStringList mediaInfo(MediaType type);

    void update(bool is_sd_card = false);

    void scan(bool scan_sd_card = false);
    QStringList extNames(MediaType type);

private:
    bool scanFoldersRecursively(QDir & dir,QStringList & result);
    QStringList booksExtNames();
    QStringList musicExtNames();
    QStringList picturesExtNames();

    QString internalStoragePath();
    QString sdPath();

    void recurseCollect(const QString &sub_dir, const QStringList &name_filters,
            QStringList &path_list);
    QStringList getFullFilter(const QStringList &lower_filter);
    void setFilterForBooks(QStringList &filter);

    void mergeList(MediaDB &db, MediaType type, QStringList &new_list,
            bool is_sd_card);

};

}   // namespace cms

#endif  // CMS_MEDIA_INFO_MANAGER_H_
