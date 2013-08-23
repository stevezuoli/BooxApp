#include "dp_all.h"
#include "adobe_surface.h"
#include "adobe_renderer.h"

namespace adobe_view
{

#ifndef USE_PIXMAP
static const int BACKGROUND_COLOR = 0xFF;
static QVector<QRgb> COLOR_TABLE;

static void InitialColorTable()
{
    if (COLOR_TABLE.size() <= 0)
    {
        for(int i = 0; i < 256; ++i)
        {
            COLOR_TABLE.push_back(qRgba(i, i, i, 255));
        }
    }
}
#endif

static unsigned char g_dithering_clip_map[256];
static bool          g_dithering_clip_map_initialized = false;
static int           g_dithering_depth = 3;

static int getBytesPerPixel( QImage::Format format )
{
    switch ( format )
    {
    case QImage::Format_RGB888:
        return 3;
    case QImage::Format_ARGB32:
        return 4;
    case QImage::Format_Indexed8:
        return 1;
    }
    return 3;
}

static bool getContentFromPage(const QImage & page,
                               const int width,
                               const int height,
                               QRect & content)
{
    static const int LINE_STEP        = 4;
    static const int SHRINK_STEP      = 4;
    static const double STOP_RANGE    = 0.4f;
    static const int BACKGROUND_COLOR = 0xffffffff;

    // top left
    int x1 = 0;
    int y1 = 0;
    // bottom right
    int x2 = width - 1;
    int y2 = height - 1;

    int left_edge = static_cast<int>(STOP_RANGE * x2);
    int right_edge = static_cast<int>((1.0f - STOP_RANGE) * x2);
    int top_edge = static_cast<int>(STOP_RANGE * y2);
    int bottom_edge = static_cast<int>((1.0f - STOP_RANGE) * y2);

    // current pixel
    QRgb cur_pix;
    bool stop[4] = {false, false, false, false};
    while (!stop[0] || !stop[1] || !stop[2] || !stop[3])
    {
        // check top line
        int x_cur = x1;
        while (x_cur < x2 && !stop[0])
        {
            cur_pix = page.pixel(x_cur, y1);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[0] = true;
                break;
            }
            x_cur += LINE_STEP;
        }

        // check bottom line
        x_cur = x1;
        while (x_cur < x2 && !stop[1])
        {
            cur_pix = page.pixel(x_cur, y2);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[1] = true;
                break;
            }
            x_cur += LINE_STEP;
        }

        // check left line
        int y_cur = y1;
        while (y_cur < y2 && !stop[2])
        {
            cur_pix = page.pixel(x1, y_cur);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[2] = true;
                break;
            }
            y_cur += LINE_STEP;
        }

        // check right line
        y_cur = y1;
        while (y_cur < y2 && !stop[3])
        {
            cur_pix = page.pixel(x2, y_cur);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[3] = true;
                break;
            }
            y_cur += LINE_STEP;
        }

        // shrink the rectangle
        if (!stop[2])
        {
            if (x1 >= left_edge)
            {
                stop[2] = true;
            }
            else
            {
                x1 += SHRINK_STEP;
            }
        }

        if (!stop[3])
        {
            if (x2 <= right_edge)
            {
                stop[3] = true;
            }
            else
            {
                x2 -= SHRINK_STEP;
            }
        }

        if (!stop[0])
        {
            if (y1 >= top_edge)
            {
                stop[0] = true;
            }
            else
            {
                y1 += SHRINK_STEP;
            }
        }

        if (!stop[1])
        {
            if (y2 <= bottom_edge)
            {
                stop[1] = true;
            }
            else
            {
                y2 -= SHRINK_STEP;
            }
        }

    }

    if (stop[0] && stop[1] && stop[2] && stop[3])
    {
        content.setTopLeft(QPoint(x1, y1));
        content.setBottomRight(QPoint(x2, y2));
        return true;
    }
    return false;
}

static void expandContentArea(const int page_width,
                              const int page_height,
                              QRect & content_area)
{
    static const int EXPAND_STEP = 4;
    // expand the content area to avoid content covering
    int inc_right = 0;
    int inc_bottom = 0;

    // expand left
    if (content_area.left() > EXPAND_STEP)
    {
        content_area.setLeft(content_area.left() - EXPAND_STEP);
        inc_right = EXPAND_STEP;
    }
    else
    {
        inc_right = content_area.left();
        content_area.setLeft(0);
    }

    // expand top
    if (content_area.top() > EXPAND_STEP)
    {
        content_area.setTop(content_area.top() - EXPAND_STEP);
        inc_bottom = EXPAND_STEP;
    }
    else
    {
        inc_bottom = content_area.top();
        content_area.setTop(0);
    }

    // expand right
    content_area.setRight(content_area.right() + inc_right + 1);
    if (content_area.right() > page_width)
    {
        content_area.setRight(page_width);
    }

    // expand bottom
    content_area.setBottom(content_area.bottom() + inc_bottom + 1);
    if (content_area.bottom() > page_height)
    {
        content_area.setBottom(page_height);
    }
}

