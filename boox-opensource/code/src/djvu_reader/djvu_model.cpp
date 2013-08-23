#include "djvu_model.h"

namespace djvu_reader
{

DjVuModel::DjVuModel()
    : BaseModel()
    , need_save_bookmarks_(false)
    , is_ready_(false)
    , source_()
{
}

DjVuModel::~DjVuModel()
{
    close();
}

bool DjVuModel::open(const QString & path)
{
    if (!path.isEmpty())
    {
        if (source_.open(path))
        {
            is_ready_ = true;
            loadOptions();
            emit docReady();
            return true;
        }
    }
    return false;
}

bool DjVuModel::close()
{
    if (!is_ready_)
    {
        qDebug("The model has been closed!");
        return true;
    }

    conf_.options.clear();

    // reset flag
    is_ready_ = false;
    return true;
}

bool DjVuModel::save()
{
    if (!isReady())
    {
        return false;
    }

    emit requestSaveAllOptions();
    saveOptions();
    return true;
}

bool DjVuModel::openCMS()
{
    if (!content_manager_.isOpen())
    {
        vbf::openDatabase(source_.fileName(), content_manager_);
    }
    return content_manager_.isOpen();
}

bool DjVuModel::loadOptions()
{
    bool ret = openCMS();
    if (ret)
    {
        ret = vbf::loadDocumentOptions(content_manager_, source_.fileName(), conf_);
    }

    ret = loadBookmarks();
    return ret;
}

bool DjVuModel::saveOptions()
{
    bool ret = openCMS();
    if (ret)
    {
        ret = vbf::saveDocumentOptions(content_manager_, source_.fileName(), conf_);
    }

    ret = saveBookmarks();
    return ret;
}

bool DjVuModel::isTheDocument(const QString &path)
{
    return ( source_.fileName() == path );
}

bool DjVuModel::metadata(const MetadataTag tag, QVariant &value)
{
    return false;
}

QImage DjVuModel::getThumbnail(const int width, const int height)
{
    return QImage();
}

int DjVuModel::firstPageNumber()
{
    return 0;
}

int DjVuModel::getPagesTotalNumber()
{
    return source_.getPageCount();
}

bool DjVuModel::saveBookmarks()
{
    bool ret = true;
    if (need_save_bookmarks_)
    {
        ret = vbf::saveBookmarks(content_manager_, path(), bookmarks_);
    }
    if (ret)
    {
        need_save_bookmarks_ = false;
    }
    return ret;
}

bool DjVuModel::loadBookmarks()
{
    need_save_bookmarks_ = false;
    bookmarks_.clear();
    return vbf::loadBookmarks(content_manager_, path(), bookmarks_);
}

QString DjVuModel::getPageText(int page_no)
{
    // TODO. Implement Me
    return QString();
}

struct LessByPosition
{
    bool operator()( const Bookmark& a, const Bookmark& b ) const
    {
        return a.data().toInt() < b.data().toInt();
    }
};

bool DjVuModel::addBookmark(const int page_start, const int page_end)
{
    if (hasBookmark(page_start, page_end))
    {
        return false;
    }

    QString title = getPageText(page_start);
    if (title.isEmpty())
    {
        // When the title is empty.
        QString format(tr("Bookmark %1"));
        title = format.arg(bookmarks_.size() + 1);
    }
    else
    {
        title = title.trimmed();
        if (title.size() > 100)
        {
            title = title.left(97);
            title.append("...");
        }
    }

    // Insert into bookmarks list.
    need_save_bookmarks_ = insertBookmark( bookmarks_,
                                           Bookmark( title, page_start ),
                                           LessByPosition());
    return need_save_bookmarks_;
}

bool DjVuModel::deleteBookmark(const int page_start, const int page_end)
{
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        int position = iter->data().toInt();
        if (page_start <= position && position <= page_end)
        {
            iter = bookmarks_.erase(iter);
            need_save_bookmarks_ = true;
            break;
        }
    }
    return need_save_bookmarks_;
}

bool DjVuModel::hasBookmark(const int page_start, const int page_end)
{
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        int position = iter->data().toInt();
        if (page_start <= position && position <= page_end)
        {
            return true;
        }
    }
    return false;
}

