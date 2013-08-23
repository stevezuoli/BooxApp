#include "webapp_onyx_db.h"

namespace webapp
{

static QString getDatabasePath(const QString & name)
{
    QString db_name(name);

    // fill the name by postfix
    if (!db_name.endsWith(".db", Qt::CaseInsensitive))
    {
        db_name = db_name + ".db";
    }

    db_name = QDir::home().filePath(db_name);
    return db_name;
}

static QStringList getTableColumns(QSqlQuery & query, const QString & table)
{
    QStringList columns;
    query.prepare("select sql from sqlite_master where name = ?");
    query.addBindValue(table);
    QString names;
    if (query.exec())
    {
        while (query.next())
        {
            names = query.value(0).toString();
        }
    }
    if (names.isEmpty())
    {
        return columns;
    }

    int idx = names.indexOf("(");
    QString table_content = names.right(names.size() - idx - 1);
    idx = table_content.indexOf(")");
    table_content = table_content.left(idx);

    idx = table_content.indexOf(",");
    while (idx >= 0)
    {
        QString column_str = table_content.left(idx);
        int space_idx = column_str.indexOf(" ");
        if (space_idx >= 0)
        {
            column_str = column_str.left(space_idx);
        }
        columns.push_back(column_str);

        table_content = table_content.right(table_content.size() - idx - 1);
        idx = table_content.indexOf(",");
    }

    // retrieve last column
    if (!table_content.isEmpty())
    {
        QString column_str = table_content.left(idx);
        int space_idx = column_str.indexOf(" ");
        if (space_idx >= 0)
        {
            column_str = column_str.left(space_idx);
        }
        columns.push_back(column_str);
    }
    return columns;
}

static QString getSqlTypeByVariant(const QVariant & value)
{
    switch (value.type())
    {
    case QVariant::ByteArray:
        return "blob";
    case QVariant::Int:
        return "integer";
    case QVariant::String:
        return "text";
    case QVariant::Double:
        return "float";
    default:
        break;
    }
    return QString();
}

OnyxDB::OnyxDB(QObject *parent)
: QObject(parent)
, database_(0)
{
}

OnyxDB::~OnyxDB()
{
}

bool OnyxDB::open(const QString & name)
{
    if (database_ == 0)
    {
        return false;
    }

    QString db_name = getDatabasePath(name);
    if (db_name_ == db_name && database_->isOpen())
    {
        return true;
    }

    db_name_ = db_name;
    database_->setDatabaseName(db_name_);
    if (!database_->open())
    {
        qDebug() << database_->lastError().text();
        return false;
    }
    return true;
}

bool OnyxDB::openOrCreate(const QString & name)
{
    QString db_name = getDatabasePath(name);
    if (db_name_ == db_name)
    {
        return true;
    }

    db_name_ = db_name;
    database_.reset(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", db_name_)));
    if (database_ != 0)
    {
        database_->setDatabaseName(db_name_);
        if (!database_->open())
        {
            qDebug() << database_->lastError().text();
            return false;
        }
    }
    return true;
}

bool OnyxDB::insert(const QString & table, const QVariantMap & values, const QVariantMap & conditions)
{
    if (database_ == 0)
    {
        return false;
    }

    if (makeSureTableExist(*database_, table, values))
    {
        QSqlQuery query(*database_);

        QString query_str("INSERT OR REPLACE into %1 (");
        query_str = query_str.arg(table);

        // add items string
        QString item_str;
        int count = 0;
        QVariantMap::const_iterator idx = values.begin();
        for (; idx != values.end(); idx++)
        {
            item_str += idx.key();
            count++;
            if (count < values.size())
            {
                item_str += ",";
            }
        }
        item_str += ") values(";
        query_str += item_str;

        // add values string
        QString value_str;
        count = 0;
        for (int i = 0; i < values.size(); ++i)
        {
            value_str += "?";
            count++;
            if (count < values.size())
            {
                value_str += ", ";
            }
        }
        value_str += ")";
        query_str += value_str;

        query.prepare(query_str);
        idx = values.begin();
        for (; idx != values.end(); idx++)
        {
            query.addBindValue(idx.value());
        }
        return query.exec();
    }
    return false;
}

QVariantList OnyxDB::select(const QString & table, const QVariantMap & conditions, const QVariantMap & options)
{
    QVariantList values;
    if (database_ == 0)
    {
        return values;
    }

    // prepare query string
    QSqlQuery query(*database_);

    // get names of all items in table
    QStringList table_columns = getTableColumns(query, table);
    if (table_columns.isEmpty())
    {
        return values;
    }

    QString query_str("select * from %1 where ");
    query_str = query_str.arg(table);

    // conditions string
    QVariantMap::const_iterator idx = conditions.begin();
    int count = 0;
    for (; idx != conditions.end(); idx++)
    {
        QString condition_str("%1 = ?");
        condition_str = condition_str.arg(idx.key());
        count++;
        if (count < conditions.size())
        {
            condition_str += " and ";
        }
        query_str += condition_str;
    }

    // options string
    idx = options.begin();
    count = 0;
    for (; idx != options.end(); idx++)
    {
        QString option_str("%1 %2");
        option_str = option_str.arg(idx.key()).arg(idx.value().toInt());
        count++;
        if (count < options.size())
        {
            option_str += " ";
        }
        query_str += option_str;
    }

    query.prepare(query_str);
    idx = conditions.begin();
    for (; idx != conditions.end(); idx++)
    {
        query.addBindValue(idx.value());
    }

    if (!query.exec())
    {
        return values;
    }

    while (query.next())
    {
        QStringList::iterator column_idx = table_columns.begin();
        QVariantMap row;
        for (int i = 0; column_idx != table_columns.end(); column_idx++, i++)
        {
            row[*column_idx] = query.value(i);
        }
        values.push_back(row);
    }
    return values;
}

QVariantList OnyxDB::exec(const QString & table, const QString & query_str)
{
    QVariantList values;
    if (database_ == 0)
    {
        return values;
    }

    // prepare query string
    QSqlQuery query(*database_);

    // get names of all items in table
    QStringList table_columns = getTableColumns(query, table);
    if (table_columns.isEmpty())
    {
        return values;
    }

    query.prepare(query_str);

    // TODO. Bind values

    if (!query.exec())
    {
        return values;
    }

    while (query.next())
    {
        QStringList::iterator column_idx = table_columns.begin();
        QVariantMap row;
        for (int i = 0; column_idx != table_columns.end(); column_idx++, i++)
        {
            row[*column_idx] = query.value(i);
        }
        values.push_back(row);
    }
    return values;

}

bool OnyxDB::makeSureTableExist(QSqlDatabase &db, const QString & table, const QVariantMap & values)
{
    if (values.isEmpty())
    {
        return false;
    }

    QSqlQuery query(db);
    QString query_str("create table if not exists %1 (");
    query_str = query_str.arg(table);

    QString primary_key("%1 %2 primary key");
    QVariantMap::const_iterator idx = values.begin();
    primary_key = primary_key.arg(idx.key()).arg(getSqlTypeByVariant(idx.value()));
    if (values.size() > 1)
    {
        primary_key += ",";
    }
    query_str += primary_key;
    idx++;

    int count = 1;
    for (; idx != values.end(); idx++)
    {
        QString item_key("%1 %2");
        item_key = item_key.arg(idx.key()).arg(getSqlTypeByVariant(idx.value()));
        count++;
        if (count < values.size())
        {
            item_key += ",";
        }
        query_str += item_key;
    }
    query_str += ")";
    return query.exec(query_str);
}

}
