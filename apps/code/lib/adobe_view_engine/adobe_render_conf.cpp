#include "dp_all.h"

#include "adobe_render_conf.h"
#include "adobe_renderer.h"
#include "adobe_document.h"

using namespace vbf;

namespace adobe_view
{

static const double DEFAULT_FONT_SIZE = 12.0f;
static const double BOUNDARY_ERROR = 5.0f;

static const bool g_enable_cache = false;

static const double ERROR = 0.1;

bool operator == ( const dpdoc::Matrix & a, const dpdoc::Matrix & b )
{
    return ( fabs( a.a - b.a ) < ERROR &&
             fabs( a.b - b.b ) < ERROR &&
             fabs( a.c - b.c ) < ERROR &&
             fabs( a.d - b.d ) < ERROR &&
             fabs( a.e - b.e ) < ERROR &&
             fabs( a.f - b.f ) < ERROR );
}

dpdoc::Matrix concat( const dpdoc::Matrix & m1, const dpdoc::Matrix & m2 )
{
    dpdoc::Matrix result;
    result.a = m2.a * m1.a + m2.b * m1.c;
    result.b = m2.a * m1.b + m2.b * m1.d;
    result.c = m2.c * m1.a + m2.d * m1.c;
    result.d = m2.c * m1.b + m2.d * m1.d;
    result.e = m1.e + m2.e * m1.a + m2.f * m1.c;
    result.f = m1.f + m2.e * m1.b + m2.f * m1.d;
    return result;
}

dpdoc::Matrix multiple( const dpdoc::Matrix & m1, const double factor )
{
    dpdoc::Matrix result;
    result.a = m1.a * factor;
    result.b = m1.b * factor;
    result.c = m1.c * factor;
    result.d = m1.d * factor;
    result.e = m1.e * factor;
    result.f = m1.f * factor;
    return result;
}

double calculateX( const dpdoc::Matrix & m, const double x, const double y )
{
    return m.a * x + m.c * y + m.e;
}

double calculateY( const dpdoc::Matrix & m, const double x, const double y )
{
    return m.b * x + m.d * y + m.f;
}

void calcDisplayArea( const dpdoc::Matrix & m,
                      const QSize & nature_size,
                      QRectF & area )
{
    QPointF left_top, right_bottom;

    left_top.setX( calculateX(m, 0, 0) );
    left_top.setY( calculateY(m, 0, 0) );

    right_bottom.setX( calculateX(m, nature_size.width(), nature_size.height()) );
    right_bottom.setY( calculateY(m, nature_size.width(), nature_size.height()) );

    area.setTop(left_top.y());
    area.setLeft(left_top.x());
    area.setRight(right_bottom.x());
    area.setBottom(right_bottom.y());
    area = area.normalized();
}

static double getScaleFactorByFontIndex( double font_ratio, double min_zoom_value )
{
    assert( min_zoom_value > ZOOM_ERR && font_ratio > ZOOM_ERR);
    return 1 / ( font_ratio * min_zoom_value );
}

static AdobeLocationPtr getCurrentLocation(AdobeRendererClient * renderer_client, bool beginning = true)
{
    return beginning ? renderer_client->getCurrentLocation() : renderer_client->getScreenEnd();
}

AdobeRenderConf::AdobeRenderConf()
    : BaseTask(dpdoc::PK_RENDER)
    , operation_(RENDER_NAVIGATE_TO_LOCATION)
    , renderer_client_( 0 )
    , need_play_voice_( false )
    , zoom_setting_( ZOOM_TO_PAGE )
    , rotate_degree_( 0 )
    , scroll_offset_x_( 0 )
    , scroll_offset_y_( 0 )
    , font_ratio_( 1.0f )
    , font_size_( DEFAULT_FONT_SIZE )
    , ext_nav_matrix_(new dpdoc::Matrix)
    , paging_mode_( dpdoc::PM_HARD_PAGES )
    , thumbnail_size_(cms::thumbnailSize( THUMBNAIL_LARGE ))
    , min_font_zoom_( 1.0f )
    , env_matrix_(new dpdoc::Matrix)
{
}

// Adobe Render Request (Handler)
AdobeRenderConf::AdobeRenderConf( AdobeRenderOperation opt,
                                  AdobeRendererClient *renderer_client )
    : BaseTask(dpdoc::PK_RENDER)
    , operation_( opt )
    , renderer_client_( renderer_client )
    , need_play_voice_( false )
    , zoom_setting_( ZOOM_TO_PAGE )
    , rotate_degree_( 0 )
    , scroll_offset_x_( 0 )
    , scroll_offset_y_( 0 )
    , font_ratio_( 1.0f )
    , font_size_( DEFAULT_FONT_SIZE )
    , ext_nav_matrix_(new dpdoc::Matrix)
    , paging_mode_( dpdoc::PM_HARD_PAGES )
    , thumbnail_size_(cms::thumbnailSize( THUMBNAIL_LARGE ))
    , min_font_zoom_( 1.0f )
    , env_matrix_(new dpdoc::Matrix)
{
}

AdobeRenderConf::AdobeRenderConf( const AdobeRenderConf & right )
    : BaseTask(dpdoc::PK_RENDER)
    , ext_nav_matrix_(new dpdoc::Matrix)
    , env_matrix_(new dpdoc::Matrix)
{
    *this = right;
}

AdobeRenderConf::~AdobeRenderConf()
{
}

AdobeRenderConf & AdobeRenderConf::operator = ( const AdobeRenderConf & right )
{
    operation_       = right.operation_;
    renderer_client_ = right.renderer_client_;
    page_position_   = right.page_position_;
    need_play_voice_ = right.need_play_voice_;
    zoom_setting_    = right.zoom_setting_;
    rotate_degree_   = right.rotate_degree_;
    scroll_offset_x_ = right.scroll_offset_x_;
    scroll_offset_y_ = right.scroll_offset_y_;
    font_ratio_      = right.font_ratio_;
    font_size_       = right.font_size_;
    *ext_nav_matrix_ = *(right.ext_nav_matrix_);
    paging_mode_     = right.paging_mode_;
    center_          = right.center_;
    //page_end_      = right.page_end_;
    viewport_        = right.viewport_;
    thumbnail_size_  = right.thumbnail_size_;
    min_font_zoom_   = right.min_font_zoom_;
    *env_matrix_     = *(right.env_matrix_);
    return *this;
}

bool operator == ( const AdobeRenderConf & a, const AdobeRenderConf & b )
{
    // only compare the display-related options
    return ( a.page_position_ == b.page_position_ &&
             a.zoom_setting_ == b.zoom_setting_ &&
             a.rotate_degree_ == b.rotate_degree_ &&
             a.font_ratio_ == b.font_ratio_ &&
             a.font_size_ == b.font_size_ &&
             *(a.ext_nav_matrix_) == *(b.ext_nav_matrix_) &&
             a.paging_mode_ == b.paging_mode_ &&
             //a.page_end_ == b.page_end_ &&
             a.viewport_ == b.viewport_ &&
             *(a.env_matrix_) == *(b.env_matrix_) );
}

void AdobeRenderConf::setExtNavMatrix( const dpdoc::Matrix* matrix)
{
    *ext_nav_matrix_ = *matrix;
}

void AdobeRenderConf::intiNavMatrix()
{
    dpdoc::Matrix nav_matrix;
    renderer_client_->renderer()->getNavigationMatrix(&nav_matrix);
    nav_matrix.e = 0.0f;
    nav_matrix.f = 0.0f;
    renderer_client_->renderer()->setNavigationMatrix(nav_matrix);
}

double AdobeRenderConf::realZoom( const QSize & nature_size,
                                  const QSize & view_size,
                                  double zoom_setting ) const
{
    if (!nature_size.isValid())
    {
        return INVALID_ZOOM;
    }

    double zoom_width  = INVALID_ZOOM_VALUE;
    double zoom_height = INVALID_ZOOM_VALUE;
    double ret = -1.0f;
    switch (rotate_degree_)
    {
    case 0:
    case 180:
        zoom_width = static_cast<double>( view_size.width() ) /
                     static_cast<double>( nature_size.width() );
        zoom_height = static_cast<double>( view_size.height() ) /
                      static_cast<double>( nature_size.height() );
        break;
    case 270:
    case 90:
        zoom_width = static_cast<double>( view_size.width() ) /
                     static_cast<double>( nature_size.height() );
        zoom_height = static_cast<double>( view_size.height() ) /
                      static_cast<double>( nature_size.width() );
        break;
    default:
        return ret;
    }

    if (zoom_setting == ZOOM_TO_PAGE)
    {
        ret = std::min( zoom_width, zoom_height );
    }
    else if (zoom_setting == ZOOM_HIDE_MARGIN)
    {
        // TODO. Implement real "hide margin" functionality
        ret = std::min( zoom_width, zoom_height );
    }
    else if (zoom_setting == ZOOM_TO_WIDTH)
    {
        ret = zoom_width;
    }
    else if (zoom_setting == ZOOM_TO_HEIGHT)
    {
        ret = zoom_height;
    }
    else if (zoom_setting > 0)
    {
        double real_factor = zoom_setting;
        if (real_factor > ZOOM_MAX)
        {
            real_factor = ZOOM_MAX;
        }
        ret = real_factor / ZOOM_ACTUAL;
    }

    static double max = ZOOM_MAX / ZOOM_ACTUAL;
    if ( ret > max )
    {
        ret = max;
    }
    return ret;
}

void AdobeRenderConf::getCenter( const QSize & display_size,
                                 bool is_view_center,
                                 QPoint & center )
{
    double view_factor = 1.0f;
    if ( renderer_client_->isInFontIndexMode() )
    {
        view_factor = getScaleFactorByFontIndex( font_ratio_, min_font_zoom_ );
    }

    if (!is_view_center)
    {
        center = center_;
    }
    else
    {
        center = renderer_client_->rect().center();
    }
    center *= view_factor;

    int hor_offset = 0, ver_offset = 0;
    if (renderer_client_->viewport().width() > display_size.width())
    {
        hor_offset = (renderer_client_->viewport().width() - display_size.width()) >> 1;
    }
    if (renderer_client_->viewport().height() > display_size.height())
    {
        ver_offset = (renderer_client_->viewport().height() - display_size.height()) >> 1;
    }

    center -= QPoint( hor_offset, ver_offset );
}

void AdobeRenderConf::updateViewport( const QSize & view_size )
{
    dpdoc::Renderer * renderer = renderer_client_->renderer();

    if ( renderer_client_->isInFontIndexMode() )
    {
        // if it is in reflowable mode and in PDF document, adjust the viewport
        // to appropriate size
        double scale_factor = getScaleFactorByFontIndex( font_ratio_, min_font_zoom_ );
        viewport_ = view_size * scale_factor;
    }

    if ( renderer_client_->viewport() != viewport_ || renderer_client_->needRestore() )
    {
        renderer_client_->setViewport( viewport_ );
    }
}

void AdobeRenderConf::exec()
{
    start();
    bool ret = false;
    renderer_client_->lock();
    if ( renderer_client_->needRestore() )
    {
        // If view needs restore the view context, just restore it and ignore
        // any operation. This is not a good implementation because it breaks
        // the original design of render request( operation based ). But there is
        // not better resolution now. Cause render tasks can be launched at any time
        // and sometimes we must make sure the view context be restored(such as reading
        // history)
        ret = restore();
    }
    else
    {
        // navigate to appint location
        switch (operation_)
        {
        case RENDER_NAVIGATE_TO_LOCATION:
        case RENDER_NAVIGATE_TO_HYPERLINK_TARGET:
        case RENDER_NAVIGATE_PREV_SCREEN:
        case RENDER_NAVIGATE_NEXT_SCREEN:
        case RENDER_NAVIGATE_TO_HIGHLIGHT:
            ret = navigate();
            break;
        case RENDER_NAVIGATE_SCROLLING:
            ret = scroll();
            break;
        case RENDER_ROTATION:
            ret = rotate();
            break;
        case RENDER_SCALE_ZOOM:
            ret = scaleSurface();
            break;
        case RENDER_SCALE_FONT_INDEX:
            ret = scaleFontRatio();
            break;
        case RENDER_SCALE_FONT_SIZE:
            ret = scaleFontSize();
            break;
        case RENDER_THUMBNAIL_SAVE:
        case RENDER_THUMBNAIL_NAVIGATE_TO_LOCATION:
        case RENDER_THUMBNAIL_NEXT_SCREEN:
        case RENDER_THUMBNAIL_PREVIOUS_SCREEN:
            {
                QSize thumb_size;
                double zoom_value = 1.0f;
                ret = scaleThumbnail( thumb_size, zoom_value );
                if ( ret )
                {
                    paintThumbnail( thumb_size, zoom_value );
                }
            }
            break;
        case RENDER_SCALE_AREA:
            ret = scaleArea();
            break;
        case RENDER_UPDATE_BY_MATRIX:
            ret = updateByExtMatrix();
            break;
        case RENDER_SWITCH_PAGE_MODE:
            ret = switchPagingMode();
            break;
        case RENDER_RESTORE:
            ret = restore();
            break;
        default:
            break;
        }
    }

    if (ret && status() == TASK_RUN )
    {
        dpdoc::Renderer * renderer = renderer_client_->renderer();
        assert( renderer != 0 );
        renderer->getNavigationMatrix( ext_nav_matrix_.get() );

        // Paint the destination
        if ( operation_ != RENDER_THUMBNAIL_SAVE &&
             operation_ != RENDER_THUMBNAIL_NAVIGATE_TO_LOCATION &&
             operation_ != RENDER_THUMBNAIL_NEXT_SCREEN &&
             operation_ != RENDER_THUMBNAIL_PREVIOUS_SCREEN )
        {
            // generate render range (for memory control)
            // disable it because the cache is not used now
            if ( g_enable_cache )
            {
                generateRenderRange();
            }
            paintSurface();
        }
    }

    if ( sys::SysStatus::instance().isSystemBusy() )
    {
        // if it is the first time rendering, set busy to be false
        sys::SysStatus::instance().setSystemBusy( false );
    }

    if (renderer_client_->needRestore())
    {
        renderer_client_->setRestoreFinished( true );
    }
    renderer_client_->unlock();
    abort();
}

void AdobeRenderConf::paintThumbnail( const QSize & thumb_size, const double & zoom_value )
{
    renderer_client_->repaintThumbnail( QRect( QPoint(0, 0), thumb_size ), zoom_value );
}

void AdobeRenderConf::paintSurface()
{
    dpdoc::Renderer * renderer = renderer_client_->renderer();
    assert( renderer != 0 );

    if ( !g_enable_cache ||
         ( renderer->getPagingMode() == dpdoc::PM_SCROLL_PAGES ||
           operation_ == RENDER_NAVIGATE_SCROLLING ||
           operation_ == RENDER_SCALE_FONT_INDEX ||
           operation_ == RENDER_SCALE_FONT_SIZE ||
           zoom_setting_ == ZOOM_HIDE_MARGIN ) )
    {
        // Do not cache anything now
        QRect painting_area = renderer_client_->displayArea();
        renderer_client_->repaintCurrentSurface( painting_area, this );
        if ( status() == TASK_RUN )
        {
            renderer_client_->updateRenderConf( *this );
        }
        return;
    }

    // get the cached surface if it exists
    AdobeMemController * mem_ctrl = renderer_client_->memoryController();
    if ( mem_ctrl == 0 )
    {
        return;
    }

    AdobeSurfacePtr surface = mem_ctrl->getSurface( *this );
    if (surface != 0)//&& surface->renderRequest() == *this )
    {
        renderer_client_->requestRepaint( surface );
        renderer_client_->updateRenderConf( *this );
        return;
    }

    // make enough memory for the surface
    if ( !mem_ctrl->makeEnoughMemory( AdobeSurface::estimateLength( renderer_client_->displayArea().size(),
                                                                    IMG_FORMAT ),
                                      page_position_ ) )
    {
        return;
    }

    // paint content on the surface
    surface.reset( new AdobeSurface( *this, renderer_client_->displayArea().size() ) );
    renderer->paint( 0,
                     0,
                     renderer_client_->displayArea().width(),
                     renderer_client_->displayArea().height(),
                     surface->data() );
    if ( status() == TASK_RUN )
    {
        // update the displaying area of view
        renderer_client_->requestRepaint( surface );
        renderer_client_->updateRenderConf( *this );
        mem_ctrl->addSurface( surface );
    }
}

void AdobeRenderConf::generateRenderRange()
{
    // get the cached surface if it exists
    AdobeMemController * mem_ctrl = renderer_client_->memoryController();
    if ( mem_ctrl == 0 ) return;

    AdobeDocumentClient * document = renderer_client_->document();
    if ( document != 0 ) return;

    AdobeLocationPtr cur_loc = getCurrentLocation(renderer_client_);
    int cur_page_number = static_cast<int>(cur_loc->getPagePosition());
    int next_page_number = static_cast<int>(page_position_->getPagePosition());
    int page_count = static_cast<int>( document->getPageCount() );

    QVector<int> render_range;
    mem_ctrl->renderPolicy()->getRenderRequests( next_page_number,
                                                 cur_page_number,
                                                 page_count,
                                                 render_range );
}

bool AdobeRenderConf::navigate()
{
    dpdoc::Renderer * renderer = renderer_client_->renderer();
    AdobeDocumentClient * document = renderer_client_->document();
    if ( renderer == 0 || document == 0 )
    {
        return false;
    }

    AdobeLocationPtr cur_loc_ptr = getCurrentLocation(renderer_client_);
    // update the minimal zoom factor
    if ( renderer_client_->viewport() != viewport_ )
    {
        updateMinFontZoomFactor();
    }

    // TODO. In epub document, system notice heap corrupted when exist program.
    // I found it happens when the navigation functions are called.
    switch (operation_)
    {
    case RENDER_NAVIGATE_TO_LOCATION:
    case RENDER_NAVIGATE_TO_HYPERLINK_TARGET:
        {
            renderer->navigateToLocation( page_position_->getData() );
        }
        break;
    case RENDER_NAVIGATE_PREV_SCREEN:
        {
            if (renderer->previousScreen())
            {
                AdobeLocationPtr new_loc_ptr = getCurrentLocation(renderer_client_);
                if ( cur_loc_ptr < new_loc_ptr )
                {
                    double page_number = cur_loc_ptr->getPagePosition() - 1.0;
                    if ( page_number < 0 )
                    {
                        return false;
                    }

                    AdobeLocationPtr prev_page_loc = document->getLocationFromPagePosition( page_number );
                    if ( prev_page_loc == 0 )
                    {
                        return false;
                    }
                    new_loc_ptr = prev_page_loc;
                    renderer->navigateToLocation( new_loc_ptr->getData() );
                }
            }
            else
            {
                return false;
            }
        }
        break;
    case RENDER_NAVIGATE_NEXT_SCREEN:
        {
            if ( renderer->nextScreen() )
            {
                AdobeLocationPtr new_loc_ptr = getCurrentLocation(renderer_client_);
                if ( cur_loc_ptr > new_loc_ptr )
                {
                    double page_number = cur_loc_ptr->getPagePosition() + 1.0;
                    if ( page_number >= document->getPageCount() )
                    {
                        return false;
                    }

                    AdobeLocationPtr next_page_loc = document->getLocationFromPagePosition( page_number );
                    if ( next_page_loc == 0 )
                    {
                        return false;
                    }
                    new_loc_ptr = next_page_loc;
                    renderer->navigateToLocation( new_loc_ptr->getData() );
                }
            }
            else
            {
                return false;
            }
        }
        break;
    default:
        break;
    }

    if (status() != TASK_RUN)
    {
        return false;
    }

    page_position_ = getCurrentLocation(renderer_client_);
    if ( !renderer_client_->isInFontIndexMode() )
    {
        if ( operation_ != RENDER_NAVIGATE_TO_HYPERLINK_TARGET )
        {
            // if it is in hard page mode, we cannot use force scaling for
            // hyperlink target
            return scaleSurface();
        }
    }
    else
    {
        return scaleFontRatio();
    }
    return true;
}

bool AdobeRenderConf::updateByExtMatrix()
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();
    dpdoc::Matrix nav_matrix;
    renderer->getNavigationMatrix( &nav_matrix );
    nav_matrix = *ext_nav_matrix_;