bool DjVuModel::updateBookmark(const int page_start, const int page_end, const QString & name)
{
    if (name.isEmpty())
    {
        return false;
    }

    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        int position = iter->data().toInt();
        if (page_start <= position && position <= page_end)
        {
            (*iter).mutable_title() = name;
            need_save_bookmarks_ = true;
            break;
        }
    }
    return need_save_bookmarks_;
}

QString DjVuModel::getFirstBookmarkTitle(const int page_start, const int page_end)
{
    QString title;
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        int position = iter->data().toInt();
        if (page_start <= position && position <= page_end)
        {
            title = iter->title();
            break;
        }
    }
    return title;
}

void DjVuModel::getBookmarksModel(QStandardItemModel & bookmarks_model)
{
    bookmarks_model.setColumnCount(2);
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    int row = 0;
    for(BookmarksIter iter  = begin; iter != end; ++iter, ++row)
    {
        // page number
        int loc = iter->data().toInt();
        if (loc >= 0)
        {
            // title
            QStandardItem *title = new QStandardItem( iter->title() );
            title->setData( iter->data() );
            title->setEditable( false );
            bookmarks_model.setItem( row, 0, title );

            // page number
            int page_number = loc + 1;
            QString str( tr("%1") );
            str = str.arg(page_number);
            QStandardItem *page = new QStandardItem(str);
            page->setEditable(false);
            page->setTextAlignment(Qt::AlignCenter);
            bookmarks_model.setItem(row, 1, page);
        }
    }

    bookmarks_model.setHeaderData(0, Qt::Horizontal, QVariant::fromValue(tr("Title")), Qt::DisplayRole);
    bookmarks_model.setHeaderData(1, Qt::Horizontal, QVariant::fromValue(tr("Page")), Qt::DisplayRole);
}

bool DjVuModel::hasOutlines()
{
    return source_.getContents()->getBookMarkList().size() > 0;
}

QStandardItemModel* DjVuModel::getOutlineModel()
{
    if (outline_model_ != 0)
    {
        return outline_model_.get();
    }

    if (hasOutlines())
    {
        const GPList<DjVmNav::DjVuBookMark>& items = source_.getContents()->getBookMarkList();
        GPosition pos = items;
        outline_model_.reset( new QStandardItemModel() );
        QStandardItem *root = outline_model_->invisibleRootItem();
        loadOutlineItem(root, items, pos, items.size());

        // set header data
        outline_model_->setHeaderData(0, Qt::Horizontal, QVariant::fromValue(tr("Title")), Qt::DisplayRole);
        outline_model_->setHeaderData(1, Qt::Horizontal, QVariant::fromValue(tr("Page")), Qt::DisplayRole);
        return outline_model_.get();
    }
    return 0;
}

void DjVuModel::loadOutlineItem(QStandardItem * parent,
                                const GPList<DjVmNav::DjVuBookMark> & items,
                                GPosition & pos,
                                int count)
{
    for (int i = 0; i < count && !!pos; ++i)
    {
        const GP<DjVmNav::DjVuBookMark> bm = items[pos];

        QString title = QString::fromUtf8(bm->displayname.getbuf());
        QString dest  = QString::fromUtf8(bm->url.getbuf());
        //if (dest.isEmpty() || ((dest.at(0) == QLatin1Char('#')) && (dest.remove(0, 1) != title)))
        if (!title.isEmpty())
        {
            if (!dest.isEmpty() && dest.at(0) == QLatin1Char('#'))
            {
                dest.remove(0, 1);
            }
            QStandardItem *model_item = new QStandardItem(title);
            model_item->setData(dest, OUTLINE_ITEM);

            QStandardItem *page_item = new QStandardItem(dest);
            page_item->setTextAlignment( Qt::AlignCenter );
            page_item->setData(dest, OUTLINE_ITEM);

            int row_count = parent->rowCount();
            parent->appendRow( model_item );
            if (page_item != 0)
            {
                parent->setChild( row_count, 1, page_item );
            }
        }
        loadOutlineItem(parent, items, ++pos, bm->count);
    }
}

QString DjVuModel::getDestByTOCIndex(const QModelIndex & index)
{
    if (outline_model_ == 0)
    {
        return QString();
    }

    QStandardItem *item = outline_model_->itemFromIndex( index );
    QString dest = item->data(OUTLINE_ITEM).toString();
    return dest;
}

const QString & DjVuModel::path() const
{
    return source_.fileName();
}

}
