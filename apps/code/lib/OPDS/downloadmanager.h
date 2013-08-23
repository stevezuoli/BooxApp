#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QFile>
#include <QObject>
#include <QQueue>
#include <QMap>
#include <QTime>
#include <QUrl>
#include <QByteArray>
#include <QNetworkAccessManager>

class DownloadManager: public QObject
{
    Q_OBJECT
private:
    explicit DownloadManager(QObject *parent = 0);
    DownloadManager(const DownloadManager &);

public:
    static DownloadManager & instance()
    {
        static  DownloadManager download_manager;
        return download_manager;
    }

    void append(const QUrl &url);
    void append(const QStringList &urlList);
    QString saveFileName(const QUrl &url);

signals:
    void downloadFinished(QByteArray *, QString , bool);
    void finished();

private slots:
    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();

private:
    QNetworkAccessManager manager_;
    QQueue<QUrl> downloadQueue_;
    QNetworkReply *currentDownload_;
    QTime downloadTime_;

    int downloadedCount_;
    int totalCount_;

    QByteArray * data_;
    QString    url_;
    QMap<QString, QString> url_redirect_;
};

#endif
