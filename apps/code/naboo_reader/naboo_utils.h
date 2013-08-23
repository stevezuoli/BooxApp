#ifndef NABOO_UTILS_H_
#define NABOO_UTILS_H_

#include <vector>
#include <fstream>
#include <math.h>


#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "onyx/base/dbus.h"

#include "onyx/ui/ui.h"

#include "onyx/dictionary/dictionary_manager.h"
#include "onyx/dictionary/dict_widget.h"

#include "onyx/tts/tts_widget.h"
#include "onyx/tts/tts.h"

#include "onyx/cms/content_thumbnail.h"

#include "onyx/base/base_model.h"
#include "onyx/ui/base_view.h"
#include "onyx/ui/base_thumbnail.h"
#include "onyx/data/bookmark.h"
#include "onyx/ui/main_window.h"
#include "onyx/sys/sys_utils.h"
#include "onyx/data/reading_history.h"
#include "onyx/ui/search_view.h"
#include "onyx/ui/status_manager.h"
#include "onyx/ui/notes_dialog.h"
#include "onyx/ui/display_pages_container.h"
#include "onyx/ui/thumbnail_layout.h"

#include "onyx/data/sketch_proxy.h"
#include "onyx/sys/sys.h"
#include "onyx/sys/platform.h"
#include "onyx/screen/screen_proxy.h"

#include "adobe_view_engine/adobe_location.h"
#include "adobe_view_engine/adobe_document.h"
#include "adobe_view_engine/adobe_renderer.h"
#include "adobe_view_engine/adobe_sketch_client.h"
#include "adobe_view_engine/adobe_events.h"

using namespace adobe_view;

namespace naboo_reader
{

enum NabooViewType
{
    NABOO_VIEW = 0,
    TOC_VIEW,
    THUMBNAIL_VIEW
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
    QPoint p1;    /// pan start point
    QPoint p2;    /// pan end point
};

// Stroke Area
class StrokeArea
{
public:
    StrokeArea() : area_() {}
    ~StrokeArea() {}

    inline void initArea(const QPoint &point);
    inline void expandArea(const QPoint &point);
    inline const QRect & getRect();
    inline const QPoint & getOriginPosition() { return origin_pos_; }
private:
    QRect area_;           /// stroke area
    QPoint origin_pos_;    /// original position of the area
};

// Annotation Context
class AnnotationCtx
{
public:
    AnnotationCtx() { reset(); }
    ~AnnotationCtx() {}

    inline void reset();
    inline void addLocation( AdobeLocationPtr loc );

    inline AdobeLocationPtr lastLocation();
    inline AdobeLocationPtr currentLocation();
    inline AdobeLocationPtr firstLocation();
    inline bool isValid();
    inline void setExplanation(const QString & content);

    QString getText( AdobeDocumentClient * document );

private:
    AdobeLocationPtr    last_location_;
    AdobeLocationPtr    current_location_;
    AdobeLocationPtr    first_location_;
    QString             explanation_;
    bool                need_fast_paint_selection_;
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

/// Get the start point
inline const QPoint & PanArea::getStart() const
{
    return p1;
}

/// Get the end point
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

inline void AnnotationCtx::reset()
{
    last_location_    = AdobeLocationPtr();
    current_location_ = AdobeLocationPtr();
    first_location_   = AdobeLocationPtr();
    explanation_.clear();
}

inline void AnnotationCtx::addLocation( AdobeLocationPtr loc )
{
    if (first_location_ == 0)
    {
        first_location_ = loc;
    }

    if (current_location_ != 0)
    {
        last_location_ = current_location_;
    }
    else
    {
        last_location_ = loc;
    }
    current_location_ = loc;
}

inline AdobeLocationPtr AnnotationCtx::lastLocation()
{
    return last_location_;
}

inline AdobeLocationPtr AnnotationCtx::currentLocation()
{
    return current_location_;
}

inline AdobeLocationPtr AnnotationCtx::firstLocation()
{
    return first_location_;
}

inline bool AnnotationCtx::isValid()
{
  return ( first_location_ != 0 &&
           current_location_ != 0 &&
           !(first_location_ == current_location_) );
}

inline void AnnotationCtx::setExplanation(const QString & content)
{
    explanation_ = content;
}

};

#endif
