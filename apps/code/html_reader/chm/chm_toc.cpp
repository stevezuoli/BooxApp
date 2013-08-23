
#include "chm_toc.h"

static const int COLLAPSE = 0;
static const int EXPAND   = 1;
static const int LEAF     = 30;

static const int TOC_ROW         = Qt::UserRole + 1;
static const int TOC_PIXMAP_TYPE = Qt::UserRole + 2;

namespace chm
{

ChmTOC::ChmTOC()
: model_()
, chm_entries_()
{
}

ChmTOC::~ChmTOC()
{
    model_.clear();
}

bool ChmTOC::generateModel(LCHMFile &ref)
{
    if (!ref.parseTableOfContents(&chm_entries_))
    {
        qWarning("Could not parse table of content.");
        return false;
    }

    model_.clear();

    int index = 0;
    QStandardItem *parent = model_.invisibleRootItem();
    QVector<QStandardItem *> ptrs;
    foreach (const LCHMParsedEntry &entry, chm_entries_)
    {
        // Create a new standard item.
        QStandardItem *item = new QStandardItem(entry.name);
        item->setData(index);
        item->setEditable(false);
        ptrs.push_back(item);

        // Get parent.
        parent = searchParent(index, chm_entries_, ptrs, model_);
        parent->appendRow(item);
        ++index;
    }
    return true;
}

/// Get url from the model index.
/// Returns true if the index is valid. Otherwise it returns false.
bool ChmTOC::urlByIndex(const QModelIndex &idx,
                        QString & url)
{
    QStandardItem *item = model_.itemFromIndex(idx);
    int row = item->data().toInt();

    if (row < 0 || row >= chm_entries_.size())
    {
        return false;
    }
    LCHMParsedEntry entry = chm_entries_[row];

    if (entry.urls.size() <= 0)
    {
        return false;
    }

    url = entry.urls.at(0);
    return true;
}

/// Get index for the given url.
QModelIndex ChmTOC::indexByUrl(const QString & url)
{
    int pos = 0;
    for(; pos < chm_entries_.size(); ++pos)
    {
        LCHMParsedEntry entry = chm_entries_[pos];
        if (entry.urls.size() > 0 && entry.urls[0] == url)
        {
            break;
        }
    }

    if (pos >= chm_entries_.size())
    {
        return QModelIndex();
    }
    return searchByData(model_.invisibleRootItem(), pos);
}



QStandardItem * ChmTOC::searchParent(const int index,
                                     QVector<LCHMParsedEntry> & entries,
                                     QVector<QStandardItem *> & ptrs,
                                     QStandardItemModel &model)
{
    int indent = entries[index].indent;
    for(int i = index - 1; i >= 0; --i)
    {
        if (entries[i].indent < indent)
        {
            return ptrs[i];
        }
    }
    return model.invisibleRootItem();
}

QModelIndex ChmTOC::searchByData(QStandardItem *parent,
                                 const int data)
{
    int rows = parent->rowCount();
    for(int i = 0; i < rows; ++i)
    {
        QStandardItem *item = parent->child(i);
        if (item)
        {
            if (item->data() == data)
            {
                return item->index();
            }

            QModelIndex index = searchByData(item, data);
            if (index.isValid())
            {
                return index;
            }
        }
    }

    return QModelIndex();
}

}   // namespace chm
