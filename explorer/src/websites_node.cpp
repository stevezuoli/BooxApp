#include <algorithm>
#include <map>
#include "util/util.h"
#include "websites_node.h"
#include "sites_database.h"
#include "onyx/data/web_history.h"

namespace explorer {

namespace model {


WebSitesNode::WebSitesNode(Node * p)
    : BranchNode(p)
{
    //changeSortCriteria(LAST_ACCESS_TIME, DESCENDING);
}

WebSitesNode::~WebSitesNode()
{
}

NodePtrs& WebSitesNode::updateChildrenInfo()
{
    updateChildren();
    return children_;
}

bool WebSitesNode::removeWebSite(const QString &name)
{
    QVariantList site_list;
    webhistory::WebHistory db;
    db.loadConf(site_list);
    for(int i = 0; i < site_list.size(); ++i)
    {
        webhistory::ThumbnailItem site(site_list.at(i).toMap());
        if (name.compare(site.title(),Qt::CaseInsensitive) == 0)
        {
            site_list.removeAt(i);
            db.saveConf(site_list);
            break;
        }
    }

    SitesDatabase sitedb;
    sitedb.deleteSite(name);

    QFile::remove(QString("/usr/share/explorer/images/small/").append(name).append(".png"));
    QFile::remove(QString("/usr/share/explorer/images/middle/").append(name).append(".png"));
    QFile::remove(QString("/usr/share/explorer/images/big/").append(name).append(".png"));

    return true;
}
bool WebSitesNode::addWebSite(const QString & str_url)
{
    QUrl url(str_url);
    if (!url.isValid())
    {
        return false;
    }
    QString host = url.host();
    if (host.isEmpty())
    {
        host = url.toString();
    }

    WebSite web = {host,host,url.toString()};
    SitesDatabase db;
    db.addSite(web);

    return true;
}

void WebSitesNode::clearAll()
{
    util::DeletePtrContainer(&children_);
}

void WebSitesNode::updateChildren()
{
    // TODO: need a better way. We have to
    // hardcode these sites in application so far.
    clearAll();
    SitesDatabase db;
    QVector<WebSite> websites = db.readSites();

    int size = websites.size();
    if (size > 0)
    {
        WebSite website;
        for (int i = 0; i < size; ++i)
        {
            website = websites.at (i);
            Node *newNode = new Node (this);
            newNode->mutable_type() = NODE_TYPE_WEBSITE;
            newNode->mutable_name() = website.name;
            newNode->mutable_display_name() = website.displayname;
            newNode->mutable_absolute_path() = website.url;
            mutable_children().push_back (newNode);
        }
    }
    
    if (!node("onyx"))
    {
        Node *onyx = new Node (this);
        onyx->mutable_type() = NODE_TYPE_WEBSITE;
        onyx->mutable_name() = "onyx";
        onyx->mutable_display_name() = "onyx";
        onyx->mutable_absolute_path() = "http://www.onyx-international.com";
        mutable_children().push_back (onyx);
    }
    if (!node("google"))
    {
        Node *google = new Node (this);
        google->mutable_type() = NODE_TYPE_WEBSITE;
        google->mutable_name() = "google";
        google->mutable_display_name() = "google";
        google->mutable_absolute_path() = "http://www.google.com";
        mutable_children().push_back (google);
    }
    if (!node("wiki"))
    {
        Node *wiki = new Node(this);
        wiki->mutable_type() = NODE_TYPE_WEBSITE;
        wiki->mutable_name() = "wiki";
        wiki->mutable_display_name() = "wiki";
        wiki->mutable_absolute_path() = "http://en.wikipedia.org/wiki/Wiki";
        mutable_children().push_back(wiki);
    }
    if (!node("adobe"))
    {
        Node *adobe = new Node(this);
        adobe->mutable_type() = NODE_TYPE_WEBSITE;
        adobe->mutable_name() = "adobe";
        adobe->mutable_display_name() = "Adobe";
        adobe->mutable_absolute_path() = "http://www.adobe.com";
        mutable_children().push_back(adobe);
    }


    QVariantList site_list_;
    webhistory::WebHistory webdb;
    webdb.loadConf(site_list_);
    for(int i = 0; i < site_list_.size(); ++i)
    {
        webhistory::ThumbnailItem site(site_list_.at(i).toMap());
        QString title = site.title().toLower();
        QString url   = site.url().toString();
        if (title.contains("host not found")) continue;
        if (title.contains("connecting")) continue;

        if((url.startsWith("http",Qt::CaseInsensitive) || url.startsWith("ftp",Qt::CaseInsensitive) || url.startsWith("www",Qt::CaseInsensitive)) && !node(title) )
        {
            Node *webpage= new Node (this);
            webpage->mutable_type() = NODE_TYPE_WEBSITE;
            webpage->mutable_name() = title;
            webpage->mutable_display_name() = title;
            webpage->mutable_absolute_path() = url;
            mutable_children().push_back (webpage);
        }
    }
}

}  // namespace model

}  // namespace explorer
