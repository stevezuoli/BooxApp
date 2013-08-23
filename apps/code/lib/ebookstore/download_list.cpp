
#include "download_list.h"

DownloadItem::DownloadItem(QNetworkReply *reply, const QString &path)
: path_(path)
, time_stamp_(QDateTime::currentDateTime())
, impl_(reply)
, received_(-1)
, total_(-1)
{
}

DownloadItem::~DownloadItem()
{
}

bool DownloadItem::progress(qint64 & received, qint64 total)
{
    received = received_;
    total = total_;
    return true;
}

bool DownloadItem::isSaveToFile()
{
    return !path_.isEmpty();
}

void DownloadItem::onProgressChanged(qint64 received, qint64 total)
{
    received_ = received;
    total_ = total;
    emit progressChanged(received_, total_);
    if (isSaveToFile())
    {
        // this->impl_->read()
    }
}

void DownloadItem::onError(QNetworkReply::NetworkError code)
{
}

void DownloadItem::onFinished()
{
    if (isSaveToFile())
    {
        emit downloadToFileFinished(path_);
    }
    else
    {
        buffer() = impl_->readAll();
        emit downloadFinished(buffer());
    }
}

QByteArray & DownloadItem::buffer()
{
    if (!buffer_)
    {
        buffer_.reset(new QByteArray());
    }
    return *buffer_;
}

QFile & DownloadItem::file()
{
    if (!file_)
    {
        file_.reset(new QFile());
    }
    return *file_;
}

DownloadList::DownloadList()
{
}

DownloadList::~DownloadList()
{
}

