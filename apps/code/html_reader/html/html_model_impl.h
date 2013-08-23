
#ifndef HTML_PLUGIN_MODEL_IMPL_H_
#define HTML_PLUGIN_MODEL_IMPL_H_

#include <QtCore/QtCore>
#include "../model_interface.h"


namespace html
{

/// Implement html model.
class HtmlModelImpl : public reader::ModelInterface
{
public:
    HtmlModelImpl();
    ~HtmlModelImpl();

public:
    virtual bool open(const QString &path);
    virtual bool close();

    /// Return supported scheme.
    virtual QString scheme();

    /// Return the home url of archive.
    virtual QUrl home();

    /// Load the data according to the url.
    virtual bool load(QUrl url, QByteArray & data);

    virtual bool supportTableOfContents();
    virtual bool tableOfContents(QStandardItemModel &toc);
    virtual bool urlFromIndex(QStandardItemModel &toc, const QModelIndex &selected, QUrl & url);
    virtual bool indexFromUrl(QStandardItemModel &toc, const QUrl & url, QModelIndex &selected);

    virtual bool nextPage(QUrl & url);
    virtual bool prevPage(QUrl & url);

private:
    bool resolve(QUrl & url);

private:
    static const QString HTML_SCHEME;

    QString path_;                  ///< Document absolute path.
    QUrl current_location_;         ///< Current page absolute chm url.
};

}   // namespace html

#endif  // HTML_PLUGIN_MODEL_IMPL_H_
