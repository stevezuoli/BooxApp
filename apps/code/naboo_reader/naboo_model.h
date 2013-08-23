#ifndef NABOO_MODEL_H_
#define NABOO_MODEL_H_

#include "naboo_utils.h"
#include "naboo_annotations_manager.h"

using namespace ui;
using namespace vbf;
using namespace adobe_view;

namespace naboo_reader
{

class NabooModel : public BaseModel
{
    Q_OBJECT
public:
    NabooModel();
    ~NabooModel();

    inline AnnotationManager* annotationMgr();
    inline AdobeDocumentClient* documentClient();
    inline const QString & path() const;
    inline bool isReady() const;
    inline bool errorFound() const;

    virtual bool isTheDocument(const QString &path);
    virtual bool metadata(const MetadataTag tag, QVariant &value);
    virtual QImage getThumbnail(const int width, const int height);
    virtual Configuration & getConf();

    // Open/Close
    bool open(const QString & path);
    bool close();
    bool save();
    bool loadOptions();

    // Bookmarks
    bool saveBookmarks();
    bool loadBookmarks();
    bool addBookmark(const Range & screen_range);
    bool deleteBookmark(const Range & screen_range);
    bool hasBookmark(const Range & screen_range);
    void getBookmarksModel(QStandardItemModel & bookmarks_model);

Q_SIGNALS:
    void requestSaveAllOptions();

private Q_SLOTS:
    void onDocumentReady();

private:
    void saveModelOptions();

private:
    AdobeDocumentClient document_;
    AnnotationManager   annotations_mgr_;
    Bookmarks           bookmarks_;
    bool                need_save_bookmarks_;
};

inline bool NabooModel::isReady() const
{
    return document_.isReady();
}

inline bool NabooModel::errorFound() const
{
    return document_.errorFound();
}

inline AnnotationManager* NabooModel::annotationMgr()
{
    return &annotations_mgr_;
}

inline AdobeDocumentClient* NabooModel::documentClient()
{
    return &document_;
}

inline const QString & NabooModel::path() const
{
    return document_.path();
}

};  // namespace naboo_view
#endif
