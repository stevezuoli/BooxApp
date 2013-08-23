
#ifndef ELISA_MESSAGE_H_
#define ELISA_MESSAGE_H_

#include "onyx/data/content.h"
#include <QtXml/QtXml>
#include <QHttp>

/// Message library used to talk with server directly.
/// Finally, we don't use IPC (DBus based) as we have to
/// use data stream between different processes.
/// The inside process library is much more easy to use.
class ElisaPlugin: public QObject
{
    Q_OBJECT
public:
    ElisaPlugin(const QString & host = QString());
    ~ElisaPlugin();

public:
    bool checkNewContent(bool check_all = false);
    bool confirmNewContent(const QStringList ids);
    bool updateAdobeId(QString & id);

private Q_SLOTS:
    void onRequestFinished( int id, bool error);

Q_SIGNALS:
    void newContentReady(content::Books & books);
    void newContentConfirmed();
    void adobeIdReady();

private:

#ifndef QT_NO_OPENSSL
    void onSslErrors(const QList<QSslError> & errors);
#endif

private:
    const QString & host() { return host_; }

private:
    QString host_;
    QHttp connection_;
    QMap<int, int> request_ids_;
};

#endif  // ELISA_MESSAGE_H_
