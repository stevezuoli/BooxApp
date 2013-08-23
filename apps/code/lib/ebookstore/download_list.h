#ifndef ONYX_DOWNLOAD_LIST_H_
#define ONYX_DOWNLOAD_LIST_H_

#include "onyx/base/base.h"
#include <QtNetwork/QtNetwork>

class DownloadItem : public QObject
{
    Q_OBJECT
public:
    DownloadItem(QNetworkReply *reply, const QString &path = QString());
    ~DownloadItem();

public:
    bool progress(qint64 & received, qint64 total);
    bool isSaveToFile();

Q_SIGNALS:
    void progressChanged(qint64 received, qint64 total);
    void downloadFinished(QByteArray & data);
    void downloadToFileFinished(const QString &path);

private Q_SLOTS:
    void onProgressChanged(qint64 received, qint64 total);
    void onError(QNetworkReply::NetworkError code);
    void onFinished();

private:
    QByteArray & buffer();
    QFile & file();

private:
    QString path_;
    QDateTime time_stamp_;
    QNetworkReply *impl_;
    qint64 received_;
    qint64 total_;
    scoped_ptr<QFile> file_;
    scoped_ptr<QByteArray> buffer_;
};

class DownloadList : public QObject
{
    Q_OBJECT

public:
    DownloadList();
    ~DownloadList();

private:
    QVector<DownloadItem *> list_;
};


#endif