    updateViewport( renderer_client_->displayArea().size() );
    renderer->setNavigationMatrix(nav_matrix);
    return true;
}

bool AdobeRenderConf::switchPagingMode()
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();
    int current_mode = renderer->getPagingMode();
    if ( current_mode == paging_mode_ )
    {
        return false;
    }

    if ( paging_mode_ == dpdoc::PM_FLOW_PAGES )
    {
        updateMinFontZoomFactor();
    }

    renderer->setPagingMode( paging_mode_ );

    // clear the environment when switching paging mode
    env_matrix_.reset(new dpdoc::Matrix);
    if (renderer_client_->isInFontIndexMode())
    {
        return scaleFontRatio();
    }
    return scaleSurface();
}

void AdobeRenderConf::updateMinFontZoomFactor()
{
    if ( renderer_client_->document()->type() == FIX_PAGE_DOCUMENT )
    {
        QSize nature_size;
        getPageNatureSize( nature_size );
        min_font_zoom_ = realZoom( nature_size,
                                   renderer_client_->displayArea().size(),
                                   ZOOM_TO_PAGE );
    }
}

bool AdobeRenderConf::restore()
{
    // NOTE: Do NOT check the return value of any viewport settings
    // otherwise it causes unknown errors.
    dpdoc::Renderer *renderer = renderer_client_->renderer();
    if ( paging_mode_ == dpdoc::PM_FLOW_PAGES )
    {
        updateMinFontZoomFactor();
    }

    // restore paging mode
    if ( paging_mode_ != renderer->getPagingMode() )
    {
        renderer->setPagingMode( paging_mode_ );
    }

    if ( renderer_client_->isInFontIndexMode() )
    {
        // restore font index
        scaleFontRatio();

        // restore font size
        scaleFontSize();
    }
    else
    {
        bool keep_same_viewport = ( viewport_ == renderer_client_->displayArea().size() );
        if ( keep_same_viewport )
        {
            // update viewport directly if same viewport is kept
            updateViewport( renderer_client_->displayArea().size() );
        }

        // restore navigation matrix
        renderer->setNavigationMatrix( *ext_nav_matrix_ );

        // restore environment matrix
        renderer->setEnvironmentMatrix( *env_matrix_ );

        if (!keep_same_viewport)
        {
            // scale surface if viewport changes
            scaleSurface();

            // ends restore process here because the content area should be rescaled in hide margin mode
            renderer_client_->setRestoreFinished(true);
        }
    }

    if (paging_mode_ != dpdoc::PM_SCROLL_PAGES)
    {
        // restore location in hard page or reflow modes
        renderer->navigateToLocation(page_position_->getData());
    }
    return true;
}

