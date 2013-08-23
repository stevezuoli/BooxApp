#ifndef IMAGE_THUMBNAIL_H
#define IMAGE_THUMBNAIL_H

#include "onyx/ui/base_thumbnail.h"

#include "image_utils.h"

using namespace onyx::ui;
using namespace ui;

namespace image
{

class ImageThumbnail : public BaseThumbnail
{
public:
    ImageThumbnail();
    explicit ImageThumbnail(const ImageKey &k);
    virtual ~ImageThumbnail();

    QImage* image() const { return image_.get(); }
    const QString & path() const;
    const QString & name() const;

    int key() const { return key_; }
    void setKey(const int k) { key_ = k; }

    void clearPage();
    int length();

    // record current time
    void recordCurrentTime();

    int index() const { return key(); }

    // update display area
    void updateDisplayArea(const QRect &rect);
    const QRect& displayArea() const { return area_; }
    void setOriginSize(const QSize & size);
    ZoomFactor zoom();

    void setImage(QImage* image) { image_.reset(image); }

    const QSize size() const { return area_.size(); }
    const QTime & time() const { return timestamp_; }

private:
    scoped_ptr<QImage> image_;       /// qt image
    ImageKey           name_;        /// name of the image
    QString            path_;        /// path of the image
    int                key_;         /// index of the thumbnail image
    QTime              timestamp_;   /// render Time
    QRect              area_;        /// display area
    QSize              origin_size_; /// original size of the image
};

};

#endif
