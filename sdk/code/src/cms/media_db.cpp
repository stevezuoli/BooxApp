// Author: Jim

#include "onyx/cms/media_db.h"
#include "onyx/data/data_tags.h"

namespace cms
{

static const QString TAG_PATH = "path";

QString typeString(MediaType type)
{
    QString type_string("unknown");
    switch (type)
    {
    case BOOKS:
        type_string = "books";
        break;
    case PICTURES:
        type_string = "pictures";
        break;
    case MUSIC:
        type_string = "music";
        break;
    default:
        break;
    }
    return type_string;
}


MediaDB::MediaDB(const QString & db_name)
    : database_name_(db_name)
{
    open();
}

MediaDB::~MediaDB()
{
    close();
}

bool MediaDB::open()
{
    if (!database_)
    {
        database_.reset(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", database_name_)));
    }

    if (!database_->isOpen())
    {
        QDir home = QDir::home();
        database_->setDatabaseName(home.filePath(database_name_));
        if (!database_->open())
        {
            qDebug() << "not open" << database_->lastError().text();
            return false;
        }
        makeSureTableExist(*database_);
    }
    return true;
}

bool MediaDB::close()
{
    if (database_)
    {
        database_->close();
        database_.reset(0);
        QSqlDatabase::removeDatabase(database_name_);
        return true;
    }
    return false;
}

MediaInfoList MediaDB::list(MediaType type)
{
    MediaInfoList list;

    QSqlQuery query(db());
    query.prepare( "select type, value from media ");
    if (!query.exec())
    {
        return list;
    }

    while (query.next())
    {
        if (typeString(type) == query.value(0).toString())
        {
            QByteArray ba = query.value(1).toByteArray();
            QDataStream stream(&ba, QIODevice::ReadOnly);
            stream >> list;
        }
    }

    return list;
}

bool MediaDB::update(MediaType type, const MediaInfoList & list)
{
    QSqlQuery query(db());
    query.prepare( "INSERT OR REPLACE into media (type, value) values(?, ?)");
    query.addBindValue(typeString(type));

    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << list;

    query.addBindValue(ba);
    return query.exec();
}

bool MediaDB::remove(MediaType type)
{
    QSqlQuery query(db());
    query.prepare( "delete from media where type = ?");
    query.addBindValue(typeString(type));
    return query.exec();
}

bool MediaDB::makeSureTableExist(QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.exec("create table if not exists media ("
               "type text primary key,"
               "value blob) ");
    return true;
}

QSqlDatabase & MediaDB::db()
{
    return *database_;
}

}   // namespace cms
