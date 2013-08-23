#ifndef ADOBE_RENDER_CONF_H_
#define ADOBE_RENDER_CONF_H_

#include "adobe_utils.h"
#include "adobe_location.h"

namespace dpdoc
{
    struct Matrix;
};

using namespace ui;
using namespace vbf;
namespace adobe_view
{

class AdobeRendererClient;
class AdobeRenderConf : public BaseTask
{
public:
    AdobeRenderConf();
    AdobeRenderConf( AdobeRenderOperation opt, AdobeRendererClient * renderer_client );
    AdobeRenderConf( const AdobeRenderConf & right );
    virtual ~AdobeRenderConf();

    AdobeRenderConf & operator = ( const AdobeRenderConf & right );

    void exec();
    void intiNavMatrix();
    bool restore();
    bool isFitForScreen();
    AdobeViewportLocations viewportLocation();

    void exportToConfiguration( Configuration & conf );
    bool loadFromConfiguration( const Configuration & conf );

    inline AdobeRenderOperation getOperation() const;
    inline void clear();
    inline void setOperation( AdobeRenderOperation opt );
    inline void setNeedPlayVoice( bool need );
    inline bool needPlayVoice() const;

    inline int getRotateDegree() const;
    inline int getRotateSetting() const;
    inline void setRotateDegree( int degree );
    inline double getZoomSetting() const;
    inline void setZoomSetting( double zoom );

    inline double getFontSize() const;
    inline void setFontSize( const double size );
    inline double getFontRatio() const;
    inline void setFontRatio( const double ratio );

    inline AdobeLocationPtr getPagePosition() const;
    inline void setPagePosition( AdobeLocationPtr page_pos );
    //inline void getHighlightInfo( int & type, int & index );
    //inline void setHighlightInfo( int type, int index );
    inline void setScrollOffset( const int h_offset, const int v_offset );
    inline void getScrollOffset( int & h_offset, int & v_offset ) const;

    inline void setExtNavMatrix( const dpdoc::Matrix *matrix);
    inline const dpdoc::Matrix* getExtNavMatrix() const;
    inline const dpdoc::Matrix* getEnvMatrix() const;
    inline int getPagingMode() const;
    inline void setPagingMode( int mode );

    inline const QPoint & getCenterPoint() const;
    inline void setCenterPoint( const QPoint & c );
    inline const QSize & getViewport() const;
    inline void setViewport( const QSize & viewport );

    inline const QSize & getThumbnailSize() const;
    inline void setThumbnailSize( const QSize & thumb_size );
private:
    // Operations
    bool navigate();
    bool scaleSurface();
    bool scaleThumbnail( QSize & thumb_size, double & zoom_value );
    bool scaleArea();
    bool scaleFontRatio();
    bool scaleFontSize();
    bool rotate();
    bool scroll();
    bool updateByExtMatrix();
    bool switchPagingMode();

    // Helper Functions
    void paintSurface();
    void paintThumbnail( const QSize & thumb_size, const double & zoom_value );
    bool translate();
    bool checkBoundary( dpdoc::Matrix* nav_matrix );
    double realZoom( const QSize & nature_size,
                     const QSize & view_size,
                     double zoom_setting ) const;
    bool getDisplaySize( double real_zoom,
                         QSize & nature_size,
                         QSize & display_size ) const;
    void getPageNatureSize( QSize & nature_size );
    void getGlobalNatureSize( QSize & nature_size );
    void getCenter( const QSize & display_size,
                    bool is_view_center,
                    QPoint & center );
    void updateViewport( const QSize & view_size );
    void updateMinFontZoomFactor();
    void restoreDisplayAreaForFontIndex( double zoom );

    // render range
    void generateRenderRange();

private:
    AdobeRendererClient  *renderer_client_;
    AdobeRenderOperation operation_;

    // Location
    AdobeLocationPtr     page_position_;

    // Play Voice
    bool                 need_play_voice_;

    // RENDER_SCALE_ZOOM
    double               zoom_setting_;

    // RENDER_ROTATE
    int                  rotate_degree_;

    // RENDER_NAVIGATE_SCROLLING
    int                  scroll_offset_x_;
    int                  scroll_offset_y_;

