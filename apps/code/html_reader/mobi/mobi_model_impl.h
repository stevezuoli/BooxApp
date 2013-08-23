#ifndef PLUGIN_MOBI_MODEL_H_
#define PLUGIN_MOBI_MODEL_H_


#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QUrl>

#include "onyx/base/base.h"
#include "palm_db/mobi_stream.h"
#include "../model_interface.h"

using namespace pdb;

namespace mobipocket
{

class MobiModelImpl : public reader::ModelInterface
{
public:
    MobiModelImpl();
    ~MobiModelImpl();

public:
    virtual bool open(const QString &path);
    virtual bool close();

    /// Return supported scheme.
    virtual QString scheme();

    /// Return the home url of archive.
    virtual QUrl home();

    /// Load the data according to the url.
    virtual bool load(QUrl url, QByteArray & data);

    /// Generate model of table of content.
    virtual bool supportTableOfContents();
    virtual bool tableOfContents(QStandardItemModel &toc);
    virtual bool urlFromIndex(QStandardItemModel &toc, const QModelIndex &selected, QUrl & url);
    virtual bool indexFromUrl(QStandardItemModel &toc, const QUrl & url, QModelIndex &selected);

    virtual bool nextPage(QUrl & url);
    virtual bool prevPage(QUrl & url);

private:
    void PreParse(char*& raw_html, size_t& size);

private:
    QString path_;
    scoped_ptr<MobiStream> stream_;
};      // mobipocket

};
#endif // PLUGIN_MOBI_MODEL_H_
