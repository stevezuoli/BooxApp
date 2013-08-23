
#include <algorithm>
#include <QtCore/QCoreApplication>
#include <QtCore/QtCore>
#include "network_reply.h"

namespace reader
{

ReaderNetworkManager::ReaderNetworkManager(ModelInterface *model)
: model_(model)
{
}

ReaderNetworkManager::~ReaderNetworkManager()
{
}

QNetworkReply * ReaderNetworkManager::createRequest(Operation op,
                                                    const QNetworkRequest & req,
                                                    QIODevice * outgoingData)
{
    // qDebug("request %s scheme %s", qPrintable(req.url().toString()), qPrintable(req.url().scheme()));
    if (req.url().scheme() != model_->scheme())
    {
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    }

    ReaderNetworkReply *reply = new ReaderNetworkReply(this, req);
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    QMetaObject::invokeMethod(reply, "startOperation", Qt::QueuedConnection);
    return reply;
}

void ReaderNetworkManager::replyFinished()
{
    QNetworkReply *r = dynamic_cast<QNetworkReply *>(sender());
    emit finished(r);
}

static const int BLOCK_SIZE = 1024 * 16;

ReaderNetworkReply::ReaderNetworkReply(ReaderNetworkManager *parent,
                                       const QNetworkRequest & request)
    : QNetworkReply(parent)
    , parent_(parent)
    , offset_(0)
    , should_read_all_(true)
{
    QNetworkReply::setOperation( QNetworkAccessManager::GetOperation );
    QNetworkReply::setRequest( request );
    QNetworkReply::setUrl(request.url());
    setOpenMode(QIODevice::ReadOnly);

    // Optimize for non-html data, like css and images.
    QString path = request.url().path();
    if (path.endsWith("html") || path.endsWith("htm"))
    {
        should_read_all_ = false;
    }

    if (!parent_->model()->load(url(), data_))
    {
        // Report an error
        qWarning("Could not load resource %s", qPrintable(request.url().toString()));
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 404);
        setError(ContentNotFoundError, tr("File not found."));
        emit error(ContentNotFoundError);
    }
    emit metaDataChanged();
}

ReaderNetworkReply::~ReaderNetworkReply()
{
}

void ReaderNetworkReply::abort()
{
    data_.clear();
    offset_ = 0;
}

void ReaderNetworkReply::close()
{
    data_.clear();
    offset_ = 0;
}

bool ReaderNetworkReply::isSequential() const
{
    // Not allowed to randomly access. Not sure yet.
    return true;
}

void ReaderNetworkReply::startOperation()
{
    // It's ready for reading now.
    emit readyRead();

    // Check the data length and request type.
    // If it's small or image, just read them all.
    if (data_.size() <= BLOCK_SIZE || should_read_all_)
    {
        emit finished();
    }
}

qint64 ReaderNetworkReply::bytesAvailable() const
{
    qint64 available = data_.size();

    if (available > BLOCK_SIZE && !should_read_all_)
    {
        available = BLOCK_SIZE;
    }
    return available + QIODevice::bytesAvailable();
}

void ReaderNetworkReply::onTimeout()
{
    emit readyRead();
}

qint64 ReaderNetworkReply::readData(char *data, qint64 maxlen)
{
    if (maxlen > data_.size())
    {
        maxlen = data_.size();
    }

    if (maxlen > BLOCK_SIZE && !should_read_all_)
    {
        maxlen = BLOCK_SIZE;
    }

    memcpy(data, data_.data(), maxlen);
    data_.remove(0, maxlen);

    if (data_.size() <= 0)
    {
        emit finished();
    }
    else if (!should_read_all_)
    {
        // About 300ms for screen update. Maybe should increase the value.
        QTimer::singleShot(1000, this, SLOT(onTimeout()));
    }
    return maxlen;
}

}   // namespace reader