bool AdobeRenderConf::isFitForScreen()
{
    return (fabs( env_matrix_->e ) > ZOOM_ERR && fabs( env_matrix_->f ) > ZOOM_ERR);
}

AdobeViewportLocations AdobeRenderConf::viewportLocation()
{
    AdobeViewportLocations locations = NO_SPACE;
    dpdoc::Renderer *renderer = renderer_client_->renderer();

    // navigation matrix
    dpdoc::Matrix nav_matrix;
    renderer->getNavigationMatrix( &nav_matrix );

    // global nature size
    QSize global_nature_size;
    getGlobalNatureSize( global_nature_size );

    QRectF display_area;
    calcDisplayArea(nav_matrix, global_nature_size, display_area);

    QSizeF origin_viewport_size = renderer_client_->viewport();
    if (display_area.width() < origin_viewport_size.width())
    {
        origin_viewport_size.setWidth(display_area.width());
    }
    if (display_area.height() < origin_viewport_size.height())
    {
        origin_viewport_size.setHeight(display_area.height());
    }

    if (nav_matrix.e < -BOUNDARY_ERROR)
    {
        locations = locations | LEFT_SPACE;
    }
    if (nav_matrix.f < -BOUNDARY_ERROR)
    {
        locations = locations | UP_SPACE;
    }
    if (origin_viewport_size.width() - nav_matrix.e < (display_area.width() - BOUNDARY_ERROR))
    {
        locations = locations | RIGHT_SPACE;
    }
    if (origin_viewport_size.height() - nav_matrix.f < (display_area.height() - BOUNDARY_ERROR))
    {
        locations = locations | BOTTOM_SPACE;
    }
    return locations;
}

