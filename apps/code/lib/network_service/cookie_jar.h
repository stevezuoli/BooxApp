#ifndef COOKIE_JAR_H_
#define COOKIE_JAR_H_

#include <QtNetwork/QNetworkCookieJar>
#include "onyx/ui/ui.h"
#include "auto_saver.h"

using namespace ui;

namespace network_service
{

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
    Q_PROPERTY(AcceptPolicy acceptPolicy READ acceptPolicy WRITE setAcceptPolicy)
    Q_PROPERTY(KeepPolicy keepPolicy READ keepPolicy WRITE setKeepPolicy)
    Q_PROPERTY(QStringList blockedCookies READ blockedCookies WRITE setBlockedCookies)
    Q_PROPERTY(QStringList allowedCookies READ allowedCookies WRITE setAllowedCookies)
    Q_PROPERTY(QStringList allowForSessionCookies READ allowForSessionCookies WRITE setAllowForSessionCookies)
    Q_ENUMS(KeepPolicy)
    Q_ENUMS(AcceptPolicy)

public:
    enum AcceptPolicy {
        AcceptAlways,
        AcceptNever,
        AcceptOnlyFromSitesNavigatedTo
    };

    enum KeepPolicy {
        KeepUntilExpire,
        KeepUntilExit,
        KeepUntilTimeLimit
    };

public:
    CookieJar(QObject *parent = 0);
    ~CookieJar();

    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;
    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

    AcceptPolicy acceptPolicy() const;
    void setAcceptPolicy(AcceptPolicy policy);

    KeepPolicy keepPolicy() const;
    void setKeepPolicy(KeepPolicy policy);

    QStringList blockedCookies() const;
    QStringList allowedCookies() const;
    QStringList allowForSessionCookies() const;

    void setBlockedCookies(const QStringList &list);
    void setAllowedCookies(const QStringList &list);
    void setAllowForSessionCookies(const QStringList &list);

Q_SIGNALS:
    void cookiesChanged();

public Q_SLOTS:
    void clear();
    void loadSettings();

private Q_SLOTS:
    void save();

private:
    void purgeOldCookies();
    void load();

private:
    bool         loaded_;
    AutoSaver    *save_timer_;
    AcceptPolicy accept_cookies_;
    KeepPolicy   keep_cookies_;

    QStringList  exceptions_block_;
    QStringList  exceptions_allow_;
    QStringList  exceptions_allow_for_session_;
};

};

#endif
