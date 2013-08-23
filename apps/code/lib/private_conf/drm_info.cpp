#include "drm_info.h"

namespace sys
{

bool checkColumns(QSqlDatabase& database)
{
    QSqlQuery query(database);
    query.prepare("select sql from sqlite_master where name = ?");
    query.addBindValue("drm_info");
    bool found = false;
    if (query.exec())
    {
        while (query.next())
        {
            QString sql = query.value(0).toString();
            if (sql.contains("activation"))
            {
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        query.prepare("alter table drm_info add activation text");
        return query.exec();
    }
    return true;
}

DRMInfo::DRMInfo()
    : activation_(DRM_ACTIVATION_NONE)
{
}

DRMInfo::DRMInfo(const DRMInfo & right)
    : id_(right.id())
    , password_(right.password())
    , url_(right.url())
    , activation_(right.activation())
{
}

DRMConfig::DRMConfig()
{
}

DRMConfig::~DRMConfig()
{
}

bool DRMConfig::makeSureTableExist(QSqlDatabase& database)
{
    if (!checkColumns(database))
    {
        qDebug("Check null columns. Create a new drm_info table");
    }

    QSqlQuery query(database);
    bool ok = query.exec("create table if not exists drm_info ("
                         "id text primary key,"
                         "password text,"
                         "url text,"
                         "activation text"
                         ")");
    return ok;

}

bool DRMConfig::info(QSqlDatabase& db, const QString & url, DRMInfo & data)
{
    // Query by name, location and size.
    QSqlQuery query(db);
    query.prepare("select id, password, url, activation from drm_info where url = ? ");
    query.addBindValue(url);

    if (query.exec() && query.next())
    {
        int index = 0;
        data.set_id(query.value(index++).toString());
        data.set_password(query.value(index++).toString());
        data.set_url(query.value(index++).toString());
        data.set_activation(query.value(index++).toString());
        return true;
    }
    return false;
}

bool DRMConfig::clear(QSqlDatabase & db)
{
    QSqlQuery clear(db);
    clear.prepare( "delete from drm_info");
    return clear.exec();
}

bool DRMConfig::update(QSqlDatabase & database, const DRMInfo & data)
{
    QSqlQuery delete_old(database);
    delete_old.prepare( "delete from drm_info where url = ?");
    delete_old.addBindValue(data.url());
    if (!delete_old.exec())
    {
        qDebug("Insert a new drm record");
    }
    else
    {
        qDebug("Update the drm record");
    }

    QSqlQuery query(database);
    query.prepare ("INSERT OR REPLACE into drm_info "
                   "(id, password, url, activation)"
                   " values "
                   "(?, ?, ?, ?)");

    query.addBindValue(data.id());
    query.addBindValue(data.password());
    query.addBindValue(data.url());
    query.addBindValue(data.activation());
    return query.exec();
}

}   // namespace sys