bool AdobeRenderConf::scaleThumbnail( QSize & thumb_size, double & zoom_value )
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();
    if (renderer->getPagingMode() != dpdoc::PM_HARD_PAGES)
    {
        renderer->setPagingMode( dpdoc::PM_HARD_PAGES );
    }

    updateViewport( viewport_ );

    if (operation_ == RENDER_THUMBNAIL_SAVE || operation_ == RENDER_THUMBNAIL_NAVIGATE_TO_LOCATION)
    {
        renderer->navigateToLocation( page_position_->getData() );
    }
    else if (operation_ == RENDER_THUMBNAIL_NEXT_SCREEN)
    {
        AdobeLocationPtr current_position = renderer_client_->getCurrentLocation();
        if (!(current_position == page_position_))
        {
            renderer->navigateToLocation( page_position_->getData() );
        }
        if (!renderer->nextScreen())
        {
            return false;
        }
    }
    else if (operation_ == RENDER_THUMBNAIL_PREVIOUS_SCREEN)
    {
        AdobeLocationPtr current_position = renderer_client_->getCurrentLocation();
        if (!(current_position == page_position_))
        {
            renderer->navigateToLocation( page_position_->getData() );
        }
        if (!renderer->previousScreen())
        {
            return false;
        }
    }

    // update page position
    page_position_ = getCurrentLocation(renderer_client_);

    // TODO. Refine this calculation
    dpdoc::Matrix nav_matrix;
    QSize page_nature_size;
    getPageNatureSize( page_nature_size );

    // caculate the zoom value by display area of view
    double real_zoom = realZoom( page_nature_size, thumbnail_size_, ZOOM_TO_PAGE );
    if (real_zoom <= 0.0f)
    {
        qDebug("Invalid Zoom Value\n");
        return false;
    }
    zoom_value = real_zoom;

    thumb_size = page_nature_size * real_zoom;
    nav_matrix = multiple( nav_matrix, real_zoom );
    renderer->setNavigationMatrix( nav_matrix );
    renderer->setEnvironmentMatrix( dpdoc::Matrix() );
    return true;
}

