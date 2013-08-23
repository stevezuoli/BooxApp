#ifndef IMAGE_UTILS_H_
#define IMAGE_UTILS_H_

#include "onyx/base/base.h"
#include "onyx/base/dbus.h"
#include "onyx/base/down_cast.h"
#include "onyx/ui/ui.h"
#include "onyx/ui/thumbnail_layout.h"

#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/sysinfo.h>
#endif

#include "onyx/ui/main_window.h"
#include "onyx/ui/display_pages_container.h"
#include "onyx/ui/base_thumbnail.h"
#include "onyx/ui/page_layout.h"
#include "onyx/ui/continuous_page_layout.h"
#include "onyx/ui/single_page_layout.h"
#include "onyx/sys/sys_utils.h"
#include "onyx/ui/base_view.h"
#include "onyx/ui/render_policy.h"
#include "onyx/ui/status_manager.h"

#include "onyx/data/sketch_proxy.h"

#include "onyx/sys/sys.h"
#include "onyx/sys/sys_conf.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/cms/content_thumbnail.h"

using namespace vbf;
using namespace ui;
using namespace sketch;

namespace image
{

const char * const EMPTY_BACKGROUND = "empty";
const int EMPTY_BACKGROUND_WIDTH    = 600;
const int EMPTY_BACKGROUND_HEIGHT   = 800;

enum ApplicationType
{
    IMAGE_APP,
    NOTES_APP
};

enum ImageStatus {
    IMAGE_STATUS_WAIT,
    IMAGE_STATUS_PAUSE,
    IMAGE_STATUS_DONE,
    IMAGE_STATUS_ABORT,
    IMAGE_STATUS_RUNNING,
    IMAGE_STATUS_FAIL
};

enum ImageViewType
{
    IMAGE_VIEW = 0,
    NOTES_VIEW,
    TOC_VIEW,
    THUMBNAIL_VIEW
};

struct ViewSetting
{
    RotateDegree rotate_orient;
    ZoomFactor   zoom_setting;
    ViewSetting(): rotate_orient(ROTATE_0_DEGREE), zoom_setting(ZOOM_TO_PAGE) {}
};

#define SLIDE_TIME_INTERVAL  5000

typedef QString         ImageKey;
typedef float           ZoomFactor;

class ImageItem;
bool isGlobalSettingChanged(const ImageItem * image);
int compare(const ImageItem & p1,
            const ImageItem & p2,
            vbf::RenderPolicy * render_policy);
int comparePriority(const ImageItem & p1,
                    const ImageItem & p2,
                    vbf::RenderPolicy * render_policy);

class ImageThumbnail;
int compare(const ImageThumbnail & p1,
            const ImageThumbnail & p2,
            vbf::RenderPolicy * render_policy);
int comparePriority(const ImageThumbnail & p1,
                    const ImageThumbnail & p2,
                    vbf::RenderPolicy * render_policy);

// Get the display area of thumbnail
void getThumbnailRectangle(const QRect& bounding_rect,
                           const QSize& origin_size,
                           QRect* thumb_rect);

// Intialize the color table
void initialColorTable();

// Get the system rotation degree
RotateDegree getSystemRotateDegree();

// Pan area
class PanArea
{
public:
    PanArea() : p1(), p2() {}
    ~PanArea() {}
    void setStartPoint(const QPoint &pos);
    void setEndPoint(const QPoint &pos);
    void getOffset(int &offset_x, int &offset_y);
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

};

#endif  // IMAGE_UTILS_H_
