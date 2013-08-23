#include "naboo_model.h"

using namespace ui;
using namespace vbf;
using namespace adobe_view;

namespace naboo_reader
{

NabooModel::NabooModel()
    : BaseModel()
    , need_save_bookmarks_(false)
{
    connect(&document_, SIGNAL(documentReadySignal()), this, SLOT(onDocumentReady()));
}

NabooModel::~NabooModel(void)
{
}

bool NabooModel::open(const QString & path)
{
    if (document_.open(path) && document_.isReady())
    {
        // intialize the annotations manager
        annotations_mgr_.attachDocument(&document_);
        return true;
    }
    return false;
}

bool NabooModel::isTheDocument(const QString &path)
{
    return document_.isTheDocument(path);
}

bool NabooModel::metadata(const MetadataTag tag, QVariant &value)
{
    return false;
}

/// Retrieve the thumbnail. We can use the metadata, but it needs
/// to scale the result image. The dedicated method is better.
QImage NabooModel::getThumbnail(const int width, const int height)
{
    return QImage();
}

Configuration & NabooModel::getConf()
{
    return documentClient()->getConf();
}

/// Close this document
bool NabooModel::close()
{
    annotations_mgr_.close();
    return document_.close();
}

/// save all of the options used in application
bool NabooModel::save()
{
    if (!document_.isReady())
    {
        return false;
    }

    emit requestSaveAllOptions();
    saveModelOptions();
    return true;
}

void NabooModel::saveModelOptions()
{
    // save all of the dirty annotations
    annotations_mgr_.save();
    if (document_.saveOptions())
    {
        saveBookmarks();
    }
}

void NabooModel::getBookmarksModel(QStandardItemModel & bookmarks_model)
{
    bookmarks_model.setColumnCount(2);
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    int row = 0;
    for(BookmarksIter iter  = begin; iter != end; ++iter, ++row)
    {
        // page number
        AdobeLocationPtr loc = iter->data().value<AdobeLocationPtr>();
        if ( loc->getData() != 0 )
        {
            // title
            QStandardItem *title = new QStandardItem( iter->title() );
            title->setData( iter->data() );
            title->setEditable( false );
            bookmarks_model.setItem( row, 0, title );

            // page number
            int page_number = static_cast<int>(loc->getPagePosition());
            QString str( tr("%1") );
            str = str.arg( page_number + 1 );
            QStandardItem *page = new QStandardItem(str);
            page->setEditable(false);
            page->setTextAlignment(Qt::AlignCenter);
            bookmarks_model.setItem(row, 1, page);
        }
    }

    bookmarks_model.setHeaderData(0, Qt::Horizontal, QVariant::fromValue(tr("Title")), Qt::DisplayRole);
    bookmarks_model.setHeaderData(1, Qt::Horizontal, QVariant::fromValue(tr("Page")), Qt::DisplayRole);
}

bool NabooModel::loadOptions()
{
    bool ret = document_.loadOptions();
    if (ret)
    {
        ret = loadBookmarks();
    }
    return ret;
}

bool NabooModel::saveBookmarks()
{
    bool ret = true;
    if (need_save_bookmarks_)
    {
        // transfer bookmarks list
        Bookmarks raw_bookmarks;
        BookmarksIter begin = bookmarks_.begin();
        BookmarksIter end   = bookmarks_.end();
        for(BookmarksIter iter  = begin; iter != end; ++iter)
        {
            AdobeLocationPtr position = iter->data().value<AdobeLocationPtr>();
            QByteArray raw_str = position->getBookmark().toUtf8();
            if ( !raw_str.isEmpty() )
            {
                Bookmark raw_bookmark(iter->title(), raw_str);
                raw_bookmarks.push_back(raw_bookmark);
            }
        }

        ret = vbf::saveBookmarks(document_.database(), document_.path(), raw_bookmarks);
    }

    if (ret)
    {
        need_save_bookmarks_ = false;
    }
    return ret;
}

bool NabooModel::loadBookmarks()
{
    need_save_bookmarks_ = false;
    Bookmarks raw_bookmarks;
    bool ret = vbf::loadBookmarks(document_.database(), document_.path(), raw_bookmarks);
    if ( ret )
    {
        bookmarks_.clear();

        // transfer bookmarks list
        BookmarksIter begin = raw_bookmarks.begin();
        BookmarksIter end   = raw_bookmarks.end();
        for(BookmarksIter iter  = begin; iter != end; ++iter)
        {
            QByteArray raw_str = iter->data().toByteArray();
            AdobeLocationPtr loc = document_.getLocationFromBookmark( raw_str.constData() );
            if ( loc != 0 )
            {
                Bookmark bookmark( iter->title(), QVariant::fromValue( loc ) );
                bookmarks_.push_back( bookmark );
            }
        }
    }
    return ret;
}

struct LessByPosition
{
    bool operator()( const Bookmark& a, const Bookmark& b ) const
    {
        return a.data().value<AdobeLocationPtr>() < b.data().value<AdobeLocationPtr>();
    }
};

bool NabooModel::addBookmark(const Range & screen_range)
{
    if (!screen_range.isValid() || hasBookmark(screen_range))
    {
        return false;
    }

    QString title = document_.getText(screen_range);
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
                                           Bookmark( title, QVariant::fromValue( screen_range.start ) ),
                                           LessByPosition());
    return need_save_bookmarks_;
}

bool NabooModel::deleteBookmark(const Range & screen_range)
{
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        AdobeLocationPtr position = iter->data().value<AdobeLocationPtr>();
        if ( screen_range.start <= position && position < screen_range.end )
        {
            iter = bookmarks_.erase(iter);
            need_save_bookmarks_ = true;
            break;
        }
    }
    return need_save_bookmarks_;
}

bool NabooModel::hasBookmark(const Range & screen_range)
{
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        AdobeLocationPtr position = iter->data().value<AdobeLocationPtr>();
        if ( screen_range.start <= position && position < screen_range.end )
        {
            return true;
        }
    }
    return false;
}

void NabooModel::onDocumentReady()
{
    loadOptions();
}

}  // namespace naboo_reader