bool AdobeRenderConf::scaleSurface()
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();

    if ( renderer->getPagingMode() != paging_mode_ )
    {
        renderer->setPagingMode( paging_mode_ );
    }

    dpdoc::Matrix nav_matrix;
    renderer->getNavigationMatrix( &nav_matrix );

    // TODO. Refine this calculation
    QSize page_nature_size;
    getPageNatureSize( page_nature_size );

    QRect painting_area = renderer_client_->displayArea();
    // caculate the zoom value by display area of view
    double real_zoom = realZoom( page_nature_size,
                                 painting_area.size(),
                                 zoom_setting_ );
    if (real_zoom <= 0.0f)
    {
        qDebug("Invalid Zoom Value\n");
        return false;
    }

    double cur_zoom = fabs(nav_matrix.a);
    cur_zoom = std::max(cur_zoom, fabs(nav_matrix.b));
    cur_zoom = std::max(cur_zoom, fabs(nav_matrix.c));
    cur_zoom = std::max(cur_zoom, fabs(nav_matrix.d));

    // get the center point for always centering the content
    QSize display_size;
    QPoint center;
    QPointF center_in_content( nav_matrix.e, nav_matrix.f );
    getDisplaySize( cur_zoom, page_nature_size, display_size );
    getCenter( display_size, true, center );
    center_in_content -= center;

    // update the content area
    double updated_zoom = real_zoom;
    if (cur_zoom > ZOOM_ERR)
    {
        updated_zoom /= cur_zoom;
    }

    nav_matrix = multiple( nav_matrix, updated_zoom );

    // update the position by center point
    center_in_content *= updated_zoom;
    getDisplaySize( real_zoom, page_nature_size, display_size );
    getCenter( display_size, true, center );
    center_in_content += center;
    nav_matrix.e = center_in_content.x();
    nav_matrix.f = center_in_content.y();

    // update viewport
    viewport_ = painting_area.size();
    updateViewport( viewport_ );

    // update the page position
    page_position_ = getCurrentLocation(renderer_client_);

    if (checkBoundary(&nav_matrix) && translate())
    {
        renderer->setNavigationMatrix(nav_matrix);
        return true;
    }
    return false;
}

