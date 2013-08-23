#ifndef BOOKMARK_MODEL_H
#define BOOKMARK_MODEL_H

#include "onyx/ui/ui.h"
#include "onyx/data/bookmark.h"
#include "onyx/data/configuration.h"

using namespace ui;
using namespace vbf;

namespace webbrowser
{

class BookmarkModel
{
public:
    BookmarkModel();
    ~BookmarkModel();

    bool saveBookmarks();
    bool loadBookmarks();
    bool addBookmark(const QString & title, const QString & url);
    bool deleteBookmark(const QString & title, const QString & url);
    bool hasBookmark(const QString & title, const QString & url);
    void getBookmarksModel(QStandardItemModel & bookmarks_model);

private:
    QString             path_;
    Bookmarks           bookmarks_;
    bool                need_save_bookmarks_;
};

};

#endif
