
#include "image_thumbnail.h"

namespace image
{

ImageThumbnail::ImageThumbnail()
: image_(0)
, name_()
, key_()
, timestamp_()
, area_()
{
}

ImageThumbnail::ImageThumbnail(const ImageKey &k)
    : image_(0)
    , key_(0)
    , timestamp_()
    , area_()
{
    QFileInfo info(k);
    name_ = info.fileName();
    path_ = info.absoluteFilePath();
}

ImageThumbnail::~ImageThumbnail()
{
}

const QString & ImageThumbnail::path() const
{
    return path_;
}

const ImageKey & ImageThumbnail::name() const
{
    return name_;
}

void ImageThumbnail::clearPage()
{
    image_.reset();
}

void ImageThumbnail::recordCurrentTime()
{
    timestamp_.restart();
}

int ImageThumbnail::length()
{
    if (image_ == 0)
    {
        return 0;
    }

    return image_->numBytes();
}

void ImageThumbnail::updateDisplayArea(const QRect &rect)
{
    area_ = rect;
}

void ImageThumbnail::setOriginSize(const QSize & size)
{
    origin_size_ = size;
}

ZoomFactor ImageThumbnail::zoom()
{
    ZoomFactor zoom = 0.0f;
    if (!origin_size_.isNull())
    {
        zoom = static_cast<ZoomFactor>(area_.width())/static_cast<ZoomFactor>(origin_size_.width());
    }
    return zoom;
}

}
