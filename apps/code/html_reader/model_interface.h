
#ifndef MODEL_INTERFACE_H_
#define MODEL_INTERFACE_H_

#include <QtGui/QtGui>
#include <QtWebKit/QtWebKit>

namespace reader
{

/// Define interface for different model.
class ModelInterface
{
public:
    ModelInterface(){}
    virtual ~ModelInterface(){}

public:
    virtual bool open(const QString &path) = 0;
    virtual bool close() = 0;

    /// Return supported scheme.
    virtual QString scheme() = 0;

    /// Return the home url of archive.
    virtual QUrl home() = 0;

    /// Load the data according to the url.
    virtual bool load(QUrl url, QByteArray & data) = 0;

    /// Generate model of table of content.
    virtual bool supportTableOfContents() = 0;
    virtual bool tableOfContents(QStandardItemModel &toc) = 0;
    virtual bool urlFromIndex(QStandardItemModel &toc, const QModelIndex &selected, QUrl & url) = 0;
    virtual bool indexFromUrl(QStandardItemModel &toc, const QUrl & url, QModelIndex &selected) = 0;

    virtual bool nextPage(QUrl & url) = 0;
    virtual bool prevPage(QUrl & url) = 0;
};

}   // namespace reader

#endif  // MODEL_INTERFACE_H_
