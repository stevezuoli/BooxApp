#ifndef IMAGE_ITEM_H_
#define IMAGE_ITEM_H_

#include "image_utils.h"
#include "image_thumbnail.h"

namespace image
{

/// The class RenderSetting contains all of the render settings of an image
class RenderSetting
{
public:
    RenderSetting()
        : content_area_()
        , rotation_(ROTATE_0_DEGREE) {}
    ~RenderSetting() {}

    RenderSetting(const RenderSetting &right)
    {
        *this = right;
    }

    RenderSetting& operator=(const RenderSetting &right)
    {
        if (*this == right)
        {
            return *this;
        }

        content_area_ = right.content_area_;
        rotation_ = right.rotation_;
        //zoom_setting_ = right.zoom_setting_;
        //zoom_value_ = right.zoom_value_;

        return *this;
    }

    bool operator==(const RenderSetting &right) const
    {
        return (content_area_ == right.content_area_ &&
            rotation_ == right.rotation_);
            //zoom_setting_ == right.zoom_setting_ &&
            //zoom_value_ == right.zoom_value_);
    }

    /// set the content area
    void setContentArea(const QRect &content_area)
    {content_area_.setSize(content_area.size());}
    const QRect& contentArea() const {return content_area_;}

    /// set rotation
    void setRotation(const RotateDegree r) { rotation_ = r; }
    RotateDegree rotation() const { return rotation_; }

    /// set zoom value
    //void setZoomValue(const ZoomFactor z) {zoom_value_ = z;}

private:
    // the rectangle of the render area
    // start point and size
    QRect content_area_;

    // the rotation degrees
    RotateDegree rotation_;

    // zoom setting
    //ZoomFactor zoom_setting_;

    // zoom value
    //ZoomFactor zoom_value_;
};

/// Private information of the image
struct PrivateInfo
{
    ImageKey       key;       /// key of the image
    QImage::Format format;    /// convert format
    int            index;     /// index of the image
    bool           dithered;  /// dithered
    bool           converted; /// converted
    bool           smoothed;  /// smoothed

    PrivateInfo()
        : key()
        , format(QImage::Format_RGB32)
        , index(-1)
        , dithered(false)
        , converted(false)
        , smoothed(false) {}
    ~PrivateInfo() {}

    void reset()
    {
        dithered = false;
        converted = false;
        format = QImage::Format_RGB32;
        smoothed = false;
    }
};

/// Configure Data
struct ConfigData
{
    bool          accessed;
    Configuration conf;

    ConfigData()
        : accessed(false)
        , conf() {}
};


class ImageModel;
class DitheringStrategy;
/// Base class of image item
class ImageItem
{
public:
    ImageItem(const ImageKey &k);
    ~ImageItem();

    /// get length of the image
    int length();

    /// Reload the page
    bool reload();

    /// Destroy the data
    void clearPage();

    /// Access the image. The accessing data would be recorded
    void access(ImageModel *model);
    void saveAccessRecord(ImageModel *model);

    /// Scaling(zoom)
    ImageStatus render(const RenderSetting &setting, ImageModel *model);

    /// Render a thumbnail image
    ImageStatus renderThumbnail(const QRect &bounding_rect,
                                QRect &display_area,
                                shared_ptr<ImageThumbnail> thumbnail,
                                ImageModel *model);

    /// Get data
    const QImage* image() const { return data_.get(); }

    /// Get some private data
    bool dithered() const { return private_info_.dithered; }
    bool converted() const { return private_info_.converted; }
    QImage::Format format() const { return private_info_.format; }
    bool smoothed() const { return private_info_.smoothed; }

    /// Get render settings
    RenderSetting& renderSetting() { return render_setting_; }

    /// Lock/unlock the image
    void unlock() {locked_ = false;}
    void lock() {locked_ = true;}
    bool locked() const {return locked_;}

    /// Set/get validation of image
    void invalidate() {valid_ = false;}
    void validate() {valid_ = true;}
    bool valid() {return valid_;}

    /// Set/get index of image
    void setIndex(const int idx) {private_info_.index = idx;}
    int index() const {return private_info_.index;}

    /// Set/get the key of the image
    void setKey(const ImageKey & key) {private_info_.key = key;}
    const ImageKey& name() const {return private_info_.key;};

    /// Get the actual size
    QSize &actualSize() {return actual_size_;}

    /// Set/get the file size
    void setFileSize(const unsigned int s) {file_size_ = s;}
    unsigned int fileSize() const {return file_size_;}

    /// Get/Set render status
    void setRenderStatus(ImageStatus s) {render_status_ = s;}
    ImageStatus renderStatus() const {return render_status_;}

private:
    // Set the render attributes
    // param setting the passed-in render setting
    void setRenderSetting(const RenderSetting &setting);

    // scaling
    bool scaled(const QSize &size);
    QImage* scaled(const QSize &size, QImage *input);

    // rotation
    void rotate(const RotateDegree orient);

    // convert
    void convert(const QImage::Format f);

    // reset the bitmap data, remove the old one and create a new one
    void resetData(const QImage &new_data);

    // check whether it is necessary to reload the image or not
    bool needReload(const RenderSetting &setting);

    // quantize the image
    void quantize();

private:
    RenderSetting       render_setting_; ///< render setting
    PrivateInfo         private_info_;   ///< private information
    ConfigData          conf_;           ///< configuration
    bool                locked_;         ///< the image is locked or not
    bool                valid_;          ///< the image is valid or not
    QSize               actual_size_;    ///< actual size of the image
    unsigned int        file_size_;      ///< size of the file
    ImageStatus         render_status_;  ///< status of rendering
    scoped_ptr<QImage>  data_;           ///< instance of the QImage

    /// The current image is dirty of not
    /// if current image is "dirty"(scaled size is smaller than the actual size),
    /// and the current scaled size is bigger than current size, the image should
    /// be reload.
    bool dirty_;

    static scoped_ptr<DitheringStrategy> dithering_strategy_;
};

};
#endif
