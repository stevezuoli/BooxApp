#include "feed.h"
#include <QUrl>

QString get_root_url();

Feed::Feed(QObject *parent)
    : QObject(parent)
    , is_navigation_(true)
{
}


void Feed::parse(const QByteArray & data)
{
    QDomDocument doc; 
    doc.setContent(data,false);

    QDomElement root = doc.documentElement();

    QDomElement child = root.firstChildElement("entry");
    while (!child.isNull()) 
    {
        parseEntryElement(child);
        child = child.nextSiblingElement("entry");
    }

}

Feed** Feed::findChildFeed(const QString & url, Feed* & parent)
{
    //if (isNavigation())
    {
        Entry * entry;
        foreach(entry,entries_)
        {
            Link link;
            foreach(link,entry->links_)
            {
                //if (link.type_.startsWith("application/atom+xml") && link.href_ == url)
                if (link.href_ == url)
                {
                    parent = this;
                    return &entry->feed_;
                }

            }

        }

        //go down children
        foreach(entry,entries_)
        {
            if (entry->feed_)
            {
               Feed ** feed = entry->feed_->findChildFeed(url,parent);
               if (feed)
               {
                    return feed;
               }
            }
        }
    }

    return 0;
}

bool Feed::isNavigation() const
{
        return is_navigation_;
}

void Feed::parseEntryElement(const QDomElement &element)
{
    Entry * entry = new Entry;
    QDomElement child = element.firstChildElement();
    while(!child.isNull())
    {
        if (child.tagName() == "title") 
        {
            entry->title_ = child.text();
        } 
        else if (child.tagName() == "id") 
        {
            entry->id_ = child.text();
        }
        else if (child.tagName() == "updated") 
        {
            entry->updated_ = child.text();
        } 
        else if (child.tagName() == "link") 
        {
            Link link;
            if (child.hasAttribute("type"))
            {
                link.type_ = child.attribute("type");
            }
            if (child.hasAttribute("href"))
            {
                link.href_ = child.attribute("href");

                QUrl url(link.href_);
                if (url.isRelative())
                {
                    QUrl tmp(get_root_url());
                    link.href_ = tmp.resolved(url).toString();
                }
            }
            if (child.hasAttribute("rel"))
            {
                link.rel_ = child.attribute("rel");
            }
            if (link.rel_.startsWith("http://opds-spec.org/acquisition"))
            {
                 is_navigation_ = false;
            }
            entry->links_.push_back(link);
        } 
         

        child = child.nextSiblingElement();
    }

    entries_.push_back(entry);
}

void Feed::dump() const
{
    int i=0;
    Entry * entry;
    foreach(entry,entries_)
    {
        qDebug("%d %s",i++,qPrintable(entry->title_));
        qDebug("\t%s",qPrintable(entry->updated_));
        if (isNavigation())
        {
            Link link;
            foreach(link,entry->links_)
            {
                if (link.type_.startsWith("application/atom+xml"))
                {
                    qDebug("\t%s",qPrintable(link.href_));
                }
            }
        }
        else
        {
            Link link;
            foreach(link,entry->links_)
            {
                if (link.rel_.startsWith("http://opds-spec.org/acquisition"))
                {
                    qDebug("\t%s",qPrintable(link.href_));
                }
            }

        }
            
    }
}

QString Feed::getLink(int i) const
{
    if ( i >= 0 && i < entries_.size())
    {
        if (isNavigation())
        {
            return entries_[i]->links_.front().href_;
        }
        else
        {
            Link link;
            foreach(link,entries_[i]->links_)
            {
                if (link.rel_.startsWith("http://opds-spec.org/acquisition"))
                {
                    return link.href_;
                }
            }
        }
    }

    qWarning("can not find index %d", i);

    return QString();
}
