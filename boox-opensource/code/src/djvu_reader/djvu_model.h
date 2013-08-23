#ifndef DJVU_MODEL_H_
#define DJVU_MODEL_H_

#include "djvu_utils.h"
#include "djvu_source.h"

using namespace ui;
using namespace vbf;

namespace djvu_reader
{

class DjVuModel : public BaseModel
{
    Q_OBJECT
public:
    DjVuModel();
    virtual ~DjVuModel();

    inline DjVuSource* source() { return &source_; }
    bool save();

    // Load & Save configurations
    bool loadOptions();
    bool saveOptions();

    // Document loading and closing
    bool open(const QString & path);
    bool close();
    bool isReady() const { return is_ready_; }
    bool isTheDocument(const QString &path);

    // Retrieve the metadata.
    bool metadata(const MetadataTag tag, QVariant &value);
    QImage getThumbnail(const int width, const int height);
    Configuration & getConf() { return conf_; }

    // Basic information
    int firstPageNumber();
    int getPagesTotalNumber();
    const QString & path() const;

    // Bookmarks
    QString getPageText(int page_no);
    bool saveBookmarks();
    bool loadBookmarks();
    bool addBookmark(const int page_start, const int page_end);
    bool deleteBookmark(const int page_start, const int page_end);
    bool hasBookmark(const int page_start, const int page_end);
    bool updateBookmark(const int page_start, const int page_end, const QString & name);
    QString getFirstBookmarkTitle(const int page_start, const int page_end);
    void getBookmarksModel(QStandardItemModel & bookmarks_model);

    // TOC
    bool hasOutlines();
    QStandardItemModel* getOutlineModel();
    QString getDestByTOCIndex(const QModelIndex & index);

Q_SIGNALS:
    void docReady();
    void requestSaveAllOptions();

private:
    bool openCMS();
    void loadOutlineItem(QStandardItem * parent, const GPList<DjVmNav::DjVuBookMark> & items, GPosition & pos, int count);

private:
    // configuration
    cms::ContentManager content_manager_;
    Configuration       conf_;
    Bookmarks           bookmarks_;
    bool                need_save_bookmarks_;

    // document info
    bool                is_ready_;
    DjVuSource          source_;

    scoped_ptr<QStandardItemModel> outline_model_;
};

};

#endif // DJVU_MODEL_H_
