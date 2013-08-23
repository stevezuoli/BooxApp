#ifndef FEED_H_
#define FEED_H_

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QDomDocument>

/// replaced by ocontent
struct Dcmeta
{
    QString name_;
    QString value_;
};

struct Link
{
    QString rel_;
    QString href_;
    QString type_;
};


class Feed;

// feed entry
struct Entry
{
    Entry():feed_(0){};

    QString title_;
    QString id_;
    QString updated_;
    QString published_;
    QString summary_;
    QString authors_;
    QString contributors_;
    QString categories_;
    QString content_;
    QString rights_;

    QList<Dcmeta> dcmetas_;
    QList<Link> links_;

    Feed * feed_;
};


class 

class Feed: public QObject
{
    Q_OBJECT
public:
        explicit Feed(QObject *parent = 0);

        void parse(const QByteArray & data);
        void parseEntryElement(const QDomElement &element);

        Feed** findChildFeed(const QString & url, Feed* & parent);
        QString getLink(int i) const;

        bool isNavigation() const;
        void dump() const;

private:
        QList<Entry* >  entries_;
        bool is_navigation_;
};

#endif
