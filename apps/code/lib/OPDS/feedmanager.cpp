#include "feedmanager.h"
#include "downloadmanager.h"

#include <QByteArray>


QString get_root_url()
{
    return FeedManager::instance().baseUrl();
}

FeedManager::FeedManager(QObject *parent)
    : QObject(parent)
    , root_feed_(0)
{
    connect(this,SIGNAL(dumpFeed(const Feed*)),this,SLOT(onDumpFeed(const Feed *)));
    connect(&DownloadManager::instance(),SIGNAL(downloadFinished(QByteArray *, QString , bool)), this, SLOT(onDownloadFinished(QByteArray *, QString , bool)));
}

QString FeedManager::baseUrl()
{
    return root_url_;
}

Feed** FeedManager::findFeed(const QString & url, Feed* & parent)
{
    Feed **  feed = &root_feed_;
    if (feed && *feed)
    {
        feed = (**feed).findChildFeed(url,parent);
    }

    return feed;
}

void FeedManager::requestFeed(const QString & url)
{
    Feed * parent;
    Feed ** feed =  findFeed(url, parent);
    if (feed && *feed == 0)
    {
        DownloadManager::instance().append(url);
    }
    else if(feed)
    {
        emit dumpFeed(*feed);
    }
    else
    {
        qDebug("%s not exist",qPrintable(url));
    }

}

void FeedManager::onDownloadFinished(QByteArray * data, QString url, bool success)
{
    if (success)
    {
        if (!root_feed_)
        {
            root_url_ = url;
        }

        Feed * parent = 0;
        Feed ** feed = findFeed(url,parent);
        if (feed)
        {
            if (parent == 0 || parent->isNavigation())
            {
                *feed = new Feed;
                (**feed).parse(*data);
                emit dumpFeed(*feed);
            }
        }

    }

    delete data;
}

void FeedManager::onDumpFeed(const Feed* feed)
{
        feed->dump();

        qDebug("100. input url");
        int i = -1;
        scanf("%d",&i);

        QString url ;
        if (i == 100)
        {
            char buf[128];
            scanf("%s",buf);
            url = QString::fromLocal8Bit(buf);
        }
        else
        {
            url = feed->getLink(i);
        }

        requestFeed(url);
}
