#include "content.h"

static const QString TITLE_TAG = "ti";
static const QString AUTHOR_TAG = "au";
static const QString ISBN_TAG = "is";
static const QString COVER_TAG = "co";
static const QString TYPE_TAG = "ty";
static const QString TAGS_TAG = "ta";

OContent::OContent()
: QVariantMap()
{
}

OContent::~OContent()
{
}

QString OContent::title()
{
    return value(TITLE_TAG).toString();
}

void OContent::setTitle(const QString &title)
{
    insert(TITLE_TAG, title);
}

QString OContent::author()
{
     return value(AUTHOR_TAG).toString();
}

void OContent::setAuthor(const QString &autor)
{
    insert(AUTHOR_TAG, autor);
}

QString OContent::ISBN()
{
     return value(ISBN_TAG).toString();
}

void OContent::setISBN(const QString &isbn)
{
    insert(ISBN_TAG, isbn);
}

QImage OContent::cover()
{
     return qVariantValue<QImage>(value(COVER_TAG));
}

void OContent::setCover(const QImage &image)
{
    insert(COVER_TAG, image);
}

QStringList OContent::tags()
{
    return value(TAGS_TAG).toStringList();
}

void OContent::setTags(const QStringList &tags)
{
    insert(TAGS_TAG, tags);
}

int OContent::contentType()
{
    return value(TYPE_TAG).toInt();
}

bool OContent::hasChildren()
{
    return (children_.size() > 0);
}

QVector<OContent> &OContent::children()
{
    return children_;
}

