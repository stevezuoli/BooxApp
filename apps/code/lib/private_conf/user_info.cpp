#include "user_info.h"

namespace sys
{

UserInfo::UserInfo()
{
}

UserInfo::~UserInfo()
{
}

bool UserInfo::isBirthday()
{
    return (QDate::currentDate().month() == birthday_.month() &&
            QDate::currentDate().day() == birthday_.day());
}


UserConfig::UserConfig()
{
}

UserConfig::~UserConfig()
{
}

bool UserConfig::makeSureTableExist(QSqlDatabase& database)
{
    QSqlQuery query(database);
    bool ok = query.exec("create table if not exists user_info ("
                         "name text,"
                         "pincode text,"
                         "birthday date,"
                         "attributes blob "
                         ")");
    return ok;

}

bool UserConfig::info(QSqlDatabase& db, UserInfo & data)
{
    // Query by name, location and size.
    QSqlQuery query(db);
    query.prepare( "select name, pincode, birthday, attributes "
                   "from user_info" );

    if (query.exec() && query.next())
    {
        int index = 0;
        data.mutable_name() = query.value(index++).toString();
        data.mutable_pincode() = query.value(index++).toString();
        data.mutable_birthday() = query.value(index++).toDate();
        data.mutable_attributes() = query.value(index++).toByteArray();
        return true;
    }
    return false;
}

bool UserConfig::update(QSqlDatabase & database, const UserInfo & data)
{
    QSqlQuery query(database);
    query.prepare( "delete from user_info");
    query.exec();

    query.prepare ("insert into user_info "
                   "(name, pincode, birthday, attributes) "
                   " values "
                   "(?, ?, ?, ?)");

    query.addBindValue(data.name());
    query.addBindValue(data.pincode());
    query.addBindValue(data.birthday());
    query.addBindValue(data.attributes());
    return query.exec();
}

bool WebAppConfig::makeSureTableExist(QSqlDatabase& db)
{
    QSqlQuery query(db);
    bool ok = query.exec("create table if not exists webapp_info ("
                         "auth_key text,"
                         "otp text"
                         ")");
    return ok;
}

QStringList WebAppConfig::credentials(QSqlDatabase& db)
{
    QStringList credentials;
    QSqlQuery query(db);
    query.prepare( "select auth_key, otp from webapp_info" );
    if (query.exec() && query.next())
    {
        int index = 0;
        QString auth_key = query.value(index++).toString();
        QString otp = query.value(index++).toString();
        credentials.push_back(auth_key);
        credentials.push_back(otp);
    }
    return credentials;
}

bool WebAppConfig::setCredentials(QSqlDatabase& db, const QStringList & credentials)
{
    if (credentials.size() < 2)
    {
        return false;
    }

    QSqlQuery query(db);
    query.prepare( "delete from webapp_info");
    query.exec();

    query.prepare ("insert into webapp_info "
                   "(auth_key, otp) "
                   " values "
                   "(?, ?)");

    query.addBindValue(credentials[0]);
    query.addBindValue(credentials[1]);
    return query.exec();
}

}   // namespace sys
