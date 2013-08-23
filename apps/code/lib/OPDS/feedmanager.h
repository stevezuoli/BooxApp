#ifndef FEED_MANAGER_H_
#define FEED_MANAGER_H_

#include "feed.h"

class FeedManager: public QObject
{
    Q_OBJECT
public:
        static FeedManager & instance()
        {
            static FeedManager feed_manager(0);
            return feed_manager;
        }

        QString baseUrl();
        void requestFeed(const QString & url);
        Feed** findFeed(const QString & url, Feed* & parent);

signals:
        void dumpFeed(const Feed* feed);

private slots:
        void onDownloadFinished(QByteArray * data, QString url, bool success);
        void onDumpFeed(const Feed* feed);

private: 
        explicit FeedManager(QObject *parent = 0);
        FeedManager(const FeedManager &) {};

private:
     QString root_url_;        
     Feed *  root_feed_;
};

#endif
