#include "bookmark_model.h"

namespace webbrowser
{

BookmarkModel::BookmarkModel()
    : path_(QDir::homePath())
    , need_save_bookmarks_(false)
{
    QFileInfo file_info(path_, "web_browser");
    path_ = file_info.absoluteFilePath();
    loadBookmarks();
}

BookmarkModel::~BookmarkModel()
{
    saveBookmarks();
}

bool BookmarkModel::saveBookmarks()
{
    ContentManager database;
    if (!openDatabase(path_, database))
    {
        return false;
    }

    bool ret = true;
    if (need_save_bookmarks_)
    {
        ret = vbf::saveBookmarks(database, path_, bookmarks_);
    }

    if (ret)
    {
        need_save_bookmarks_ = false;
    }
    return ret;
}

bool BookmarkModel::loadBookmarks()
{
    ContentManager database;
    if (!openDatabase(path_, database))
    {
        return false;
    }

    need_save_bookmarks_ = false;
    bookmarks_.clear();
    return vbf::loadBookmarks(database, path_, bookmarks_);
}

struct LessByPosition
{
    bool operator()( const Bookmark& a, const Bookmark& b ) const
    {
        return a.title() < b.title();
    }
};

bool BookmarkModel::addBookmark(const QString & title, const QString & url)
{
    if (hasBookmark(title, url))
    {
        return false;
    }

    QString bookmark_title = title;
    if (bookmark_title.isEmpty())
    {
        // When the title is empty.
        QString format(QApplication::tr("Bookmark %1"));
        bookmark_title = format.arg(bookmarks_.size() + 1);
    }
    else
    {
        bookmark_title = bookmark_title.trimmed();
        if (bookmark_title.size() > 100)
        {
            bookmark_title = bookmark_title.left(97);
            bookmark_title.append("...");
        }
    }

    // Insert into bookmarks list.
    need_save_bookmarks_ = insertBookmark(bookmarks_, Bookmark(bookmark_title, url), LessByPosition());
    return need_save_bookmarks_;
}

bool BookmarkModel::deleteBookmark(const QString & title, const QString & url)
{
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        QString bookmark_url = iter->data().toString();
        if ( url == bookmark_url )
        {
            iter = bookmarks_.erase(iter);
            need_save_bookmarks_ = true;
            break;
        }
    }
    return need_save_bookmarks_;
}

bool BookmarkModel::hasBookmark(const QString & title, const QString & url)
{
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        QString bookmark_url = iter->data().toString();
        if ( url == bookmark_url )
        {
            return true;
        }
    }
    return false;
}

void BookmarkModel::getBookmarksModel(QStandardItemModel & bookmarks_model)
{
    bookmarks_model.setColumnCount(2);
    BookmarksIter begin = bookmarks_.begin();
    BookmarksIter end   = bookmarks_.end();
    int row = 0;
    for(BookmarksIter iter  = begin; iter != end; ++iter, ++row)
    {
        // title
        QStandardItem *title = new QStandardItem(iter->title());
        title->setData(iter->data());
        title->setEditable(false);
        bookmarks_model.setItem(row, 0, title);

        // url
        QString url_str = iter->data().toString();
        QStandardItem * url = new QStandardItem(url_str);
        url->setEditable(false);
        url->setTextAlignment(Qt::AlignCenter);
        bookmarks_model.setItem(row, 1, url);
    }

    bookmarks_model.setHeaderData(0, Qt::Horizontal, QVariant::fromValue(QApplication::tr("Title")), Qt::DisplayRole);
    bookmarks_model.setHeaderData(1, Qt::Horizontal, QVariant::fromValue(QApplication::tr("URL")), Qt::DisplayRole);
}

}
