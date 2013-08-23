#ifndef DJVU_UTILS_H_
#define DJVU_UTILS_H_

#include "onyx/base/base.h"
#include "onyx/base/dbus.h"
#include "onyx/base/down_cast.h"
#include "onyx/base/base_model.h"
#include "onyx/base/base_tasks.h"
#include "onyx/base/tasks_handler.h"

#include "onyx/ui/ui.h"
#include "onyx/ui/thumbnail_layout.h"
#include "onyx/ui/base_thumbnail.h"
#include "onyx/ui/base_view.h"
#include "onyx/ui/languages.h"
#include "onyx/ui/main_window.h"
#include "onyx/ui/display_pages_container.h"
#include "onyx/ui/page_layout.h"
#include "onyx/ui/continuous_page_layout.h"
#include "onyx/ui/single_page_layout.h"
#include "onyx/ui/notes_dialog.h"
#include "onyx/ui/render_policy.h"
#include "onyx/ui/status_manager.h"

#include "onyx/cms/content_thumbnail.h"

#include "onyx/data/bookmark.h"
#include "onyx/data/reading_history.h"
#include "onyx/data/sketch_proxy.h"

#include "onyx/sys/sys_utils.h"
#include "onyx/sys/sys.h"
#include "onyx/sys/sys_conf.h"

#include "onyx/screen/screen_proxy.h"

#include "libdjvu/DjVuAnno.h"
#include "libdjvu/DjVuText.h"
#include "libdjvu/DjVuImage.h"
#include "libdjvu/ByteStream.h"
#include "libdjvu/DjVmNav.h"
#include "libdjvu/DjVuDocument.h"
#include "libdjvu/IFFByteStream.h"
#include "libdjvu/DataPool.h"
#include "libdjvu/GPixmap.h"
#include "libdjvu/GBitmap.h"

namespace djvu_reader
{

enum DjvuViewType
{
    DJVU_VIEW = 0,
    TOC_VIEW,
    THUMBNAIL_VIEW
};

enum DjVuFormatStyle
{
    DDJVU_FORMAT_BGR24,           /* truecolor 24 bits in BGR order */
    DDJVU_FORMAT_RGB24,           /* truecolor 24 bits in RGB order */
    DDJVU_FORMAT_RGBMASK16,       /* truecolor 16 bits with masks */
    DDJVU_FORMAT_RGBMASK32,       /* truecolor 32 bits with masks */
    DDJVU_FORMAT_GREY8,           /* greylevel 8 bits */
    DDJVU_FORMAT_PALETTE8,        /* paletized 8 bits (6x6x6 color cube) */
    DDJVU_FORMAT_MSBTOLSB,        /* packed bits, msb on the left */
    DDJVU_FORMAT_LSBTOMSB,        /* packed bits, lsb on the left */
};

enum DjVuRenderMode
{
  DDJVU_RENDER_COLOR = 0,       /* color page or stencil */
  DDJVU_RENDER_BLACK,           /* stencil or color page */
  DDJVU_RENDER_COLORONLY,       /* color page or fail */
  DDJVU_RENDER_MASKONLY,        /* stencil or fail */
  DDJVU_RENDER_BACKGROUND,      /* color background layer */
  DDJVU_RENDER_FOREGROUND,      /* color foreground layer */
};

enum OutlineProperty
{
    OUTLINE_ITEM = Qt::UserRole + 1
};

enum ThumbnailRenderDirection
{
    THUMBNAIL_RENDER_INVALID = -1,
    THUMBNAIL_RENDER_CURRENT_PAGE = 0,
    THUMBNAIL_RENDER_NEXT_PAGE,
    THUMBNAIL_RENDER_PREVIOUS_PAGE
};

struct ViewSetting
{
    RotateDegree rotate_orient;
    ZoomFactor   zoom_setting;
    ViewSetting(): rotate_orient(ROTATE_0_DEGREE), zoom_setting(ZOOM_HIDE_MARGIN) {}
};

// Pan area
class PanArea
{
public:
    PanArea() : p1(), p2() {}
    ~PanArea() {}
    inline void setStartPoint(const QPoint &pos);
    inline void setEndPoint(const QPoint &pos);
    inline void getOffset(int &offset_x, int &offset_y);
    inline const QPoint & getStart() const;
    inline const QPoint & getEnd() const;
private:
    /// pan start point
    QPoint p1;

