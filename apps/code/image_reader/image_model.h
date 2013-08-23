#ifndef IMAGE_MODEL_H_
#define IMAGE_MODEL_H_

#include "image_utils.h"
#include "image_item.h"

#include "image_items_manager.h"
#include "image_render_policy.h"

using namespace onyx::ui;
using namespace vbf;

namespace image
{

class ImageModel : public BaseModel
{
    Q_OBJECT
public:
    ImageModel(void);
    ~ImageModel();

    inline int imageCount();
    inline const QString & directory();
    inline vbf::RenderPolicy* renderPolicy();
    inline const QString & initPath() { return path_; }
    inline const QString & name() { return name_; }
    cms::ContentManager & contentManager() { return database_; }
    Configuration & getConf() { return conf_; }

    bool open(const QString & path);
    bool open(const QString & path, const QString & name);
    bool close();
    bool save();

    bool isTheDocument(const QString &path);
    bool metadata(const MetadataTag tag, QVariant &value);
    QImage getThumbnail(const int width, const int height);

    int getIndexByImageName(const ImageKey &key);
    bool getImageNameByIndex(const int idx, ImageKey &key);
    void setCurrentImage(const int idx);
    inline int getCurrentImageIndex() {return current_idx_;}

    shared_ptr<ImageItem> getImage(int idx);
    shared_ptr<ImageItem> getImage(const ImageKey &key);
    shared_ptr<ImageItem> addImage(const ImageKey &key);
    bool getImageActualSize(int idx, int &width, int &height);

    /// Get the images manager
    inline ItemsManager<QString, ImageItem>& getImagesMgr() { return images_mgr_; }

    /// Get the thumbnails manager
    inline ItemsManager<QString, ImageThumbnail>& getThumbsMgr() { return thumbs_mgr_; }

    bool reload(const QString &path);
    void clear();
    int  getIndexOfInitImage();
    void setSorting(QDir::SortFlags flag);
    bool removeImage(const ImageKey &key);

    static bool isImage(const QString & path);

public Q_SLOTS:
    void onImageReady(shared_ptr<ImageItem> image, ImageStatus status, bool notify = false);
    void onThumbnailReady(shared_ptr<ImageThumbnail> thumb, const QRect& bounding_rect);
    void onSaveModelOptions();

Q_SIGNALS:
    /// document loading process is ready signal
    void modelReadySignal(const int init_page);

    /// model is going to be closed
    void modelClosingSignal();

    /// save all of the options
    void requestSaveAllOptions();

    /// current page changed signal
    void currentImageChangedSignal(const int, const int);

    /// rendering image is ready
    void renderingImageReadySignal(shared_ptr<ImageItem> image, ImageStatus status, bool update_screen);

    /// rendering thumbnail is ready
    void renderingThumbnailReadySignal(shared_ptr<BaseThumbnail> thumb,
                                       const QRect &bounding_rect);

private:
    typedef QVector<ImageKey> Entries;
    typedef Entries::iterator EntriesIter;

private:
    // items manager of all images
    ItemsManager<QString, ImageItem>      images_mgr_;
    ItemsManager<QString, ImageThumbnail> thumbs_mgr_;

    Entries entries_; // the entries table
    QFileInfo file_;  // the infomation of the intialized image file
    QDir dir_;        // the files information in current directory

    scoped_ptr<vbf::RenderPolicy> render_policy_;  // the policy of prerendering

    vbf::Configuration conf_;
    cms::ContentManager database_;
    int current_idx_;

    QString path_;
    QString name_;
    int image_count_;

private:
    void resetImages();
    void makeImageItem(const QFileInfo &info, const int index);
    void invalidateAllImages();
    void removeInvalidImages();
};

/// Get the total number of images
int ImageModel::imageCount()
{
    return image_count_;
}

/// Get the path of directory
const QString & ImageModel::directory()
{
    return path_;
}

/// Get the render policy
vbf::RenderPolicy* ImageModel::renderPolicy()
{
    return render_policy_.get();
}

};
#endif
