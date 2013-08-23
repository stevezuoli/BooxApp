#include "onyx/data/user_behavior.h"
#include "onyx/data/data_tags.h"

namespace onyx
{

namespace data
{

UserBehavior::UserBehavior()
{
}

UserBehavior::UserBehavior(const UserBehavior &source)
    : app_name_(source.app_name_)
    , doc_path_(source.doc_path_)
    , page_(source.page_)
    , type_(source.type_)
    , time_stamp_(source.time_stamp_)
{
}

UserBehavior::UserBehavior(const QString &name, const QString &path, quint32 page,
            UserBehaviorType type, const QTime &time)
    : app_name_(name)
    , doc_path_(path)
    , page_(page)
    , type_(type)
    , time_stamp_(time)
{
}

UserBehavior::~UserBehavior()
{
}

QDataStream & operator<< ( QDataStream & out, const UserBehavior & behavior )
{
    out << behavior.appName();
    out << behavior.docPath();
    out << behavior.page();
    out << behavior.type();
    out << behavior.timeStamp();
    return out;
}

QDataStream & operator>> ( QDataStream & in, UserBehavior & behavior )
{
    in >> behavior.mutableAppName();
    in >> behavior.mutableDocPath();
    in >> behavior.mutablePage();
    in >> behavior.mutableType();
    in >> behavior.mutableTimeStamp();
    return in;
}

bool serialize(const UserBehavior &behavior, QByteArray &data)
{
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);
    stream << behavior;
    return true;
}

bool deserialize(const QByteArray & data, UserBehavior &behavior)
{
    QByteArray copy(data);
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    QDataStream stream(&buffer);
    stream >> behavior;
    return true;
}

}   // namespace data

}   // namespace onyx