class AdobeSurfacePrivate : public dpdoc::Surface
{
public:
    AdobeSurfacePrivate();
    AdobeSurfacePrivate( const AdobeRenderConf & render_conf, const QSize & size );
    virtual ~AdobeSurfacePrivate();

    virtual int getSurfaceKind();
    virtual int getPixelLayout();
    virtual unsigned char * getTransferMap( int channel );
    virtual unsigned char * getDitheringClipMap( int channel );
    virtual int getDitheringDepth( int channel );
    virtual unsigned char * checkOut( int xMin, int yMin, int xMax, int yMax, size_t * stride );
    virtual void checkIn( unsigned char * basePtr );

    AdobeSurfacePrivate & operator = ( const AdobeSurfacePrivate & );
    inline const AdobeRenderConf & renderConf() const;
    int length();

public:
#ifdef USE_PIXMAP
    inline QPixmap image() { return image_; }
#else
    inline QImage image() { return image_; }
#endif
    void clear();
    void clear( const QRect & area );
    void scale( const QSize & size );
    QRect getContentArea();

    static void intializeDitheringClipMap();
    static void setDitheringDepth(int d);

private:
    AdobeRenderConf     render_conf_;
#ifdef USE_PIXMAP
    QPixmap             image_;
#else
    QImage              image_;
#endif
};

bool operator == (AdobeSurfacePrivate &a, AdobeSurfacePrivate &b)
{
    return a.renderConf() == b.renderConf();
}

AdobeSurfacePrivate::AdobeSurfacePrivate()
{
}

AdobeSurfacePrivate::AdobeSurfacePrivate( const AdobeRenderConf & conf, const QSize & size )
    : render_conf_( conf )
#ifndef USE_PIXMAP
    , image_( size, IMG_FORMAT )
#else
    , image_( size )
#endif
{
#ifndef USE_PIXMAP
    if ( IMG_FORMAT == QImage::Format_Indexed8 ) InitialColorTable();
#endif
    intializeDitheringClipMap();
    clear();
}

AdobeSurfacePrivate::~AdobeSurfacePrivate()
{
}

inline const AdobeRenderConf & AdobeSurfacePrivate::renderConf() const
{
    return render_conf_;
}

void AdobeSurfacePrivate::intializeDitheringClipMap()
{
    if (!g_dithering_clip_map_initialized)
    {
        dpdoc::Surface::initDitheringClipMap(g_dithering_clip_map, g_dithering_depth);
        g_dithering_clip_map_initialized = true;
    }
}

AdobeSurfacePrivate & AdobeSurfacePrivate::operator = ( const AdobeSurfacePrivate & right )
{
    render_conf_ = right.render_conf_;
    image_ = right.image_.copy();
    return *this;
}

int AdobeSurfacePrivate::getSurfaceKind()
{
    return dpdoc::SK_RASTER;
}

int AdobeSurfacePrivate::getPixelLayout()
{
    //qDebug("depth %d", image_.depth());
    switch ( image_.depth() )
    {
    case 24:
        //qDebug("rgb pixel format");
        return dpdoc::PL_RGB;
    case 32:
        //qDebug("argb pixel format");
        return dpdoc::PL_ARGB;
    case 8:
        //qDebug("8-bits per channel format");
        return dpdoc::PL_L;
    }
    qDebug("bgr  pixel format");
    return dpdoc::PL_BGR;
}

unsigned char * AdobeSurfacePrivate::getTransferMap(int channel)
{
    return 0;
}

unsigned char * AdobeSurfacePrivate::getDitheringClipMap(int channel)
{
    int depth = getDitheringDepth( 0 );
    if ( depth == 0 || depth > 3 )
    {
        return 0;
    }
    return g_dithering_clip_map;
}

void AdobeSurfacePrivate::setDitheringDepth(int d)
{
    if (g_dithering_depth != d)
    {
        g_dithering_depth = d;
        g_dithering_clip_map_initialized = false;
        intializeDitheringClipMap();
    }
}

int AdobeSurfacePrivate::getDitheringDepth(int channel)
{
    int format = getPixelLayout();
    return format == dpdoc::PL_L ? g_dithering_depth : 0;
}

unsigned char* AdobeSurfacePrivate::checkOut( int xMin, int yMin, int xMax, int yMax, size_t * stride )
{
#ifndef USE_PIXMAP
    *stride = image_.bytesPerLine();
#else
    *stride = image_.qwsBytesPerLine();
#endif

    int bytes_per_pixel = image_.depth() / 8;
    if (bytes_per_pixel < 1)
    {
        bytes_per_pixel = 1;
    }

#ifndef USE_PIXMAP
    const uchar * buf = image_.bits();
#else
    const uchar * buf = image_.qwsBits();
#endif
    return (uchar*)buf + (*stride) * yMin + bytes_per_pixel * xMin;
}

void AdobeSurfacePrivate::checkIn( unsigned char * basePtr )
{
    // TODO. Implement Me
}