    /// pan end point
    QPoint p2;
};

class StrokeArea
{
public:
    StrokeArea() : area_() {}
    ~StrokeArea() {}

    void initArea(const QPoint &point);
    void expandArea(const QPoint &point);
    const QRect & getRect();
    const QPoint & getOriginPosition() { return origin_pos_; }
private:
    QRect area_;           /// stroke area
    QPoint origin_pos_;    /// original position of the area
};

/// The class RenderSetting contains all of the render settings of an image
class RenderSetting
{
public:
    RenderSetting()
        : content_area_()
        , rotation_(ROTATE_0_DEGREE)
        , clip_image_(false)
        , is_thumbnail_(false)
        , thumbnail_direction_(THUMBNAIL_RENDER_INVALID){}
    ~RenderSetting() {}

    RenderSetting(const RenderSetting &right)
    {
        *this = right;
    }

    RenderSetting& operator = (const RenderSetting &right)
    {
        if (*this == right)
        {
            return *this;
        }

        content_area_ = right.content_area_;
        clip_area_    = right.clip_area_;
        rotation_     = right.rotation_;
        clip_image_   = right.clip_image_;
        is_thumbnail_ = right.is_thumbnail_;
        thumbnail_direction_ = right.thumbnail_direction_;
        return *this;
    }

    bool operator == (const RenderSetting &right) const
    {
        return (clip_image_ == right.clip_image_
                && (clip_image_ ? clip_area_ == right.clip_area_ : 1)
                && content_area_ == right.content_area_
                && rotation_ == right.rotation_);
    }

    bool operator != (const RenderSetting & right) const
    {
        return !(*this == right);
    }

    // set the content area
    void setContentArea(const QRect &content_area) { content_area_.setSize(content_area.size()); }
    const QRect& contentArea() const {return content_area_;}

    // set rotation
    void setRotation(const RotateDegree r) { rotation_ = r; }
    RotateDegree rotation() const { return rotation_; }

    // clipping
    void setClipArea(const QRect &clip_area) { clip_area_ = clip_area; }
    const QRect& clipArea() const { return clip_area_; }
    void setClipImage(bool c) {clip_image_ = c;}
    bool isClipImage() const { return clip_image_; }

    // Thumbnail
    bool isThumbnail() const { return is_thumbnail_; }
    void setToBeThumbnail(bool is_thumbnail) { is_thumbnail_ = is_thumbnail; }
    void setThumbnailDirection(ThumbnailRenderDirection direction) { thumbnail_direction_ = direction; }
    ThumbnailRenderDirection thumbnailDirection() const { return thumbnail_direction_; }

private:
    QRect        content_area_;     ///< the rectangle of the render area
    RotateDegree rotation_;         ///< the rotation degrees
    QRect        clip_area_;        ///< clip area
    bool         clip_image_;       ///< clip the image?

    bool                        is_thumbnail_;          ///< Is thumbnail image?
    ThumbnailRenderDirection    thumbnail_direction_;   ///< Thumbnail direction
};

class GlobalRenderFormat
{
private:
    GlobalRenderFormat()
    {
        update(DDJVU_FORMAT_GREY8, 0, 0);
        rtoptobottom = true;
        ytoptobottom = true;
        ditherbits = 8;
    }

    NO_COPY_AND_ASSIGN(GlobalRenderFormat);

public:
    static GlobalRenderFormat & instance()
    {
        static GlobalRenderFormat format;
        return format;
    }

