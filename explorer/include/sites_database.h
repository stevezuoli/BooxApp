#ifndef SITES_DATABASE_H
#define SITES_DATABASE_H
#include "onyx/base/base.h"
#include <QtSql>
using namespace base;
namespace explorer
{

class WebSite {
public:
    QString name;
    QString displayname;
    QString url;
bool operator== (WebSite righthand){
    return displayname==righthand.displayname;
    }
};

class SitesDatabase
{
public:
    SitesDatabase();
    ~SitesDatabase() {
        close();
    }
    QVector<WebSite> readSites();
    bool addSite(WebSite website);
    bool deleteSite(const QString & name);
    void close();
private:
    bool initialize();
    bool makeSureTableExist (QSqlDatabase& db);
    scoped_ptr<QSqlDatabase> db_;
};
}
#endif // SITE_DATABASE_H