bool AdobeRenderConf::scaleArea()
{
    if (zoom_setting_ <= 0)
    {
        return false;
    }

    dpdoc::Renderer *renderer = renderer_client_->renderer();

    dpdoc::Matrix nav_matrix;
    renderer->getNavigationMatrix( &nav_matrix );

    // TODO. Refine this calculation
    QSize page_nature_size;
    getPageNatureSize( page_nature_size );

    double cur_zoom = fabs(nav_matrix.a);
    cur_zoom = std::max(cur_zoom, fabs(nav_matrix.b));
    cur_zoom = std::max(cur_zoom, fabs(nav_matrix.c));
    cur_zoom = std::max(cur_zoom, fabs(nav_matrix.d));

    double real_zoom = zoom_setting_ * cur_zoom / ZOOM_ACTUAL;
    static double max = ZOOM_MAX / ZOOM_ACTUAL;
    if (real_zoom > max)
    {
        real_zoom = max;
    }

    // update the zoom_setting_ to be the real zoom value
    zoom_setting_ = real_zoom * ZOOM_ACTUAL;

    // get the center point for always centering the content
    QSize display_size;
    QPoint center;
    QPointF center_in_content( nav_matrix.e, nav_matrix.f );
    getDisplaySize( cur_zoom, page_nature_size, display_size );
    getCenter( display_size, false, center );
    center_in_content -= center;

    // update the content area
    double updated_zoom = real_zoom;
    if (cur_zoom > ZOOM_ERR)
    {
        updated_zoom /= cur_zoom;
    }

    nav_matrix = multiple( nav_matrix, updated_zoom );

    // update the position by center point
    center_in_content *= updated_zoom;
    getDisplaySize( real_zoom, page_nature_size, display_size );
    getCenter( display_size, true, center );
    center_in_content += center;
    nav_matrix.e = center_in_content.x();
    nav_matrix.f = center_in_content.y();

    // update the viewport
    updateViewport( renderer_client_->displayArea().size() );

    // update the page position
    page_position_ = getCurrentLocation(renderer_client_);

    if (checkBoundary(&nav_matrix) && translate())
    {
        // record the zoom setting, for page navigation
        zoom_setting_ = real_zoom * ZOOM_ACTUAL;
        renderer->setNavigationMatrix(nav_matrix);
        return true;
    }
    return false;
}

bool AdobeRenderConf::scaleFontRatio()
{
    updateViewport( renderer_client_->displayArea().size() );
    restoreDisplayAreaForFontIndex( font_ratio_ * min_font_zoom_ );
    return true;
}

bool AdobeRenderConf::scaleFontSize()
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();
    if (font_size_ >= DEFAULT_FONT_SIZE)
    {
        updateViewport( renderer_client_->displayArea().size() );
        renderer->setDefaultFontSize( font_size_ );
        return true;
    }
    return false;
}

void AdobeRenderConf::restoreDisplayAreaForFontIndex( double zoom )
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();

    // TODO. update the navigation matrix to be centered
    dpdoc::Matrix nav_matrix;
    dpdoc::Matrix prev_matrix;
    renderer->getNavigationMatrix(&prev_matrix);
    if ( !( nav_matrix == prev_matrix ) )
    {
        renderer->setNavigationMatrix( nav_matrix );
    }

    dpdoc::Matrix env_matrix;
    env_matrix = multiple( env_matrix, zoom );
    if ( !( *env_matrix_ == env_matrix ) || renderer_client_->needRestore() || operation_ == RENDER_RESTORE )
    {
        *env_matrix_ = env_matrix;
        renderer->setEnvironmentMatrix( *env_matrix_ );
    }
}

bool AdobeRenderConf::rotate()
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();
    dpdoc::Matrix nav_matrix;
    //renderer->getNavigationMatrix( &nav_matrix );

    QSize page_nature_size;
    getPageNatureSize( page_nature_size );

    // still need scale before ratation
    double real_zoom = realZoom( page_nature_size,
                                 renderer_client_->displayArea().size(),
                                 zoom_setting_ );
    if (real_zoom <= 0.0f)
    {
        return false;
    }

    nav_matrix = multiple( nav_matrix, real_zoom );

    // rotate
    nav_matrix.e = 0.0f;
    nav_matrix.f = 0.0f;

    double cos_v = cos( rotate_degree_ * PI / 180 );
    if (fabs(cos_v) < ZOOM_ERR)
    {
        cos_v = 0;
    }

    double sin_v = sin( rotate_degree_ * PI / 180 );
    if (fabs(sin_v) < ZOOM_ERR)
    {
        sin_v = 0;
    }

    int rotate_offset_width = 0;
    int rotate_offset_height = 0;
    switch (rotate_degree_)
    {
        case 0:
            rotate_offset_width = 0;
            rotate_offset_height = 0;
            break;
        case 90:
            rotate_offset_width = page_nature_size.height();
            rotate_offset_height = 0;
            break;
        case 180:
            rotate_offset_width = page_nature_size.width();
            rotate_offset_height = page_nature_size.height();
            break;
        case 270:
            rotate_offset_width = 0;
            rotate_offset_height = page_nature_size.width();
            break;
        default:
            break;
    }

    dpdoc::Matrix rotate_matrix( cos_v,
                                 sin_v,
                                 -sin_v,
                                 cos_v,
                                 rotate_offset_width,
                                 rotate_offset_height );
    nav_matrix = concat( nav_matrix, rotate_matrix );

    if (translate() && checkBoundary(&nav_matrix))
    {
        renderer->setNavigationMatrix(nav_matrix);
        return true;
    }
    return false;
}

