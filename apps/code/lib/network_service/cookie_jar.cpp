#include <QtWebKit/QWebSettings>
#include "cookie_jar.h"

static const unsigned int JAR_VERSION = 23;

QT_BEGIN_NAMESPACE
QDataStream &operator<<(QDataStream &stream, const QList<QNetworkCookie> &list)
{
    stream << JAR_VERSION;
    stream << quint32(list.size());
    for (int i = 0; i < list.size(); ++i)
        stream << list.at(i).toRawForm();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<QNetworkCookie> &list)
{
    list.clear();

    quint32 version;
    stream >> version;

    if (version != JAR_VERSION)
        return stream;

    quint32 count;
    stream >> count;
    for(quint32 i = 0; i < count; ++i)
    {
        QByteArray value;
        stream >> value;
        QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(value);
        if (newCookies.count() == 0 && value.length() != 0) {
            qWarning() << "CookieJar: Unable to parse saved cookie:" << value;
        }
        for (int j = 0; j < newCookies.count(); ++j)
            list.append(newCookies.at(j));
        if (stream.atEnd())
            break;
    }
    return stream;
}
QT_END_NAMESPACE

namespace network_service
{

CookieJar::CookieJar(QObject *parent)
    : QNetworkCookieJar(parent)
    , loaded_(false)
    , save_timer_(new AutoSaver(this))
    , accept_cookies_(AcceptOnlyFromSitesNavigatedTo)
{
}

CookieJar::~CookieJar()
{
    if (keep_cookies_ == KeepUntilExit)
    {
        clear();
    }
    save_timer_->saveIfNeccessary();
}

void CookieJar::clear()
{
    setAllCookies(QList<QNetworkCookie>());
    save_timer_->changeOccurred();
    emit cookiesChanged();
}

void CookieJar::load()
{
    if (loaded_)
        return;
    // load cookies and exceptions
    qRegisterMetaTypeStreamOperators<QList<QNetworkCookie> >("QList<QNetworkCookie>");
    QSettings cookieSettings(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QLatin1String("/cookies.ini"), QSettings::IniFormat);
    setAllCookies(qvariant_cast<QList<QNetworkCookie> >(cookieSettings.value(QLatin1String("cookies"))));
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    exceptions_block_ = cookieSettings.value(QLatin1String("block")).toStringList();
    exceptions_allow_ = cookieSettings.value(QLatin1String("allow")).toStringList();
    exceptions_allow_for_session_ = cookieSettings.value(QLatin1String("allowForSession")).toStringList();
    qSort(exceptions_block_.begin(), exceptions_block_.end());
    qSort(exceptions_allow_.begin(), exceptions_allow_.end());
    qSort(exceptions_allow_for_session_.begin(), exceptions_allow_for_session_.end());

    loadSettings();
}

void CookieJar::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("cookies"));
    QByteArray value = settings.value(QLatin1String("acceptCookies"),
                        QLatin1String("AcceptOnlyFromSitesNavigatedTo")).toByteArray();
    QMetaEnum acceptPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    accept_cookies_ = acceptPolicyEnum.keyToValue(value) == -1 ?
                        AcceptOnlyFromSitesNavigatedTo :
                        static_cast<AcceptPolicy>(acceptPolicyEnum.keyToValue(value));

    value = settings.value(QLatin1String("keepCookiesUntil"), QLatin1String("KeepUntilExpire")).toByteArray();
    QMetaEnum keepPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KeepPolicy"));
    keep_cookies_ = keepPolicyEnum.keyToValue(value) == -1 ?
                        KeepUntilExpire :
                        static_cast<KeepPolicy>(keepPolicyEnum.keyToValue(value));

    if (keep_cookies_ == KeepUntilExit)
    {
        setAllCookies(QList<QNetworkCookie>());
    }

    loaded_ = true;
    emit cookiesChanged();
}

void CookieJar::save()
{
    if (!loaded_)
    {
        return;
    }

    purgeOldCookies();
    QString directory = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    if (directory.isEmpty())
    {
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    }

    if (!QFile::exists(directory))
    {
        QDir dir;
        dir.mkpath(directory);
    }
    QSettings cookieSettings(directory + QLatin1String("/cookies.ini"), QSettings::IniFormat);
    QList<QNetworkCookie> cookies = allCookies();
    for (int i = cookies.count() - 1; i >= 0; --i)
    {
        if (cookies.at(i).isSessionCookie())
            cookies.removeAt(i);
    }
    cookieSettings.setValue(QLatin1String("cookies"), qVariantFromValue<QList<QNetworkCookie> >(cookies));
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    cookieSettings.setValue(QLatin1String("block"), exceptions_block_);
    cookieSettings.setValue(QLatin1String("allow"), exceptions_allow_);
    cookieSettings.setValue(QLatin1String("allowForSession"), exceptions_allow_for_session_);

    // save cookie settings
    QSettings settings;
    settings.beginGroup(QLatin1String("cookies"));
    QMetaEnum acceptPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    settings.setValue(QLatin1String("acceptCookies"), QLatin1String(acceptPolicyEnum.valueToKey(accept_cookies_)));

    QMetaEnum keepPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KeepPolicy"));
    settings.setValue(QLatin1String("keepCookiesUntil"), QLatin1String(keepPolicyEnum.valueToKey(keep_cookies_)));
}

