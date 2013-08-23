#ifndef SYS_DRM_INFO_H__
#define SYS_DRM_INFO_H__

#include <QtSql/QtSql>
#include <QString>

namespace sys
{

const char * const DRM_ACTIVATION_NONE = "none";
const char * const DRM_ACTIVATION_OTA  = "ota";
const char * const DRM_ACTIVATION_ADE  = "ade";

/// DRM information
class DRMInfo
{
public:
    DRMInfo();
    DRMInfo(const DRMInfo & right);
    ~DRMInfo() {}

    const QString & id() const { return id_; }
    void set_id(const QString& id) { id_ = id; }

    const QString & password() const { return password_; }
    void set_password(const QString& password) { password_ = password; }

    const QString & url() const { return url_; }
    void set_url(const QString& url) { url_ = url; }

    const QString & activation() const { return activation_; }
    void set_activation(const QString& activation) { activation_ = activation; }

private:
    QString id_;
    QString password_;
    QString url_;
    QString activation_;
};


/// Interface to database.
class DRMConfig
{
public:
    DRMConfig();
    ~DRMConfig();

private:
    static bool makeSureTableExist(QSqlDatabase& db);
    static bool info(QSqlDatabase& db, const QString & url, DRMInfo & data);
    static bool update(QSqlDatabase & db, const DRMInfo & data);
    static bool clear(QSqlDatabase & db);

    friend class PrivateConfig;
};

};

#endif  // SYS_USER_INFO_H__