QRect AdobeSurfacePrivate::getContentArea()
{
    // intialize the content area
    QImage hide_margin_image = image_;
    QRect content_area(QPoint(0, 0), image_.size());
    int width  = image_.width();
    int height = image_.height();
    if (getContentFromPage(image_, width, height, content_area))
    {
        // get real size of content area
        expandContentArea(width, height, content_area);
    }
    return content_area;
}

void AdobeSurfacePrivate::scale( const QSize & size )
{
    image_ = image_.scaled( size );
}

void AdobeSurfacePrivate::clear()
{
#ifndef USE_PIXMAP
    if (image_.format() == QImage::Format_Indexed8)
    {
        image_.setColorTable(COLOR_TABLE);
    }
    image_.fill( QColor(255, 255, 255).rgb( ));
#else
    image_.fill();
#endif
}

void AdobeSurfacePrivate::clear( const QRect & area )
{
#ifndef USE_PIXMAP
    int stride = image_.bytesPerLine();
#else
    qDebug("AdobeSurfacePrivate::clear %d", image_.qwsBytesPerLine());
    int stride = image_.qwsBytesPerLine();
#endif

    int bytes_per_pixel = image_.depth() / 8;
    if (bytes_per_pixel < 1)
    {
        bytes_per_pixel = 1;
    }

    // get the overlap area to prevent out-of-bounding accessing
    QRect actual_area = image_.rect().intersect( area );

#ifndef USE_PIXMAP
    const uchar * buf = image_.bits();
#else
    const uchar * buf = image_.qwsBits();
#endif

    unsigned char * data = (uchar*)buf + stride * actual_area.top() +
                           bytes_per_pixel * actual_area.left();

    for ( int y = actual_area.top(); y < actual_area.bottom(); ++y )
    {
        memset( data, 255, actual_area.width() * bytes_per_pixel );
        data = (uchar*)buf + stride * (y + 1) + bytes_per_pixel * actual_area.left();
    }
}

int AdobeSurfacePrivate::length()
{
#ifndef USE_PIXMAP
    return image_.bytesPerLine() * image_.height();
#else
    qDebug("in length %d %d\n\n", image_.qwsBytesPerLine(),  image_.height());
    return image_.qwsBytesPerLine() * image_.height();
#endif
}

// Adobe Surface
AdobeSurface::AdobeSurface()
    : surface_private_(new AdobeSurfacePrivate())
{
}

AdobeSurface::AdobeSurface(const AdobeRenderConf & render_conf, const QSize & size)
    : surface_private_(new AdobeSurfacePrivate(render_conf, size))
{
}

AdobeSurface::~AdobeSurface()
{
}

dpdoc::Surface * AdobeSurface::data() const
{
    return surface_private_.get();
}

AdobeSurface & AdobeSurface::operator = ( const AdobeSurface & right )
{
    *surface_private_ = *right.surface_private_;
    return *this;
}

const AdobeRenderConf & AdobeSurface::renderConf() const
{
    return surface_private_->renderConf();
}

int AdobeSurface::length()
{
    return surface_private_->length();
}

#ifdef USE_PIXMAP
QPixmap AdobeSurface::image()
{
    return surface_private_->image();
}
#else
QImage AdobeSurface::image()
{
    return surface_private_->image();
}
#endif

void AdobeSurface::clear()
{
    surface_private_->clear();
}

void AdobeSurface::clear(const QRect & area)
{
    surface_private_->clear(area);
}

void AdobeSurface::scale(const QSize & size)
{
    surface_private_->scale(size);
}

QRect AdobeSurface::getContentArea()
{
    return surface_private_->getContentArea();
}

int AdobeSurface::estimateLength( const QSize surf_size, QImage::Format format )
{
    return getBytesPerPixel( format )  * surf_size.width() * surf_size.height();
}

int AdobeSurface::comparePriority( const AdobeSurface * s1, const AdobeSurface * s2, RenderPolicy * policy )
{
    assert( s1 != 0 && s2 != 0 );

    int s1_page_number = static_cast<int>(s1->renderConf().getPagePosition()->getPagePosition());
    int s2_page_number = static_cast<int>(s2->renderConf().getPagePosition()->getPagePosition());

    int src_pri = policy->getPriority( s1_page_number );
    int dst_pri = policy->getPriority( s2_page_number );

    // the value of priority is lower, the priority is higher
    if (src_pri > dst_pri)
    {
        return -1;
    }
    else if (src_pri < dst_pri)
    {
        return 1;
    }
    return 0;
}

void AdobeSurface::intializeDitheringClipMap()
{
    AdobeSurfacePrivate::intializeDitheringClipMap();
}

void AdobeSurface::setDitheringDepth(int d)
{
    AdobeSurfacePrivate::setDitheringDepth(d);
}

bool operator == ( AdobeSurfacePtr & a, AdobeSurfacePtr & b )
{
    return *(a->surface_private_) == *(b->surface_private_);
}

}
