// Authors: John

#ifndef ONYX_INBOX_H__
#define ONYX_INBOX_H__


#include "message.h"

using namespace base;

namespace sys
{

class Inbox
{
public:
    Inbox(const QString &name = "");
    ~Inbox();

public:
    bool open();
    bool close();

    // Services session.
    void allMessages(Services &);
    bool calibrationService(Service &);
    bool musicService(Service &);
    bool networkService(Service &);
    bool webBrowserService(Service &);
    bool registerService(const Service &, const QString &);
    bool unRegisterService(const Service &);

    // Locale
    QLocale locale();
    bool setLocale(const QLocale & locale);

    // Dictionary directory.
    bool dictionaryRoots(QStringList & dirs);

    QString selectedDictionary();
    bool selectDictionary(const QString & name);

    // Time and date.
    static bool setTimezone(const QString & name);
    static QString currentTimezone();
    static void setDate(int year, int month, int day,
                        int hour, int minute, int second);

    // Music service
    static bool isMusicPlayerAvailable();

    // volume.
    int volume();
    bool setVolume(const int);
    bool mute(bool mute);
    bool isMute();

    // The suspend interval in ms. 0 means it's disabled.
    int suspendInterval();
    bool setSuspendInterval(int ms);

    // The shutdown interval in ms. 0 means it's disabled.
    int shutdownInterval();
    bool setShutdownInterval(int ms);

    // Wifi configuration.
    bool clearWifiProfiles();
    bool loadWifiProfiles(WifiProfiles & all);
    bool saveWifiProfiles(WifiProfiles & all);

    // Dialup configuration.
    bool clearDialupProfiles();
    bool loadDialupProfiles(DialupProfiles & all);
    bool saveDialupProfiles(DialupProfiles & all);

    // Page turning direction.
    static int direction(const QPoint & old_position, const QPoint & new_position);
    static void setDirection(int);
    static int distance();

    // Internet settings.
    QString downloadFolder();

    // System hardware information.
    static QString serialNumber();
    static QString deviceId();
    static QString version();

    // Enable to change boot splash.
    static bool isUpdateSplashEnabled();

    // Get customized home page.
    static bool hasHomePage();
    static const QString & homePageName();

    // User info.
    bool userInfo(UserInfo & data);
    bool updateUserInfo(const UserInfo & data);

private:
    scoped_ptr<QSqlDatabase> database_;
};

};

#endif  // SYSTEM_CONF_H__