bool AdobeRenderConf::scroll()
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();

    dpdoc::Matrix nav_matrix;
    renderer->getNavigationMatrix( &nav_matrix );

    // translate
    nav_matrix.e -= scroll_offset_x_;
    nav_matrix.f -= scroll_offset_y_;

    // check bounds
    if (checkBoundary(&nav_matrix))
    {
        renderer->setNavigationMatrix(nav_matrix);
        if (zoom_setting_ == ZOOM_HIDE_MARGIN)
        {
            return scaleSurface();
        }
        page_position_ = getCurrentLocation(renderer_client_);
        return true;
    }
    return false;
}

bool AdobeRenderConf::checkBoundary(dpdoc::Matrix * nav_matrix)
{
    // get the global nature size
    QSize global_nature_size;
    getGlobalNatureSize( global_nature_size );

    QRectF display_area;
    calcDisplayArea(*nav_matrix, global_nature_size, display_area);

    QSizeF origin_viewport_size = renderer_client_->viewport();
    if (display_area.width() < origin_viewport_size.width())
    {
        origin_viewport_size.setWidth(display_area.width());
    }
    if (display_area.height() < origin_viewport_size.height())
    {
        origin_viewport_size.setHeight(display_area.height());
    }

    QRectF origin_viewport = QRectF( QPointF(0, 0), origin_viewport_size );
    QRectF viewport = display_area.intersect(origin_viewport);
    if (viewport.width() < origin_viewport.width())
    {
        // need adjust the e value
        qDebug("need adjust e value");

        if ( viewport.width() <= 0 )
        {
            // no overlap, just set nav_matrix.e = 0
            nav_matrix->e = 0;
        }
        else
        {
            double offset_e = origin_viewport.width() - viewport.width();
            if (display_area.left() > 0)
            {
                offset_e *= -1.0f;
            }
            nav_matrix->e += offset_e;
        }
    }

    if (viewport.height() < origin_viewport.height())
    {
        // need adjust the f value
        qDebug("need adjust f value");

        double offset_f = origin_viewport.height() - viewport.height();
        if (display_area.top() > 0)
        {
            offset_f *= -1.0f;
        }
        nav_matrix->f += offset_f;
    }
    return true;
}

bool AdobeRenderConf::translate()
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();

    // TODO. update the navigation matrix to be centered
    if ( !renderer_client_->isInFontIndexMode() || operation_ == RENDER_ROTATION )
    {
        env_matrix_.reset(new dpdoc::Matrix);
    }

    QSize page_nature_size;
    getPageNatureSize( page_nature_size) ;
    double real_zoom = realZoom( page_nature_size,
                                 renderer_client_->viewport(),
                                 zoom_setting_ );

    QSize display_size;
    if (getDisplaySize(real_zoom, page_nature_size, display_size))
    {
        if (renderer_client_->viewport().width() > display_size.width())
        {
            env_matrix_->e = (renderer_client_->viewport().width() - display_size.width()) >> 1;
        }

        if (renderer_client_->viewport().height() > display_size.height())
        {
            env_matrix_->f = (renderer_client_->viewport().height() - display_size.height()) >> 1;
        }
    }
    renderer->setEnvironmentMatrix( *env_matrix_ );
    return true;
}

void AdobeRenderConf::getGlobalNatureSize( QSize & nature_size )
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();
    dpdoc::Rectangle dimensions;
    renderer->getNaturalSize( &dimensions );
    nature_size.setWidth( static_cast<int>( dimensions.xMax - dimensions.xMin ) );
    nature_size.setHeight( static_cast<int>( dimensions.yMax - dimensions.yMin ) );
    return;
}

void AdobeRenderConf::getPageNatureSize( QSize & nature_size )
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();
    int page_mode = renderer->getPagingMode();
    if ( page_mode != dpdoc::PM_HARD_PAGES )
    {
        renderer->setPagingMode( dpdoc::PM_HARD_PAGES );
    }

    dpdoc::Rectangle dimensions;
    renderer->getNaturalSize( &dimensions );
    nature_size.setWidth( static_cast<int>( dimensions.xMax - dimensions.xMin ) );
    nature_size.setHeight( static_cast<int>( dimensions.yMax - dimensions.yMin ) );

    if (page_mode != renderer->getPagingMode())
    {
        renderer->setPagingMode( page_mode );
    }
    return;
}

bool AdobeRenderConf::getDisplaySize( double real_zoom,
                                      QSize & nature_size,
                                      QSize & display_size ) const
{
    if (real_zoom < 0)
    {
        return false;
    }

    switch (rotate_degree_)
    {
    case 0:
    case 180:
        display_size.setWidth( static_cast<int>( real_zoom * nature_size.width() ) );
        display_size.setHeight( static_cast<int>( real_zoom * nature_size.height() ) );
        break;
    case 270:
    case 90:
        display_size.setWidth( static_cast<int>( real_zoom * nature_size.height() ) );
        display_size.setHeight( static_cast<int>( real_zoom * nature_size.width() ) );
        break;
    default:
        return false;
    }
    return true;
}

