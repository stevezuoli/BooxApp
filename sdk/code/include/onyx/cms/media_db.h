// Author: Jim

#ifndef CMS_MEDIA_DB_H_
#define CMS_MEDIA_DB_H_

#include <QtCore/QtCore>
#include <QtSql/QtSql>
#include "onyx/base/base.h"
#include "onyx/data/data.h"

namespace cms
{

enum MediaType
{
    UNKNOWN = -1,
    BOOKS,
    PICTURES,
    MUSIC,
};

QString typeString(MediaType type);

typedef QStringList MediaInfoList;

class MediaDB
{
public:
    MediaDB(const QString & db_name = "media.db");
    ~MediaDB();

public:
    bool open();
    bool close();

    MediaInfoList list(MediaType type = BOOKS);

    bool update(MediaType type, const MediaInfoList & list);
    bool remove(MediaType type);

private:
    bool makeSureTableExist(QSqlDatabase &db);
    QSqlDatabase & db();

private:
    scoped_ptr<QSqlDatabase> database_;
    const QString database_name_;

};

}  // namespace cms

#endif  // CMS_MEDIA_DB_H_
