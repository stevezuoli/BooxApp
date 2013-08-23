#ifndef NETWORK_ACCESS_MANAGER_H
#define NETWORK_ACCESS_MANAGER_H

#include "ns_utils.h"

namespace network_service
{

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    NetworkAccessManager(QObject *parent = 0, bool is_proxy = false);

    void enableSavingPassword(bool save);
    bool savePassword();

    void enableAutoComplete(bool enable);
    bool autoComplete();

    void clearCookies();

protected:
    QNetworkReply * createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoing_data = 0 );
    bool isProxy() {return is_proxy_; }
    bool useProxy() {return use_proxy_; }
    bool isUrlProxyException(const QUrl&);

Q_SIGNALS:
    void requestSavePassword(const QByteArray & data);

public Q_SLOTS:
    void loadSettings();

private Q_SLOTS:
    void onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *auth);
    void onProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth);
#ifndef QT_NO_OPENSSL
    void onSSLErrors(QNetworkReply *reply, const QList<QSslError> &error);
#endif

private:
#ifdef NEED_CONFIRM_SSL
    QList<QString>                   ssl_trusted_hosts_;
#endif
    scoped_ptr<NetworkAccessManager> proxy_manager_;
    scoped_ptr<QStringList>          proxy_exceptions_;
    QByteArray                       data_;
    bool is_proxy_;
    bool use_proxy_;
};

NetworkAccessManager * getAccessManagerInstance();
};

#endif
