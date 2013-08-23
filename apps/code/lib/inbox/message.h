// Authors: John

#ifndef ONYX_MESSAGE_H__
#define ONYX_MESSAGE_H__


namespace inbox
{

typedef long long id_long;
typedef QByteArray blob;


class Message
{
public:
    Message();
    ~Message();

public:
    id_long id() const;

    const QString & publisher() const;
    QString & mutable_publisher();

    const QString & title() const;
    QString & mutable_title();

    const QString & description() const;
    QString & mutable_description();

    const QDate & timestamp() const;
    QDate & mutable_timestamp();

    const blob & data() const;
    blob & mutable_data() const;

    // Additional data for extension.
    void attributes(QVariantMap & attributes);
    void setAttributes(QVariantMap & attributes);

private:
    void clear();

    id_long & mutable_id() { return id_; }

    blob & mutable_attributes() { return attributes_; }
    const blob & attributes() const { return attributes_; }

private:
    id_long id_;
    QString publisher_;
    QString title_;
    QString description_;
    blob attributes_;
};

};

#endif  // ONYX_MESSAGE_H__
