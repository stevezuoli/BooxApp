#ifndef WEB_ONYX_DB_H_
#define WEB_ONYX_DB_H_

#include <QtSql/QtSql>
#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "onyx/ui/ui.h"

using namespace ui;

namespace webapp
{

class OnyxDB : public QObject
{
    Q_OBJECT
public:
    OnyxDB(QObject *parent = 0);
    ~OnyxDB();

public Q_SLOTS:
    bool open(const QString & name);
    bool openOrCreate(const QString & name);
    bool insert(const QString & table, const QVariantMap & values, const QVariantMap & conditions);
    QVariantList select(const QString & table, const QVariantMap & conditions, const QVariantMap & options);
    QVariantList exec(const QString & table, const QString & query_str);

private:
    bool makeSureTableExist(QSqlDatabase &db, const QString & table, const QVariantMap & values);

private:
    scoped_ptr<QSqlDatabase> database_;    ///< sqlite qt wrapper.
    QString                  db_name_;     ///< Connection name of the db
};

};   // namespace webapp

#endif
