#include "conf.h"

namespace sys
{

PrivateConfig::PrivateConfig()
{
    open();

    UserConfig::makeSureTableExist(*database_);
    DRMConfig::makeSureTableExist(*database_);
    WebAppConfig::makeSureTableExist(*database_);
}

PrivateConfig::~PrivateConfig()
{
    close();
}

/// The database is in home directory.
bool PrivateConfig::open()
{
    if (!database_)
    {
        database_.reset(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "private_config")));
        database_->setDatabaseName(QDir::home().filePath("private_config.db"));
    }
    return database_->open();
}

bool PrivateConfig::close()
{
    if (database_)
    {
        database_->close();
        database_.reset(0);
        QSqlDatabase::removeDatabase("private_config");
        return true;
    }
    return false;
}

bool PrivateConfig::userInfo(UserInfo & data)
{
    return UserConfig::info(*database_, data);
}

bool PrivateConfig::updateUserInfo(const UserInfo & data)
{
    return UserConfig::update(*database_, data);
}

bool PrivateConfig::getDRMInfo(const QString & url, DRMInfo & data)
{
    return DRMConfig::info(*database_, url, data);
}

bool PrivateConfig::updateDRMInfo(const DRMInfo & data)
{
    return DRMConfig::update(*database_, data);
}

bool PrivateConfig::clearDRMInfo()
{
    return DRMConfig::clear(*database_);
}

QStringList PrivateConfig::getCredentials()
{
    return WebAppConfig::credentials(*database_);
}

bool PrivateConfig::setCredentials(const QStringList & credentials)
{
    return WebAppConfig::setCredentials(*database_, credentials);
}

QString PrivateConfig::waveformInfo()
{
    QDir home = QDir::home();
    QString path = home.filePath("waveform_info");

    QString info;
    if (!QFile::exists(path))
    {
        info += "V110_B030_60_WE1602_BTC";
    }
    else
    {
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        info += file.readAll();
    }
    return info;
}

QString PrivateConfig::binaryFingerprint()
{
    return qgetenv("GIT_COMMIT");
}

QString PrivateConfig::imei()
{
    QDir home = QDir::home();
    QString path = home.filePath("imei");

    QString info;
    if (!QFile::exists(path))
    {
        info += "";
    }
    else
    {
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        info += file.readAll();
    }
    return info;
}

void PrivateConfig::saveImei(const QString &imei)
{
    QDir home = QDir::home();
    QString path = home.filePath("imei");

    if (!QFile::exists(path))
    {
        QFile file(path);
        file.open(QIODevice::ReadWrite);
        file.write(imei.toAscii());
    }
}


}   // namespace sys