    // RENDER_SCALE_FONT_INDEX
    double               font_ratio_;

    // RENDER_SCALE_FONT_SIZE
    double               font_size_;

    // RENDER_UPDATE_BY_MATRIX
    scoped_ptr<dpdoc::Matrix> ext_nav_matrix_;

    // RENDER_SWITCH_PAGE_MODE
    int                  paging_mode_;

    // RENDER_SCALE_AREA
    QPoint               center_;

    QSize                viewport_;
    QSize                thumbnail_size_;
    double               min_font_zoom_;
    scoped_ptr<dpdoc::Matrix> env_matrix_;

    friend bool operator == ( const AdobeRenderConf & a, const AdobeRenderConf & b );
};

void AdobeRenderConf::clear()
{
    page_position_ = AdobeLocationPtr();
}

AdobeRenderOperation AdobeRenderConf::getOperation() const
{
    return operation_;
}

void AdobeRenderConf::setNeedPlayVoice( bool need )
{
    need_play_voice_ = need;
}

bool AdobeRenderConf::needPlayVoice() const
{
    return need_play_voice_;
}

void AdobeRenderConf::setOperation( AdobeRenderOperation opt )
{
    operation_ = opt;
}

int AdobeRenderConf::getRotateDegree() const
{
    return rotate_degree_;
}

void AdobeRenderConf::setRotateDegree( int degree )
{
    rotate_degree_ = degree;
}

double AdobeRenderConf::getZoomSetting() const
{
    return zoom_setting_;
}

void AdobeRenderConf::setZoomSetting( double zoom )
{
    zoom_setting_ = zoom;
}

AdobeLocationPtr AdobeRenderConf::getPagePosition() const
{
    return page_position_;
}

void AdobeRenderConf::setPagePosition( AdobeLocationPtr page_pos )
{
    page_position_ = page_pos;
}

void AdobeRenderConf::setScrollOffset( const int h_offset, const int v_offset )
{
    scroll_offset_x_ = h_offset;
    scroll_offset_y_ = v_offset;
}

void AdobeRenderConf::getScrollOffset( int & h_offset, int & v_offset ) const
{
    h_offset = scroll_offset_x_;
    v_offset = scroll_offset_y_;
}

int AdobeRenderConf::getRotateSetting() const
{
    switch (rotate_degree_)
    {
    case 0:
        return ROTATE_0_DEGREE;
    case 90:
        return ROTATE_90_DEGREE;
    case 180:
        return ROTATE_180_DEGREE;
    case 270:
        return ROTATE_270_DEGREE;
    }
    return INVALID_ROTATE_DEGREE;
}

double AdobeRenderConf::getFontSize() const
{
    return font_size_;
}

void AdobeRenderConf::setFontSize( const double size )
{
    font_size_ = size;
}

double AdobeRenderConf::getFontRatio() const
{
    return font_ratio_;
}

void AdobeRenderConf::setFontRatio( const double ratio )
{
    font_ratio_ = ratio;
}

int AdobeRenderConf::getPagingMode() const
{
    return paging_mode_;
}

const dpdoc::Matrix* AdobeRenderConf::getExtNavMatrix() const
{
   return ext_nav_matrix_.get();
}

const dpdoc::Matrix* AdobeRenderConf::getEnvMatrix() const
{
    return env_matrix_.get();
}

void AdobeRenderConf::setPagingMode( int mode )
{
    paging_mode_ = mode;
}

const QPoint & AdobeRenderConf::getCenterPoint() const
{
    return center_;
}

void AdobeRenderConf::setCenterPoint( const QPoint & c )
{
    center_ = c;
}

const QSize & AdobeRenderConf::getViewport() const
{
    return viewport_;
}

void AdobeRenderConf::setViewport( const QSize & viewport )
{
    viewport_ = viewport;
}

const QSize & AdobeRenderConf::getThumbnailSize() const
{
    return thumbnail_size_;
}

void AdobeRenderConf::setThumbnailSize( const QSize & thumb_size )
{
    thumbnail_size_ = thumb_size;
}

bool operator == ( const AdobeRenderConf & a, const AdobeRenderConf & b );

};

Q_DECLARE_METATYPE(adobe_view::AdobeRenderConf);

#endif
