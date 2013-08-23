#include "onyx/screen/screen_proxy.h"
#include "access_manager.h"
#include "cookie_jar.h"
#include "auto_complete.h"

#include <QSslConfiguration>

namespace network_service
{

NetworkAccessManager * getAccessManagerInstance()
{
    static NetworkAccessManager * instance = 0;
    if (instance == 0)
    {
        instance = new NetworkAccessManager();
        instance->setCookieJar(new CookieJar);
    }
    return instance;
}

NetworkAccessManager::NetworkAccessManager(QObject *parent, bool is_proxy)
    : QNetworkAccessManager(parent)
    , proxy_manager_(0)
    , proxy_exceptions_(0)
    , is_proxy_(is_proxy)
    , use_proxy_(false)
{
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
            SLOT(onAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
            SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
#ifndef QT_NO_OPENSSL
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
            SLOT(onSSLErrors(QNetworkReply*, const QList<QSslError>&)));
#endif
    loadSettings();

    QNetworkDiskCache *disk_cache = new QNetworkDiskCache(this);
    if (disk_cache != 0)
    {
        QString location = getCacheLocation();
        disk_cache->setCacheDirectory(location);
        setCache(disk_cache);
        if (proxy_manager_ != 0)
        {
            proxy_manager_->setCache(disk_cache);
        }
    }
}

void NetworkAccessManager::loadSettings()
{
    if (is_proxy_)
    {
        return;
    }

    if (proxy_manager_ != 0)
    {
        proxy_manager_.reset();
    }

    if (proxy_exceptions_ != 0)
    {
        proxy_exceptions_.reset();
    }

    QSettings settings;
    settings.beginGroup(QLatin1String("proxy"));
    QNetworkProxy proxy;
    if (settings.value(QLatin1String("enabled"), false).toBool())
    {
        if (settings.value(QLatin1String("type"), 0).toInt() == 0)
        {
            proxy = QNetworkProxy::Socks5Proxy;
        }
        else
        {
            proxy = QNetworkProxy::HttpProxy;
        }
        proxy.setHostName(settings.value(QLatin1String("hostName")).toString());
        proxy.setPort(settings.value(QLatin1String("port"), 1080).toInt());
        proxy.setUser(settings.value(QLatin1String("userName")).toString());
        proxy.setPassword(settings.value(QLatin1String("password")).toString());

        if (settings.value(QLatin1String("useExceptions"), false).toBool())
        {
            proxy_exceptions_.reset(new QStringList);
            QString exc = settings.value(QLatin1String("Exceptions")).toString();
            QStringList list = exc.split(QChar(';'));
            foreach(QString str, list)
            {
                *proxy_exceptions_ << str.trimmed();
            }
        }
        use_proxy_ = true;
        proxy_manager_.reset(new NetworkAccessManager(0, true));
        proxy_manager_->setCookieJar(new CookieJar);
        proxy_manager_->setProxy(proxy);
    }
    settings.endGroup();
    setProxy(proxy);
}

void NetworkAccessManager::onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    QString message = tr("Sign in for \"%1\" at %2");
    message = message.arg(Qt::escape(reply->url().toString())).arg(Qt::escape(reply->url().toString()));
    SignInDialog sign_in_dialog(0, message);
    if (sign_in_dialog.popup(QString(), QString()) != QDialog::Accepted)
    {
        return;
    }

    auth->setUser(sign_in_dialog.id());
    auth->setPassword(sign_in_dialog.password());
}

void NetworkAccessManager::clearCookies()
{
    CookieJar* cookie_jar = (CookieJar*)cookieJar();
    if (cookie_jar != 0)
    {
        cookie_jar->clear();
    }
}

void NetworkAccessManager::onProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth)
{
    QString message = tr("Connect to proxy \"%1\" using:");
    message = message.arg(Qt::escape(proxy.hostName()));
    SignInDialog sign_in_dialog(0, message);
    if (sign_in_dialog.popup(QString(), QString()) != QDialog::Accepted)
    {
        return;
    }

    auth->setUser(sign_in_dialog.id());
    auth->setPassword(sign_in_dialog.password());
}

