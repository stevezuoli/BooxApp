// Authors: John

#ifndef SYS_USER_INFO_H__
#define SYS_USER_INFO_H__

#include <QtCore/QtCore>
#include <QtSql/QtSql>

namespace sys
{

/// Maintain user information in memory.
class UserInfo
{
public:
    UserInfo();
    ~UserInfo();

public:
    const QString & name() const { return name_; }
    QString & mutable_name() { return name_; }

    const QString & pincode() const  { return pincode_; }
    QString & mutable_pincode() { return pincode_; }

    const QDate & birthday() const { return birthday_;}
    QDate & mutable_birthday() { return birthday_;}
    bool isBirthday();

    const QByteArray & attributes() const { return attributes_; }
    QByteArray & mutable_attributes() { return attributes_; }

private:
    QString name_;
    QString pincode_;
    QDate birthday_;
    QByteArray attributes_;
};


/// Interface to database.
class UserConfig
{
public:
    UserConfig();
    ~UserConfig();

private:
    friend class PrivateConfig;

    static bool makeSureTableExist(QSqlDatabase& db);
    static bool info(QSqlDatabase& db, UserInfo & data);
    static bool update(QSqlDatabase & db, const UserInfo & data);
};

/// Info of device web application account
class WebAppConfig
{
private:
    WebAppConfig() {}
    ~WebAppConfig() {}
public:
    static bool makeSureTableExist(QSqlDatabase& db);
    static QStringList credentials(QSqlDatabase& db);
    static bool setCredentials(QSqlDatabase& db, const QStringList & credentials);
};

};

#endif  // SYS_USER_INFO_H__
