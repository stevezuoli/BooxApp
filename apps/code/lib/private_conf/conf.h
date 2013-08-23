
#ifndef PRIVATE_CONF_H_
#define PRIVATE_CONF_H_

#include "onyx/base/base.h"
#include "user_info.h"
#include "drm_info.h"

namespace sys
{

class PrivateConfig
{
public:
    PrivateConfig();
    ~PrivateConfig();

public:
    bool open();
    bool close();

    // User info.
    bool userInfo(UserInfo & data);
    bool updateUserInfo(const UserInfo & data);

    // DRM info.
    bool getDRMInfo(const QString & url, DRMInfo & data);
    bool updateDRMInfo(const DRMInfo & data);
    bool clearDRMInfo();

    // Webapp Info
    QStringList getCredentials();
    bool setCredentials(const QStringList & credentials);

    // Waveform information.
    static QString waveformInfo();

    static QString imei();
    static void saveImei(const QString &imei);

    static QString binaryFingerprint();

private:
    scoped_ptr<QSqlDatabase> database_;
};

}   // namespace sys


#endif  // PRIVATE_CONF_H_
