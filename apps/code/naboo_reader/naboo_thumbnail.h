#ifndef NABOO_THUMBNAIL_H
#define NABOO_THUMBNAIL_H

#include "onyx/ui/base_thumbnail.h"
#include "naboo_utils.h"

using namespace onyx::ui;
using namespace ui;

namespace naboo_reader
{

class NabooThumbnail : public BaseThumbnail
{
public:
    NabooThumbnail();
    NabooThumbnail(const AdobeLocationPtr & loc, ZoomFactor zoom_value);
    virtual ~NabooThumbnail();

    const QRect& displayArea() const;
    QImage* image() const { return image_.get(); }
    const QString & path() const;
    const QString & name() const;
    int key() const;
    ZoomFactor zoom() { return zoom_value_; }

    void setImage(QImage* image) { image_.reset(image); rect_ = image_->rect(); }

private:
    scoped_ptr<QImage> image_;       /// qt image
    AdobeLocationPtr   location_;    /// location
    ZoomFactor         zoom_value_;  /// zoom_value
    QString            bookmark_;
    QString            name_;
    int                position_;
    QRect              rect_;
};

NabooThumbnail::NabooThumbnail()
{
}

NabooThumbnail::NabooThumbnail(const AdobeLocationPtr & loc, ZoomFactor zoom_value)
    : image_(0)
    , location_(loc)
    , zoom_value_(zoom_value)
    , bookmark_(location_->getBookmark())
    , name_()
    , position_(static_cast<int>(location_->getPagePosition()))
    , rect_(QPoint(0, 0), cms::thumbnailSize(THUMBNAIL_LARGE))
{
    name_.setNum(position_ + 1);
}

NabooThumbnail::~NabooThumbnail()
{
}

const QString & NabooThumbnail::path() const
{
    return bookmark_;
}

const QString & NabooThumbnail::name() const
{
    return name_;
}

int NabooThumbnail::key() const
{
    return position_;
}

const QRect& NabooThumbnail::displayArea() const
{
    // NOT implement
    return rect_;
}

};

#endif
