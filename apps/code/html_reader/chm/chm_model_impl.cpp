
#include "chm_model_impl.h"

namespace chm
{

const QString ChmModelImpl::CHM_SCHEME = "chm";
static const int ROLE = Qt::UserRole+1;

/// Implement chm model.
ChmModelImpl::ChmModelImpl()
{
}

ChmModelImpl::~ChmModelImpl()
{
    close();
}

bool ChmModelImpl::open(const QString &path)
{
    close();

    if (!chm_file_.loadFile(path))
    {
        qDebug("Coud not open document %s", qPrintable(path));
        return false;
    }

    // Update.
    path_ = path;
    current_location_.clear();
    current_location_.setScheme(CHM_SCHEME);
    current_location_.setPath(chm_file_.homeUrl());
    return true;
}

bool ChmModelImpl::close()
{
    if (path_.isEmpty())
    {
        return false;
    }

    chm_file_.closeAll();
    path_.clear();
    return true;
}

QString ChmModelImpl::scheme()
{
    return CHM_SCHEME;
}

QUrl ChmModelImpl::home()
{
    QUrl url;
    url.setScheme(CHM_SCHEME);
    url.setPath(chm_file_.homeUrl());
    return url;
}

bool ChmModelImpl::load(QUrl url, QByteArray & data)
{
    if (!resolve(url))
    {
        return false;
    }

    current_location_.setPath(url.path());
    return chm_file_.getFileContentAsBinary(&data, current_location_.path());
}

bool ChmModelImpl::resolve(QUrl & url)
{
    if (url.isRelative())
    {
        url = current_location_.resolved(url);
        return true;
    }
    return true;
}

bool ChmModelImpl::supportTableOfContents()
{
    return true;
}

bool ChmModelImpl::tableOfContents(QStandardItemModel &model)
{
    QVector<LCHMParsedEntry> & all = entries();

    model.clear();

    int index = 0;
    QStandardItem *parent = model.invisibleRootItem();
    QVector<QStandardItem *> ptrs;
    foreach (const LCHMParsedEntry &entry, all)
    {
        // Create a new standard item.
        QStandardItem *item = new QStandardItem(entry.name);
        item->setData(index, ROLE);
        item->setEditable(false);
        ptrs.push_back(item);

        // Get parent.
        parent = searchParent(index, all, ptrs, model);
        parent->appendRow(item);
        ++index;
    }
    return true;
}

/// Get url from the specified model
bool ChmModelImpl::urlFromIndex(QStandardItemModel &toc,
                                const QModelIndex &selected,
                                QUrl & url)
{
    QVector<LCHMParsedEntry> & all = entries();
    if (all.size() <= 0)
    {
        return false;
    }

    int index = toc.data(selected, ROLE).toInt();
    if (index < 0 || index >= all.size())
    {
        return false;
    }

    const QStringList & urls = all.at(index).urls;
    if (urls.size() <= 0)
    {
        return false;
    }

    url.setScheme(CHM_SCHEME);
    url.setPath(urls.front());
    return true;
}

bool ChmModelImpl::indexFromUrl(QStandardItemModel &toc,
                                const QUrl & url,
                                QModelIndex &selected)
{
    QStandardItem *ptr = searchByUrl(toc.invisibleRootItem(), entries(), url);
    if (ptr == 0)
    {
        return false;
    }
    selected = ptr->index();
    return true;
}

/// Search in the toc to get the next link.
bool ChmModelImpl::nextPage(QUrl & url)
{
    QVector<LCHMParsedEntry> & all = entries();
    if (all.size() <= 0)
    {
        return false;
    }

    int index = 0;
    for(; index < all.size(); ++index)
    {
        const LCHMParsedEntry & entry = all.at(index);
        const QStringList & urls = entry.urls;
        if (urls.size() > 0 && urls.front() == url.path())
        {
            break;
        }
    }

    if (++index >= all.size())
    {
        return false;
    }

    const LCHMParsedEntry & entry = all.at(index);
    const QStringList & urls = entry.urls;
    if (urls.size() > 0)
    {
        url.setPath(urls.front());
        return true;
    }
    return false;
}

bool ChmModelImpl::prevPage(QUrl & url)
{
    QVector<LCHMParsedEntry> & all = entries();
    if (all.size() <= 0)
    {
        return false;
    }

    int index = 0;
    for(; index < all.size(); ++index)
    {
        const LCHMParsedEntry & entry = all.at(index);
        const QStringList & urls = entry.urls;
        if (urls.size() > 0 && urls.front() == url.path())
        {
            break;
        }
    }

    if (--index < 0)
    {
        return false;
    }

    const LCHMParsedEntry & entry = all.at(index);
    const QStringList & urls = entry.urls;
    if (urls.size() > 0)
    {
        url.setPath(urls.front());
        return true;
    }
    return false;
}

QStandardItem * ChmModelImpl::searchParent(const int index,
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

QModelIndex ChmModelImpl::searchByData(QStandardItem *parent,
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

QStandardItem * ChmModelImpl::searchByUrl(QStandardItem * parent,
                                          QVector<LCHMParsedEntry> & entries,
                                          const QUrl & url)
{
    if (parent == 0)
    {
        return 0;
    }

    bool ok = false;
    int index = parent->data(ROLE).toInt(&ok);
    if (ok && index >= 0 && index < entries.size())
    {
        const QStringList &urls = entries.at(index).urls;
        if (urls.size() > 0 && urls.front() == url.path())
        {
            return parent;
        }
    }

    int count = parent->rowCount();
    QStandardItem *ptr = 0;
    for(int i = 0; i < count; ++i)
    {
        ptr = searchByUrl(parent->child(i), entries, url);
        if (ptr)
        {
            return ptr;
        }
    }
    return ptr;
}

/// Return all chm entries.
QVector<LCHMParsedEntry> & ChmModelImpl::entries()
{
    if (chm_entries_.size() <= 0)
    {
        if (!chm_file_.parseTableOfContents(&chm_entries_))
        {
            qWarning("Could not parse table of contents.");
        }
    }
    return chm_entries_;
}

}   // namespace chm

