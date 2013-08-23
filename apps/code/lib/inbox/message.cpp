
#include "message.h"

namespace inbox
{

Message::Message()
{
}

Message::~Message()
{
}

id_long Message::id() const
{
    return id_;
}

const QString & Message::publisher() const
{
    return publisher_;
}

QString & Message::mutable_publisher()
{
    return publisher_;
}

const QString & Message::title() const
{
    return title_;
}

QString & Message::mutable_title()
{
    return title_;
}

const QString & Message::description() const
{
    return description_;
}

QString & Message::mutable_description()
{
    return description_;
}

const QDate & Message::timestamp() const
{
    return timestamp_;
}

QDate & Message::mutable_timestamp()
{
    return timestamp_;
}

const blob & Message::data() const
{
    return data_;
}

blob & Message::mutable_data() const
{
    return data_;
}

// Additional data for extension.
void Message::attributes(QVariantMap & att)
{
    QBuffer buffer(&attributes_);
    buffer.open(QIODevice::ReadOnly);
    QDataStream stream(&buffer);
    stream >> att;
}

void Message::setAttributes(QVariantMap & att)
{
    attributes_.clear();
    QBuffer buffer(&attributes_);
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);
    stream << att;
}

void Message::clear()
{
    publisher_.clear();
    title_.clear();
    description_.clear();
    attributes_.clear();
}


};
