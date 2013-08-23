
#ifndef ONYX_CONTENT_H_
#define ONYX_CONTENT_H_

#include <QtCore/QtCore>
#include <QImage>


/// Onyx content. It can be an ebook or a catalog.
class OContent : public QVariantMap
{
public:
    OContent();
    ~OContent();

public:
    QString title();
    void setTitle(const QString &);

    QString author();
    void setAuthor(const QString &);

    QString ISBN();
    void setISBN(const QString &);

    QImage cover();
    void setCover(const QImage &);

    QStringList tags();
    void setTags(const QStringList &);

    int contentType();

    QString localCopyFileName();

    bool hasChildren();
    QVector<OContent> &children();

private:
    QVector<OContent> children_;        // Occupy 4 bytes.
};


#endif

