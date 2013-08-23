
#include "image_model.h"

#include "onyx/base/base.h"
#include "image_item.h"
#include "image_tasks_handler.h"
#include "util/qstring_support.h"
#include "util/string_cast.h"

namespace image
{

static const unsigned int IMAGE_SIZE_LIMIT = 20 * 1024 * 1024;
static const unsigned int THUMB_SIZE_LIMIT = 10 * 1024 * 1024;

ImageModel::ImageModel(void)
  : render_policy_(new ImageRenderPolicy())
  , current_idx_(0)
  , image_count_(0)
{
#ifdef FIXED_MEM_CONTROL
    images_mgr_.SetMemLimit(IMAGE_SIZE_LIMIT);
    thumbs_mgr_.SetMemLimit(THUMB_SIZE_LIMIT);
#endif
    connect(this,
            SIGNAL(requestSaveAllOptions()),
            this,
            SLOT(onSaveModelOptions()));
}

ImageModel::~ImageModel()
{
    clear();
}

/// Estimate whether the queried document is this one. (Inherited from
/// BaseModel)
bool ImageModel::isTheDocument(const QString &path)
{
    return path_ == path;
}

/// Retrieve the thumbnail. We can use the metadata, but it needs
/// to scale the result image. The dedicated method is better.
QImage ImageModel::getThumbnail(const int width, const int height)
{
    return QImage();
}

/// Open a document by specified path. The path must be in UTF-8
bool ImageModel::open(const QString & path)
{
    return open(path, QString());
}

/// Open a document by specified path. The path must be in UTF-8
bool ImageModel::open(const QString & path, const QString & name)
{
    name_ = name;
    bool ret = reload(path);
    if (ret)
    {
        image_count_ = images_mgr_.itemCount();
        int init_page = getIndexOfInitImage();
        if (init_page < 0)
        {
            init_page = 0;
        }

        emit modelReadySignal(init_page);
    }
    return ret;
}

/// Retrieve the metadata.
bool ImageModel::metadata(const MetadataTag tag, QVariant &value)
{
    return false;
}

/// Close this document
bool ImageModel::close()
{
    emit modelClosingSignal();

    // clear all pending and running tasks
    ImageTasksHandler::instance().stopThread();
    ImageTasksHandler::instance().clearTasks();

    // save all of the accessing data of images
    ItemsManager<QString, ImageItem>::CacheMap & all_images = images_mgr_.images();
    foreach(shared_ptr<ImageItem> item, all_images)
    {
        if (item != 0)
        {
            item->saveAccessRecord(this);
        }
    }

    database_.close();
    clear();
    return true;
}

/// save all of the options related to this virtual document
bool ImageModel::save()
{
    emit requestSaveAllOptions();
    return true;
}

/// save model related options
void ImageModel::onSaveModelOptions()
{
    // save the file info of current image
    ImageKey current_key;
    if (getImageNameByIndex(current_idx_, current_key))
    {
        path_ = current_key;
    }
}

/// Set/get current image index
void ImageModel::setCurrentImage(const int idx)
{
    current_idx_ = idx;
    emit currentImageChangedSignal(current_idx_, image_count_);
}

/// Get the index by image name
int ImageModel::getIndexByImageName(const ImageKey &key)
{
    int idx = -1;
    shared_ptr<ImageItem> image = getImage(key);
    if (image.get())
    {
        idx = image->index();
    }
    return idx;
}

/// Get the key by index
bool ImageModel::getImageNameByIndex(const int idx, ImageKey &key)
{
    if (idx < 0 || idx >= entries_.size())
    {
        return false;
    }

    key = entries_[idx];
    return true;
}

/// Get the destination image by name
shared_ptr<ImageItem> ImageModel::getImage(const ImageKey &key)
{
    return images_mgr_.getImage(key);
}

/// get original size of a special page
bool ImageModel::getImageActualSize(int idx, int &width, int &height)
{
    shared_ptr<ImageItem> image = getImage(idx);
    if (image != 0)
    {
        if (image->image() == 0)
        {
            image->reload();
        }

        width = image->actualSize().width();
        height = image->actualSize().height();
        return true;
    }
    return false;
}

/// Reload all of the images by the document/directory path
bool ImageModel::reload(const QString &path)
{
    if (!openDatabase( path_, database_ ))
    {
        return false;
    }

    file_.setFile(path);
    dir_ = file_.dir();
    path_ = file_.absoluteFilePath();
    resetImages();

    // Clear all of the thumbnail images?
    thumbs_mgr_.clear();
    return true;
}

/// Clear all of the cached data
void ImageModel::clear()
{
    entries_.clear();
    images_mgr_.clear();
    thumbs_mgr_.clear();
}

bool ImageModel::isImage(const QString & path)
{
    static QList<QByteArray> formats = QImageReader::supportedImageFormats();
    int formats_num = formats.size();
    for (int k = 0; k < formats_num; ++k)
    {
        QByteArray format = formats.at(k);
        if (path.endsWith(QString(format), Qt::CaseInsensitive))
        {
            return true;
        }
    }
    return false;
}

void ImageModel::resetImages()
{
    invalidateAllImages();
    removeInvalidImages();

    QDir::Filters filters = QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot;
    QFileInfoList list = dir_.entryInfoList(filters);
    int img_idx = 0;
    for (int idx = 0; idx < list.size(); ++idx)
    {
        QFileInfo info = list.at(idx);
        QString path = info.absoluteFilePath();
        if (ImageModel::isImage(path))
        {
            // it is a valid image, make sure the corresponding image item exist
            makeImageItem(info, img_idx++);
            entries_.push_back(path);
        }
    }
}

void ImageModel::makeImageItem(const QFileInfo &info, const int index)
{
    ImageKey key = info.absoluteFilePath();

    shared_ptr<ImageItem> item = images_mgr_.getImage(key);
    if (item.get())
    {
        // TODO.the image exists, check the validation
        // For now, return directly
        item->setIndex(index);
        return;
    }

    // create a new empty image item
    item.reset(new ImageItem(key));
    item->setIndex(index);
    item->setFileSize(info.size());

    images_mgr_.addImage(item->name(), item);
    return;
}

/// Get image by index
shared_ptr<ImageItem> ImageModel::getImage(int index)
{
    if (index < 0 || index >= entries_.size())
    {
        return shared_ptr<ImageItem>();
    }

    return getImage(entries_[index]);
}

/// Get index of intialized image
int ImageModel::getIndexOfInitImage()
{
    QString name = file_.filePath();
    return getIndexByImageName(name);
}

/// Set sorting method
void ImageModel::setSorting(QDir::SortFlags flag)
{
    dir_.setSorting(flag);
    resetImages();
}

/// Add a new image by key name
shared_ptr<ImageItem> ImageModel::addImage(const ImageKey &key)
{
    shared_ptr<ImageItem> item(images_mgr_.getImage(key));
    if (item.get())
    {
        return item;
    }

    // create a new empty image item
    item.reset(new ImageItem(key));
    images_mgr_.addImage(item->name(), item);

    // TODO. add image key into entries list and other information
    return item;
}

/// Remove an existing image
bool ImageModel::removeImage(const ImageKey &key)
{
    return images_mgr_.removeImage(key);
}

void ImageModel::invalidateAllImages()
{
    images_mgr_.invalidateAll();
    entries_.clear();
}

void ImageModel::removeInvalidImages()
{
    images_mgr_.removeInvalid();
}

/// Handle the event of image ready
/// This function would be called when a render task finishes
void ImageModel::onImageReady(shared_ptr<ImageItem> image,
                              ImageStatus status,
                              bool notify)
{
    // notify the listener, such as vier, to display the page
    emit renderingImageReadySignal(image, status, notify);
}

/// Handle the event of thumbnail ready
void ImageModel::onThumbnailReady(shared_ptr<ImageThumbnail> thumb,
                                  const QRect& bounding_rect)
{
    thumb->recordCurrentTime();
    emit renderingThumbnailReadySignal(thumb, bounding_rect);
}

}
