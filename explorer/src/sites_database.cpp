#include "sites_database.h"
namespace explorer {

bool SitesDatabase::initialize() {
    if (!db_) {
        db_.reset (new QSqlDatabase (QSqlDatabase::addDatabase ("QSQLITE", "sites")));
        db_->setDatabaseName (QDir::home().filePath ("sites.db"));
    }
    return db_->open();
}

SitesDatabase::SitesDatabase() {
    initialize();
}

bool SitesDatabase::makeSureTableExist(QSqlDatabase& db) {
     QSqlQuery query(db);
     bool ok = query.exec("create table if not exists websites ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "name TEXT, "
                          "displayname TEXT, "
                          "url TEXT"
                          ");");
    return ok;
}

QVector<WebSite> SitesDatabase::readSites() {
    if (!makeSureTableExist (*db_)) {
        qDebug()<<" error on execute makeSureTableExist";
        return QVector<WebSite>();
    }
    QSqlQuery query(*db_);
    query.prepare("SELECT * FROM websites;");
    if (!query.exec()) {
        qDebug()<<"fail to select all from websites";
     }
    QVector<WebSite> websites;
    WebSite web;
    while (query.next()) {
       web.name = query.value(1).toString();
       web.displayname = query.value(2).toString();
       web.url = query.value(3).toString();
       websites.append(web);
    }
    return websites;
}

bool SitesDatabase::addSite(WebSite website) {
    QVector<WebSite> websites = readSites();
    if (websites.indexOf(website) == -1) {
        QSqlQuery query(*db_);
        // TODO check whether website is in the table
        if (!query.prepare("insert into websites (id, name, displayname, url) "
                      "values (:id, :name, :displayname, :url)")) {
            qDebug()<<"Fail here";
            return false;
        }
        query.bindValue(":name", website.name);
        query.bindValue(":displayname", website.displayname);
        query.bindValue(":url", website.url);
        if (!query.exec()) {
            return false;
        }
        return true;
    }
    return false;
}
bool SitesDatabase::deleteSite(const QString & name) {
        QSqlQuery query(*db_);
        
        if (!query.prepare("delete from websites where name = :name")) {
            qDebug()<<"Fail here";
            return false;
        }
        query.bindValue(":name", name);
        if (!query.exec()) {
            return false;
        }
        return true;
    
    return false;
}
void SitesDatabase::close() {
    if (db_) {
        db_->close();
        db_.reset(0);
        QSqlDatabase::removeDatabase("sites");
    }
}

}
