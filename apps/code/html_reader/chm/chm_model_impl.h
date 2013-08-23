
#ifndef CHM_PLUGIN_MODEL_IMPL_H_
#define CHM_PLUGIN_MODEL_IMPL_H_

#include "../model_interface.h"
#include "libchmfile.h"

namespace chm
{

/// Implement chm model.
class ChmModelImpl : public reader::ModelInterface
{
public:
    ChmModelImpl();
    ~ChmModelImpl();

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
    QStandardItem * searchParent(const int, QVector<LCHMParsedEntry> &,
                                 QVector<QStandardItem *> &,
                                 QStandardItemModel &);

    QModelIndex searchByData(QStandardItem * parent, const int data);
    QStandardItem *searchByUrl(QStandardItem * parent, QVector<LCHMParsedEntry> &, const QUrl & url);

    QVector<LCHMParsedEntry> & entries();

private:
    static const QString CHM_SCHEME;

private:
    QString path_;                  ///< Document absolute path.
    LCHMFile chm_file_;
    QUrl current_location_;         ///< Current page absolute chm url.
    QVector<LCHMParsedEntry> chm_entries_;
};

}   // namespace chm

#endif  // CHM_PLUGIN_MODEL_IMPL_H_