void CookieJar::purgeOldCookies()
{
    QList<QNetworkCookie> cookies = allCookies();
    if (cookies.isEmpty())
    {
        return;
    }

    int oldCount = cookies.count();
    QDateTime now = QDateTime::currentDateTime();
    for (int i = cookies.count() - 1; i >= 0; --i)
    {
        if (!cookies.at(i).isSessionCookie() && cookies.at(i).expirationDate() < now)
        {
            cookies.removeAt(i);
        }
    }

    if (oldCount == cookies.count())
    {
        return;
    }
    setAllCookies(cookies);
    emit cookiesChanged();
}

QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const
{
    CookieJar *that = const_cast<CookieJar*>(this);
    if (!loaded_)
    {
        that->load();
    }

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        QList<QNetworkCookie> noCookies;
        return noCookies;
    }

    return QNetworkCookieJar::cookiesForUrl(url);
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    if (!loaded_)
    {
        load();
    }

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        return false;
    }

    QString host = url.host();
    bool eBlock = qBinaryFind(exceptions_block_.begin(), exceptions_block_.end(), host) != exceptions_block_.end();
    bool eAllow = qBinaryFind(exceptions_allow_.begin(), exceptions_allow_.end(), host) != exceptions_allow_.end();
    bool eAllowSession = qBinaryFind(exceptions_allow_for_session_.begin(), exceptions_allow_for_session_.end(), host) != exceptions_allow_for_session_.end();

    bool addedCookies = false;
    // pass exceptions
    bool acceptInitially = (accept_cookies_ != AcceptNever);
    if ((acceptInitially && !eBlock)
        || (!acceptInitially && (eAllow || eAllowSession))) {
        // pass url domain == cookie domain
        QDateTime soon = QDateTime::currentDateTime();
        soon = soon.addDays(90);
        foreach(QNetworkCookie cookie, cookieList) {
            QList<QNetworkCookie> lst;
            if (keep_cookies_ == KeepUntilTimeLimit
                && !cookie.isSessionCookie()
                && cookie.expirationDate() > soon) {
                    cookie.setExpirationDate(soon);
            }
            lst += cookie;
            if (QNetworkCookieJar::setCookiesFromUrl(lst, url)) {
                addedCookies = true;
            } else {
                // finally force it in if wanted
                if (accept_cookies_ == AcceptAlways) {
                    QList<QNetworkCookie> cookies = allCookies();
                    cookies += cookie;
                    setAllCookies(cookies);
                    addedCookies = true;
                }
#if 0
                else
                    qWarning() << "setCookiesFromUrl failed" << url << cookieList.value(0).toRawForm();
#endif
            }
        }
    }

    if (addedCookies) {
        save_timer_->changeOccurred();
        emit cookiesChanged();
    }
    return addedCookies;
}

CookieJar::AcceptPolicy CookieJar::acceptPolicy() const
{
    if (!loaded_)
        (const_cast<CookieJar*>(this))->load();
    return accept_cookies_;
}

void CookieJar::setAcceptPolicy(AcceptPolicy policy)
{
    if (!loaded_)
        load();
    if (policy == accept_cookies_)
        return;
    accept_cookies_ = policy;
    save_timer_->changeOccurred();
}

CookieJar::KeepPolicy CookieJar::keepPolicy() const
{
    if (!loaded_)
        (const_cast<CookieJar*>(this))->load();
    return keep_cookies_;
}

void CookieJar::setKeepPolicy(KeepPolicy policy)
{
    if (!loaded_)
        load();
    if (policy == keep_cookies_)
        return;
    keep_cookies_ = policy;
    save_timer_->changeOccurred();
}

QStringList CookieJar::blockedCookies() const
{
    if (!loaded_)
        (const_cast<CookieJar*>(this))->load();
    return exceptions_block_;
}

QStringList CookieJar::allowedCookies() const
{
    if (!loaded_)
        (const_cast<CookieJar*>(this))->load();
    return exceptions_allow_;
}

QStringList CookieJar::allowForSessionCookies() const
{
    if (!loaded_)
        (const_cast<CookieJar*>(this))->load();
    return exceptions_allow_for_session_;
}

void CookieJar::setBlockedCookies(const QStringList &list)
{
    if (!loaded_)
        load();
    exceptions_block_ = list;
    qSort(exceptions_block_.begin(), exceptions_block_.end());
    save_timer_->changeOccurred();
}

void CookieJar::setAllowedCookies(const QStringList &list)
{
    if (!loaded_)
        load();
    exceptions_allow_ = list;
    qSort(exceptions_allow_.begin(), exceptions_allow_.end());
    save_timer_->changeOccurred();
}

void CookieJar::setAllowForSessionCookies(const QStringList &list)
{
    if (!loaded_)
        load();
    exceptions_allow_for_session_ = list;
    qSort(exceptions_allow_for_session_.begin(), exceptions_allow_for_session_.end());
    save_timer_->changeOccurred();
}

}