void AdobeRenderConf::exportToConfiguration( Configuration & conf )
{
    conf.options[CONFIG_PAGING_MODE]  = paging_mode_;

    AdobeLocationPtr current_location = renderer_client_->getScreenBeginning();
    if (current_location == 0)
    {
        current_location = renderer_client_->getCurrentLocation();
    }
    dp::String bookmark = current_location->getData()->getBookmark();
    QString bookmark_str(bookmark.utf8());
    conf.options[CONFIG_DOC_LOCATION] = bookmark_str;

    conf.options[CONFIG_FONT_RATIO] = font_ratio_;
    conf.options[CONFIG_FONT_SIZE]  = font_size_;
    conf.options[CONFIG_VIEW_PORT]  = viewport_;

    conf.options[CONFIG_ZOOM_SETTING] = zoom_setting_;
    if (renderer_client_->zoomingMode() == ZOOM_HIDE_MARGIN)
    {
        conf.options[CONFIG_ZOOM_SETTING] = ZOOM_HIDE_MARGIN;
    }

    dpdoc::Renderer *renderer = renderer_client_->renderer();
    assert( renderer != 0 );
    dpdoc::Matrix nav_matrix;
    renderer->getNavigationMatrix( &nav_matrix );
    conf.options[CONFIG_NAV_MATRIX_A] = nav_matrix.a;
    conf.options[CONFIG_NAV_MATRIX_B] = nav_matrix.b;
    conf.options[CONFIG_NAV_MATRIX_C] = nav_matrix.c;
    conf.options[CONFIG_NAV_MATRIX_D] = nav_matrix.d;
    conf.options[CONFIG_NAV_MATRIX_E] = nav_matrix.e;
    conf.options[CONFIG_NAV_MATRIX_F] = nav_matrix.f;

    conf.options[CONFIG_ENV_MATRIX_A] = env_matrix_->a;
    conf.options[CONFIG_ENV_MATRIX_B] = env_matrix_->b;
    conf.options[CONFIG_ENV_MATRIX_C] = env_matrix_->c;
    conf.options[CONFIG_ENV_MATRIX_D] = env_matrix_->d;
    conf.options[CONFIG_ENV_MATRIX_E] = env_matrix_->e;
    conf.options[CONFIG_ENV_MATRIX_F] = env_matrix_->f;
}

bool AdobeRenderConf::loadFromConfiguration( const Configuration & conf )
{
    AdobeDocumentClient * document = renderer_client_->document();
    assert( document != 0 );

    // operation
    operation_ = RENDER_NAVIGATE_TO_LOCATION;

    // paging mode
    bool ret = true;
    bool ok = false;
    paging_mode_ = conf.options[CONFIG_PAGING_MODE].toInt( &ok );
    if ( !ok )
    {
        paging_mode_ = dpdoc::PM_HARD_PAGES;
        ret = false;
    }

    // location
    QString loc_bookmark = conf.options[CONFIG_DOC_LOCATION].toString();
    if ( loc_bookmark.isEmpty() )
    {
        //page_position_ = document->getBeginning();
        page_position_ = getCurrentLocation(renderer_client_);
        ret = false;
    }
    else
    {
        page_position_ = document->getLocationFromBookmark( loc_bookmark.toUtf8().data() );
    }

    // font ratio
    font_ratio_ = conf.options[CONFIG_FONT_RATIO].toDouble( &ok );
    if ( !ok )
    {
        font_ratio_ = 1.0f;
        ret = false;
    }

    // font size
    font_size_ = conf.options[CONFIG_FONT_SIZE].toDouble( &ok );
    if ( !ok )
    {
        font_size_ = DEFAULT_FONT_SIZE;
        ret = false;
    }

    // zoom setting
    zoom_setting_ = conf.options[CONFIG_ZOOM_SETTING].toDouble( &ok );
    if ( !ok )
    {
        zoom_setting_ = ZOOM_TO_PAGE;
        ret = false;
    }

    // view port
    viewport_ = conf.options[CONFIG_VIEW_PORT].toSize();
    if ( viewport_.isEmpty() )
    {
        viewport_ = renderer_client_->viewport();
        ret = false;
    }

    // navigation matrix
    ext_nav_matrix_->a = conf.options[CONFIG_NAV_MATRIX_A].toDouble( &ok );
    if ( !ok )
    {
        ext_nav_matrix_->a = 1.0f;
        ret = false;
    }
    ext_nav_matrix_->b = conf.options[CONFIG_NAV_MATRIX_B].toDouble( &ok );
    if ( !ok )
    {
        ext_nav_matrix_->b = 0.0f;
        ret = false;
    }
    ext_nav_matrix_->c = conf.options[CONFIG_NAV_MATRIX_C].toDouble( &ok );
    if ( !ok )
    {
        ext_nav_matrix_->c = 0.0f;
        ret = false;
    }
    ext_nav_matrix_->d = conf.options[CONFIG_NAV_MATRIX_D].toDouble( &ok );
    if ( !ok )
    {
        ext_nav_matrix_->d = 1.0f;
        ret = false;
    }
    ext_nav_matrix_->e = conf.options[CONFIG_NAV_MATRIX_E].toDouble( &ok );
    if ( !ok )
    {
        ext_nav_matrix_->e = 0.0f;
        ret = false;
    }
    ext_nav_matrix_->f = conf.options[CONFIG_NAV_MATRIX_F].toDouble( &ok );
    if ( !ok )
    {
        ext_nav_matrix_->f = 0.0f;
        ret = false;
    }

    // environment matrix
    env_matrix_->a = conf.options[CONFIG_ENV_MATRIX_A].toDouble( &ok );
    if ( !ok )
    {
        env_matrix_->a = 1.0f;
        ret = false;
    }
    env_matrix_->b = conf.options[CONFIG_ENV_MATRIX_B].toDouble( &ok );
    if ( !ok )
    {
        env_matrix_->b = 0.0f;
        ret = false;
    }
    env_matrix_->c = conf.options[CONFIG_ENV_MATRIX_C].toDouble( &ok );
    if ( !ok )
    {
        env_matrix_->c = 0.0f;
        ret = false;
    }
    env_matrix_->d = conf.options[CONFIG_ENV_MATRIX_D].toDouble( &ok );
    if ( !ok )
    {
        env_matrix_->d = 1.0f;
        ret = false;
    }
    env_matrix_->e = conf.options[CONFIG_ENV_MATRIX_E].toDouble( &ok );
    if ( !ok )
    {
        env_matrix_->e = 0.0f;
        ret = false;
    }
    env_matrix_->f = conf.options[CONFIG_ENV_MATRIX_F].toDouble( &ok );
    if ( !ok )
    {
        env_matrix_->f = 0.0f;
        ret = false;
    }

    return ret;
}

}
