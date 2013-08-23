
#include "downloadmanager.h"

#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <stdio.h>

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
    , downloadedCount_(0)
    , totalCount_(0)
    , data_(0)
{
}

void DownloadManager::append(const QStringList &urlList)
{
    foreach (QString url, urlList)
    {
        append(QUrl::fromEncoded(url.toLocal8Bit()));
    }

    if (downloadQueue_.isEmpty())
    {
        QTimer::singleShot(0, this, SIGNAL(finished()));
    }
}

void DownloadManager::append(const QUrl &url)
{
    if (downloadQueue_.isEmpty())
    {
        QTimer::singleShot(0, this, SLOT(startNextDownload()));
    }

    downloadQueue_.enqueue(url);
    ++totalCount_;
}

QString DownloadManager::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if (basename.isEmpty())
    {
        basename = "download";
    }

    if (QFile::exists(basename)) 
    {
        // already exists, don't overwrite
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i)))
        {
            ++i;
        }

        basename += QString::number(i);
    }

    basename.prepend("/home/zyf/bookstore/");
    return basename;
}

void DownloadManager::startNextDownload()
{
    if (downloadQueue_.isEmpty()) 
    {
        qDebug("%d/%d files downloaded successfully\n", downloadedCount_, totalCount_);
        emit finished();
        return;
    }

    QUrl url = downloadQueue_.dequeue();

    url_ = url.toString();
    data_ = new QByteArray;
    //qDebug("download url:%s schema:%s isRelative:%d valid:%d",qPrintable(url.toString()),qPrintable(url.scheme()),url.isRelative(),url.isValid());


    QNetworkRequest request(url);
    request.setRawHeader("user-agent", "FBReader/0.12.10");
    request.setRawHeader("Accept", "*/*");
    currentDownload_ = manager_.get(request);
    connect(currentDownload_, SIGNAL(downloadProgress(qint64,qint64)), SLOT(downloadProgress(qint64,qint64)));
    connect(currentDownload_, SIGNAL(finished()), SLOT(downloadFinished()));
    connect(currentDownload_, SIGNAL(readyRead()), SLOT(downloadReadyRead()));

    // prepare the output
    qDebug("Downloading %s...\n", url.toEncoded().constData());
    downloadTime_.start();
}

void DownloadManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    // calculate the download speed
    double speed = bytesReceived * 1000.0 / downloadTime_.elapsed();
    QString unit;
    if (speed < 1024) 
    {
        unit = "bytes/sec";
    }
    else if (speed < 1024*1024) 
    {
        speed /= 1024;
        unit = "kB/s";
    }
    else 
    {
        speed /= 1024*1024;
        unit = "MB/s";
    }

    //qDebug("\rRecieved:%lld/%lld speed:%.1f %s",bytesReceived,bytesTotal,speed,qPrintable(unit));
    printf("\rRecieved:%lld/%lld speed:%.1f %s          ",bytesReceived,bytesTotal,speed,qPrintable(unit));
    if (bytesReceived == bytesTotal)
    {
        printf("\n");
    }
}

void DownloadManager::downloadFinished()
{
/*
    QList<QByteArray> headerList = currentDownload_->rawHeaderList ();
    QByteArray header;
    foreach(header, headerList)
    {
        qDebug("%s:%s",header.constData(),currentDownload_->rawHeader(header).constData());
    }
*/

    if (currentDownload_->error()) 
    {
        // download failed
        qDebug("Failed: %s\n", qPrintable(currentDownload_->errorString()));
        emit  downloadFinished(data_, url_, false);
    }
    else 
    {
        qDebug("Succeeded.\n");
        ++downloadedCount_;

        //test if redirect
        QVariant var = currentDownload_->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (var.isValid())
        {
           QUrl toUrl = var.toUrl(); 
           //TODO avoid death loop
           QString fromUrl = url_redirect_.value(url_);
           if (fromUrl == toUrl.toString())
           {
                qWarning("dead loop %s",qPrintable(fromUrl));
           }
           else
           {
               append(toUrl);
               url_redirect_.insert(toUrl.toString(),url_);        

           }
           qDebug("redirect to %s", qPrintable(toUrl.toString()));
        }
        else
        {
            QUrl url(url_);
            QString filename = saveFileName(url);

            QFile output;
            output.setFileName(filename);
            if (!output.open(QIODevice::WriteOnly)) 
            {
                qDebug("Problem opening save file '%s' for download '%s': %s\n", qPrintable(filename), url.toEncoded().constData(), qPrintable(output.errorString()));

            }
            else
            {
                output.write(*data_);
                output.close();
                qDebug("save to file:%s\n",qPrintable(filename));
            }


            while (url_redirect_.find(url_) != url_redirect_.end())
            {
                QString tmp = url_redirect_.value(url_);
                url_redirect_.remove(url_);
                url_ = tmp;
            }

            emit  downloadFinished(data_, url_, true);
        }
    }

    currentDownload_->deleteLater();
    startNextDownload();
}

void DownloadManager::downloadReadyRead()
{
   data_->append(currentDownload_->readAll());
}
