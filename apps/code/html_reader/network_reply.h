

#ifndef READER_NETWORK_REPLY_H_
#define READER_NETWORK_REPLY_H_

#include <QtCore/QIODevice>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "model_interface.h"

namespace reader
{

/// Create customized network access manager in order to load resource from
/// different archive.
class ReaderNetworkManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    ReaderNetworkManager(ModelInterface *model);
    ~ReaderNetworkManager();

public:
    inline ModelInterface * model() { return model_; }

private Q_SLOTS:
    void replyFinished();

protected:
    virtual QNetworkReply * createRequest(Operation, const QNetworkRequest&, QIODevice * outgoingData = 0);

private:
    ModelInterface *model_;
};


/// Implement the QNetworkReply to load resource from customized archive.
class ReaderNetworkReply: public QNetworkReply
{
    Q_OBJECT

public:
    ReaderNetworkReply(ReaderNetworkManager *parent, const QNetworkRequest & request);
    ~ReaderNetworkReply();

public:
    virtual void abort();

    // reimplemented from QIODevice
    virtual void close();
    virtual bool isSequential() const;
    virtual qint64 bytesAvailable() const;

    QNetworkAccessManager *manager() const { return static_cast<QNetworkAccessManager *>(parent_); }

public Q_SLOTS:
    void startOperation();
    void onTimeout();

protected:
    virtual qint64 readData(char *data, qint64 maxlen);

private:
    mutable ReaderNetworkManager *parent_;
    int offset_;
    QByteArray data_;
    bool should_read_all_;
};

};  // namespace reader

#endif  // READER_NETWORK_REPLY_H_