    inline bool update(DjVuFormatStyle style, int nargs, unsigned int *args);

public:
    DjVuFormatStyle format_style;
    unsigned int rgb[3][256];
    unsigned int palette[6 * 6 * 6];
    unsigned int xorval;
    double gamma;
    char ditherbits;
    bool rtoptobottom;
    bool ytoptobottom;
};

/// Set start point of pan area
inline void PanArea::setStartPoint(const QPoint &pos)
{
    p1 = pos;
}

/// Set end point of pan area
inline void PanArea::setEndPoint(const QPoint &pos)
{
    p2 = pos;
}

/// Get the offset of pan area
inline void PanArea::getOffset(int &offset_x, int &offset_y)
{
    offset_x = p1.x() - p2.x();
    offset_y = p1.y() - p2.y();
}

inline const QPoint & PanArea::getStart() const
{
    return p1;
}

inline const QPoint & PanArea::getEnd() const
{
    return p2;
}

/// Initialize the stroke area
inline void StrokeArea::initArea(const QPoint &point)
{
    origin_pos_ = point;
    area_.setTopLeft(point);
    area_.setBottomRight(point);
}

/// Expand the stroke area
inline void StrokeArea::expandArea(const QPoint &point)
{
    if (area_.left() > point.x())
    {
        area_.setLeft(point.x());
    }
    else if (area_.right() < point.x())
    {
        area_.setRight(point.x());
    }

    if (area_.top() > point.y())
    {
        area_.setTop(point.y());
    }
    else if (area_.bottom() < point.y())
    {
        area_.setBottom(point.y());
    }
}

/// Get the bounding rectangle of stroke area
inline const QRect & StrokeArea::getRect()
{
    area_ = area_.normalized();
    return area_;
}

bool GlobalRenderFormat::update(DjVuFormatStyle style, int nargs, unsigned int *args)
{
    memset(&rgb, 0, sizeof(rgb));
    memset(&palette, 0, sizeof(palette));

    format_style = style;
    rtoptobottom = false;
    ytoptobottom = false;
    gamma = 2.2;

    // Ditherbits
    ditherbits = 32;
    if (format_style == DDJVU_FORMAT_RGBMASK16)
    {
        ditherbits = 16;
    }
    else if (format_style == DDJVU_FORMAT_PALETTE8)
    {
        ditherbits = 8;
    }
    else if (format_style == DDJVU_FORMAT_MSBTOLSB || format_style == DDJVU_FORMAT_LSBTOMSB)
    {
        ditherbits = 1;
    }

    // Args
    switch(format_style)
    {
        case DDJVU_FORMAT_RGBMASK16:
        case DDJVU_FORMAT_RGBMASK32:
        {
            if (!args || nargs < 3 || nargs > 4)
            {
                return false;
            }
            {
                // extra nesting for windows
                for (int j=0; j<3; j++)
                {
                    int shift = 0;
                    unsigned int mask = args[j];
                    for (shift = 0; shift < 32 && !(mask & 1); shift++)
                    {
                        mask >>= 1;
                    }
                    if ((shift >= 32) || (mask & (mask + 1)))
                    {
                        return false;
                    }
                    for (int i = 0; i < 256; i++)
                    {
                        rgb[j][i] = (mask & ((int)((i * mask + 127.0) / 255.0))) << shift;
                    }
                }
            }
            if (nargs >= 4)
            {
                xorval = args[3];
            }
            break;
        }
        case DDJVU_FORMAT_PALETTE8:
        {
            if (nargs != 6*6*6 || !args)
            {
                return false;
            }
            {
                // extra nesting for windows
                for (int k = 0; k < 6*6*6; k++)
                {
                    palette[k] = args[k];
                }
            }
            { // extra nesting for windows
                int j = 0;
                for(int i = 0; i < 6; i++)
                {
                    for(; j < (i+1) * 0x33 - 0x19 && j < 256; j++)
                    {
                        rgb[0][j] = i * 6 * 6;
                        rgb[1][j] = i * 6;
                        rgb[2][j] = i;
                    }
                }
            }
            break;
        }
        case DDJVU_FORMAT_RGB24:
        case DDJVU_FORMAT_BGR24:
        case DDJVU_FORMAT_GREY8:
        case DDJVU_FORMAT_LSBTOMSB:
        case DDJVU_FORMAT_MSBTOLSB:
            if (!nargs)
            {
                return false;
            }
            break;
        default:
            return false;
    }
    return true;
}

};

#endif