#ifndef QT_NO_OPENSSL
void NetworkAccessManager::onSSLErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
#ifdef NEED_CONFIRM_SSL
    // check if SSL certificate has been trusted already
    QString reply_host = reply->url().host() + ":" + reply->url().port();
    if(!ssl_trusted_hosts_.contains(reply_host))
    {
        QStringList error_strings;
        for (int i = 0; i < error.count(); ++i)
        {
            error_strings += error.at(i).errorString();
        }
        QString errors = error_strings.join(QLatin1String("\n"));
        ui::MessageDialog ssl_message(QMessageBox::Information,
                                      tr("SSL Connection"),
                                      tr("SSL Errors:\n\n%1\n\n%2\n\n"
                                      "Do you want to ignore these errors for this host?").arg(reply->url().toString()).arg(errors),
                                      QMessageBox::Yes|QMessageBox::No);
        int ret = ssl_message.exec();
        if (ret == QMessageBox::Yes)
        {
            ssl_trusted_hosts_.append(reply_host);
            reply->ignoreSslErrors();
        }
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
    }
#else
    reply->ignoreSslErrors();
#endif
}
#endif

QNetworkReply * NetworkAccessManager::createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoing_data )
{
    if (use_proxy_ && proxy_manager_ && !isUrlProxyException(req.url()))
    {
        return proxy_manager_->createRequest(op, req, outgoing_data);
    }

    if (outgoing_data && (op == PostOperation))
    {
        QSettings settings;
        settings.beginGroup(QLatin1String("websettings"));
        bool need_save_password = settings.value(QLatin1String("savePasswords"), true).toBool();
        settings.endGroup();
        if (need_save_password)
        {
            data_ = outgoing_data->readAll();
            QUrl url = req.url();
            if (!req.rawHeader("Referer").isEmpty())
            {
                url = QUrl(req.rawHeader("Referer"));
            }

            // set data in request url
            AutoComplete::instance()->setFormData(url, data_);
            AutoComplete::instance()->evaluate(url);

            // set data in url of current frame
            emit requestSavePassword(data_);

            QBuffer* buf = new QBuffer(&data_, this);
            buf->open(QIODevice::ReadOnly);
            return QNetworkAccessManager::createRequest(op, req, buf);
        }
    }

    return QNetworkAccessManager::createRequest(op, req, outgoing_data);
}

bool NetworkAccessManager::isUrlProxyException(const QUrl& url)
{
    if (proxy_exceptions_ == 0)
    {
        return false;
    }

    QString host = url.host();
    QRegExp rx;
    rx.setCaseSensitivity( Qt::CaseInsensitive );
    rx.setPatternSyntax(QRegExp::Wildcard);

    foreach(QString ex, *proxy_exceptions_)
    {
        if (ex.length() > 0)
        {
            rx.setPattern(ex); 
            if (host.startsWith(ex, Qt::CaseInsensitive) || rx.exactMatch(host))
            {
                return true;
            }
        }
    }
    return false;
}

void NetworkAccessManager::enableSavingPassword(bool save)
{
    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
    settings.setValue(QLatin1String("savePasswords"), save);
    settings.endGroup();
}

bool NetworkAccessManager::savePassword()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
    bool need_save_password = settings.value(QLatin1String("savePasswords"), true).toBool();
    settings.endGroup();
    return need_save_password;
}

void NetworkAccessManager::enableAutoComplete(bool enable)
{
    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
    settings.setValue(QLatin1String("enableAutoComplete"), enable);
    settings.endGroup();
}

bool NetworkAccessManager::autoComplete()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
    bool enable_auto_complete = settings.value(QLatin1String("enableAutoComplete"), true).toBool();
    settings.endGroup();
    return enable_auto_complete;
}

}
