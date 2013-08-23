#ifndef CHM_PLUGIN_TOC_H_
#define CHM_PLUGIN_TOC_H_

#include <QtGui/QtGui>
#include "libchmfile.h"


namespace chm
{

class ChmTOC
{
public:
    ChmTOC();
    ~ChmTOC();

public:
    inline QStandardItemModel* model() { return &model_; }
    bool generateModel(LCHMFile &ref);

    bool urlByIndex(const QModelIndex &index, QString &url);
    QModelIndex indexByUrl(const QString & url);

private:
    QStandardItem * searchParent(const int, QVector<LCHMParsedEntry> &,
                                 QVector<QStandardItem *> &,
                                 QStandardItemModel &);

    QModelIndex searchByData(QStandardItem * parent, const int data);

private:
    QStandardItemModel       model_;  ///< Model of toc for display on tree view
    QVector<LCHMParsedEntry> chm_entries_;    ///< Internal chm entries.
};

};  // namespace chm

#endif  // CHM_PLUGIN_TOC_H_
