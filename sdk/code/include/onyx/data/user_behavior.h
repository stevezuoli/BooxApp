#ifndef USER_BEHAVIOR_H_
#define USER_BEHAVIOR_H_

#include <QtCore/QtCore>

namespace onyx
{

namespace data
{



class UserBehavior
{
public:
    enum UserBehaviorType
    {
        PAGE_TURNING = 1,

    };

    UserBehavior();
    UserBehavior(const UserBehavior &source);
    UserBehavior(const QString &name, const QString &path, quint32 page,
            UserBehaviorType type, const QTime &time);
    ~UserBehavior();

public:
    const QString & appName() const { return app_name_; }
    QString & mutableAppName() { return app_name_; }

    const QString & docPath() const { return doc_path_; }
    QString & mutableDocPath() { return doc_path_; }

    quint32 page() const { return page_; }
    quint32 & mutablePage() { return page_; }

    int type() const { return type_; }
    int & mutableType() { return type_; }

    const QTime & timeStamp() const { return time_stamp_; }
    QTime & mutableTimeStamp() { return time_stamp_; }

private:
    QString app_name_;
    QString doc_path_;
    quint32 page_;
    int type_;
    QTime time_stamp_;

};

QDataStream & operator<< ( QDataStream & out, const UserBehavior & behavior );
QDataStream & operator>> ( QDataStream & in, UserBehavior & behavior );

bool serialize(const UserBehavior &behavior, QByteArray &data);
bool deserialize(const QByteArray & data, UserBehavior &behavior);

}   // namespace data

}   // namespace onyx

#endif  // USER_BEHAVIOR_H_
