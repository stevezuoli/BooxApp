#include "dm_manager.h"
#include "access_manager.h"
#include <QWebSettings>

namespace network_service
{

static const char * CONTENT_DISPOSITION = "content-disposition";

DownloadManager * getDownloadManagerInstance()
{
    static DownloadManager * instance = 0;
    if (instance == 0)
    {
        instance = new DownloadManager();
    }
    return instance;
}

/*!
    DownloadManager is a Dialog that contains a list of DownloadItems

    It is a basic download manager.  It only downloads the file, doesn't do BitTorrent,
    extract zipped files or anything fancy.
  */
DownloadManager::DownloadManager(QWidget *parent)
    : auto_saver_(new AutoSaver(this))
    , manager_(getAccessManagerInstance())
    , remove_policy_(SuccessFullDownload)
{
    model_ = new DownloadModel(this);
    //load();
}

DownloadManager::~DownloadManager()
{
    auto_saver_->changeOccurred();
    auto_saver_->saveIfNeccessary();
}

int DownloadManager::activeDownloads() const
{
    int count = 0;
    for (int i = 0; i < downloads_.count(); ++i)
    {
        if (downloads_.at(i)->downloading())
        {
            ++count;
        }
    }
    return count;
}

void DownloadManager::download(const QNetworkRequest &request, bool requestFileName)
{
    if (request.url().isEmpty())
    {
        return;
    }
    handleUnsupportedContent(manager_->get(request), requestFileName);
}

static QString getFileNameByPattern(const QByteArray & content_disposition, const QString & pattern)
{
    QRegExp rx(pattern);
    QString file_name;
    int pos = rx.indexIn(content_disposition);
    if ( pos >= 0 )
    {
        file_name = rx.cap(1);
        file_name = QString::fromLocal8Bit(QByteArray::fromPercentEncoding(file_name.toLocal8Bit()));

        qDebug("Raw File Name:%s", file_name.toLocal8Bit().constData());
        file_name = file_name.trimmed();
        if (file_name.startsWith(QLatin1Char('"')) && file_name.endsWith(QLatin1Char('"')))
        {
            file_name = file_name.mid(1, file_name.size() - 2);
            qDebug("Last File Name:%s", file_name.toLocal8Bit().constData());
        }
    }
    return file_name;
}

void DownloadManager::handleUnsupportedContent(QNetworkReply *reply, bool requestFileName)
{
    if (!reply || reply->url().isEmpty())
    {
        return;
    }

    QVariant header = reply->header(QNetworkRequest::ContentLengthHeader);
    bool ok;
    int size = header.toInt(&ok);
    if (ok && size == 0)
    {
        return;
    }

    // content-disposition
    QString file_name;
    if (reply->hasRawHeader(CONTENT_DISPOSITION))
    {
        QByteArray content_disposition = reply->rawHeader(CONTENT_DISPOSITION);
        if (content_disposition.contains("filename"))
        {
            QString pattern1 = QString("filename=") + QString('"') + QString("(.*)") + QString('"');
            file_name = getFileNameByPattern(content_disposition, pattern1);
            if (file_name.isEmpty())
            {
                QString pattern2 = QString("filename=") + QString("(.*)");
                file_name = getFileNameByPattern(content_disposition, pattern2);
            }
        }
    }

    QNetworkReply *r = getAccessManagerInstance()->get(QNetworkRequest(reply->url()));
    reply->deleteLater();

    qDebug("Reply URL:%s, File Name:%s", r->url().toString().toUtf8().constData(), file_name.toLocal8Bit().constData());
    DownloadItem *item = new DownloadItem(r, requestFileName, 0, file_name);
    addItem(item);
}

void DownloadManager::addItem(DownloadItem *item)
{
    connect(item, SIGNAL(statusChanged()), this, SLOT(updateColumn()));
    int column = downloads_.count();
    model_->beginInsertColumns(QModelIndex(), column, column);
    downloads_.append(item);
    model_->endInsertColumns();
    updateItemCount();

    emit itemAdded(item);
}

bool DownloadManager::stillDownloading() const
{
    foreach (DownloadItem* item, downloads_)
    {
        if (item->downloading())
        {
            return true;
        }
    }
    return false;
}

void DownloadManager::updateColumn()
{
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    int column = downloads_.indexOf(item);
    if (-1 == column)
    {
        return;
    }

    bool remove = false;
    QWebSettings *global_settings = QWebSettings::globalSettings();
    if (!item->downloading()
        && global_settings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        remove = true;
    }

    if (item->downloadedSuccessfully()
        && removePolicy() == DownloadManager::SuccessFullDownload)
    {
        remove = true;
    }
    if (remove)
    {
        model_->removeColumn(column);
    }

}

DownloadManager::RemovePolicy DownloadManager::removePolicy() const
{
    return remove_policy_;
}

void DownloadManager::setRemovePolicy(RemovePolicy policy)
{
    if (policy == remove_policy_)
    {
        return;
    }

    remove_policy_ = policy;
    auto_saver_->changeOccurred();
}

void DownloadManager::save() const
{
    QSettings settings;
    settings.beginGroup(QLatin1String("downloadmanager"));
    QMetaEnum remove_policy_enum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("RemovePolicy"));
    settings.setValue(QLatin1String("removeDownloadsPolicy"), QLatin1String(remove_policy_enum.valueToKey(remove_policy_)));

    for (int i = 0; i < downloads_.count(); ++i)
    {
        QString key = QString(QLatin1String("download_%1_")).arg(i);
        settings.setValue(key + QLatin1String("url"), downloads_[i]->url_);
        settings.setValue(key + QLatin1String("location"), QFileInfo(downloads_[i]->output_).filePath());
        settings.setValue(key + QLatin1String("done"), downloads_[i]->downloadedSuccessfully());
    }

    int i = downloads_.count();
    QString key = QString(QLatin1String("download_%1_")).arg(i);
    while (settings.contains(key + QLatin1String("url")))
    {
        settings.remove(key + QLatin1String("url"));
        settings.remove(key + QLatin1String("location"));
        settings.remove(key + QLatin1String("done"));
        key = QString(QLatin1String("download_%1_")).arg(++i);
    }
}

void DownloadManager::load()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("downloadmanager"));

    QByteArray value = settings.value(QLatin1String("removeDownloadsPolicy"), QLatin1String("Never")).toByteArray();
    QMetaEnum remove_policy_enum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("RemovePolicy"));
    remove_policy_ = remove_policy_enum.keyToValue(value) == -1 ?
                        Never :
                        static_cast<RemovePolicy>(remove_policy_enum.keyToValue(value));

    int i = 0;
    QString key = QString(QLatin1String("download_%1_")).arg(i);
    while (settings.contains(key + QLatin1String("url")))
    {
        QUrl url = settings.value(key + QLatin1String("url")).toUrl();
        QString file_name = settings.value(key + QLatin1String("location")).toString();
        bool done = settings.value(key + QLatin1String("done"), true).toBool();
        if (!url.isEmpty() && !file_name.isEmpty())
        {
            DownloadItem *item = new DownloadItem();
            item->output_.setFileName(file_name);
            item->url_ = url;
            addItem(item);
        }
        key = QString(QLatin1String("download_%1_")).arg(++i);
    }
}

void DownloadManager::cleanup()
{
    if (downloads_.isEmpty())
    {
        return;
    }
    model_->removeColumns(0, downloads_.count());
    updateItemCount();
    auto_saver_->changeOccurred();
}

void DownloadManager::updateItemCount()
{
    int count = downloads_.count();
    qDebug("Item Count:%d", count);
}

}
