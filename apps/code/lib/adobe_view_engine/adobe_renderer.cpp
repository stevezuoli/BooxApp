#include "dp_all.h"
#include "adobe_renderer.h"

using namespace ui;
using namespace base;

namespace adobe_view
{

static const int OVERLAP_DISTANCE = 80;

static void resetPageDecoration( dpdoc::Renderer * renderer )
{
    dpdoc::PageDecoration decoration;
    decoration.pageGap     = 0;
    decoration.borderWidth = 0;
    decoration.shadowColor = 0;
    decoration.shadowWidth = 0;

    renderer->setPageDecoration( decoration );
}

class AdobeRendererClientPrivate : public dpdoc::RendererClient
{
public:
    AdobeRendererClientPrivate(AdobeRendererClient *host);
    virtual ~AdobeRendererClientPrivate();

    virtual void * getOptionalInterface( const char * name );
    virtual int getInterfaceVersion();
    virtual double getUnitsPerInch();
    virtual void requestRepaint( int x_min, int y_min, int x_max, int y_max );
    virtual void navigateToURL( const dp::String& url, const dp::String& target );
    virtual void reportMouseLocationInfo( const dpdoc::MouseLocationInfo& info );
    virtual void reportInternalNavigation();
    virtual void reportDocumentSizeChange();
    virtual void reportHighlightChange( int highlightType );
    virtual void reportRendererError(const dp::String& errorString);
    virtual void finishedPlaying();

    bool initializeByDocument(dpdoc::Document * document);

    inline dpdoc::Renderer* renderer() { return renderer_; }
    inline void enableRepainting(bool enable) { repaint_enabled_ = enable; }
    inline dpdoc::Matrix & lastNavigationMatrix() { return last_nav_matrix_; }
    inline void clear();

private:
    AdobeRendererClient *host_;
    dpdoc::Renderer     *renderer_;
    QRect               dirty_area_;
    dpdoc::Matrix       last_nav_matrix_;
    bool                repaint_enabled_;
};

AdobeRendererClientPrivate::AdobeRendererClientPrivate(AdobeRendererClient *host)
    : host_(host)
    , renderer_(0)
    , repaint_enabled_(false)
{
}

AdobeRendererClientPrivate::~AdobeRendererClientPrivate()
{
}

void AdobeRendererClientPrivate::clear()
{
    if (renderer_ != 0)
    {
        renderer_->release();
        renderer_ = 0;
    }
}

void * AdobeRendererClientPrivate::getOptionalInterface( const char * name )
{
    return 0;
}

int AdobeRendererClientPrivate::getInterfaceVersion()
{
    return 1;
}

double AdobeRendererClientPrivate::getUnitsPerInch()
{
    // Important Comments:
    // Actually the environment matrix and viewport should be calculated by current DPI.
    // That means these two values should be set by the pixels per inch.
    // ppi = UPI / 72;
    // Viewport /= ppi;
    // Enviroment Matrix *= ppi.
    // However, the surface is always rendered by both viewport and environment matrix.
    // The effect of ppi to these two values are counteracted. So we can set the viewport
    // by the view size and calculate the environment matrix by real scaling directly.
    return 96;
}

void AdobeRendererClientPrivate::requestRepaint( int x_min, int y_min, int x_max, int y_max )
{
    if (!repaint_enabled_ || renderer_ == 0)
    {
        //qDebug("Rendering is held");
        return;
    }

    if( x_min < 0 )
    {
        x_min = 0;
    }

    if( y_min < 0 )
    {
        y_min = 0;
    }

    if( x_max > host_->displayArea().width() )
    {
        x_max = host_->displayArea().width();
    }

    if( y_max > host_->displayArea().height() )
    {
        y_max = host_->displayArea().height();
    }

    if( x_min < x_max && y_min < y_max )
    {
        if( dirty_area_.left() >= dirty_area_.right() )
        {
            dirty_area_.setLeft( x_min );
            dirty_area_.setRight( x_max );
            dirty_area_.setTop( y_min );
            dirty_area_.setBottom( y_max );

            // DO NOT repaint directly on screen because the layout is usually
            // incorrect. Use render request handler to handle it.
            dp::ref<dpdoc::Location> location = renderer_->getCurrentLocation();
            AdobeLocationPtr loc_ptr( new AdobeLocation( location ) );
            if (!(host_->renderConf().getPagePosition() == loc_ptr))
            {
                host_->gotoPosition( loc_ptr );
            }

            // reset the dirty area
            dirty_area_.setTopLeft( QPoint(0, 0) );
            dirty_area_.setBottomRight( QPoint(-1, -1) );

            // make sure the requestRepaint is executed once per UI operation
            enableRepainting(false);
        }
        else
        {
            if( dirty_area_.left() > x_min )
            {
                dirty_area_.setLeft( x_min );
            }

            if( dirty_area_.top() > y_min )
            {
                dirty_area_.setTop( y_min );
            }

            if( dirty_area_.right() < x_max )
            {
                dirty_area_.setRight( x_max );
            }

            if( dirty_area_.bottom() < y_max )
            {
                dirty_area_.setBottom( y_max );
            }
        }
    }
}

void AdobeRendererClientPrivate::navigateToURL( const dp::String& url, const dp::String& target )
{
    // Navigate to given URL
    qDebug("TODO: Try to navigate to URL: %s, Target: %s", url.utf8(), target.utf8());
}

void AdobeRendererClientPrivate::reportMouseLocationInfo( const dpdoc::MouseLocationInfo& info )
{
    // Report the position information of mouse
    if ( !info.linkURL.isNull() )
    {
        // TODO. Check the internal hyperlink at first
        QString url(info.linkURL.utf8());
        host_->notifyExternalHyperlinkClicked(url);
    }
}

void AdobeRendererClientPrivate::reportInternalNavigation()
{
}

void AdobeRendererClientPrivate::reportDocumentSizeChange()
{
}

void AdobeRendererClientPrivate::reportHighlightChange( int highlightType )
{
}

void AdobeRendererClientPrivate::reportRendererError(const dp::String& errorString)
{
}

void AdobeRendererClientPrivate::finishedPlaying()
{
}

bool AdobeRendererClientPrivate::initializeByDocument(dpdoc::Document * document)
{
    if (renderer_ != 0)
    {
        renderer_->release();
        renderer_ = 0;
    }

    renderer_ = document->createRenderer(this);
    return renderer_ != 0;
}

// AdobeRendererClient
AdobeRendererClient::AdobeRendererClient(QWidget *parent)
    : parent_(parent)
    , renderer_private_(new AdobeRendererClientPrivate(this))
    , document_client_(0)
    , render_conf_(RENDER_NAVIGATE_TO_LOCATION, this)
    , surface_(render_conf_, QSize(600, 800))
    , zooming_mode_(ZOOM_HIDE_MARGIN)
    , need_restore_(false)
    , is_locked_(false)
{
}

AdobeRendererClient::~AdobeRendererClient()
{
}

dpdoc::Renderer* AdobeRendererClient::renderer()
{
    return renderer_private_->renderer();
}

void AdobeRendererClient::notifyExternalHyperlinkClicked(const QString & url)
{
    emit externalHyperlinkClicked(url);
}

void AdobeRendererClient::enableRepaint()
{
    renderer_private_->enableRepainting(true);
}

void AdobeRendererClient::disableRepaint()
{
    renderer_private_->enableRepainting(false);
}

void AdobeRendererClient::attachDocumentClient(AdobeDocumentClient* client)
{
    if (document_client_ == client)
    {
        return;
    }

    document_client_ = client;
    connect(document_client_, SIGNAL(documentReadySignal()), this, SLOT(onDocumentReady()));
    //connect(document_client_, SIGNAL(documentCloseSignal()), this, SLOT(onDocumentClose()));
}

void AdobeRendererClient::deattachDocumentClient()
{
    disconnect(document_client_, SIGNAL(documentReadySignal()), this, SLOT(onDocumentReady()));
    //disconnect(document_client_, SIGNAL(documentCloseSignal()), this, SLOT(onDocumentClose()));
    document_client_ = 0;
}

void AdobeRendererClient::requestRepaint(AdobeSurfacePtr surface)
{
    surface_ = *surface.get();
}

void AdobeRendererClient::repaintCurrentSurface(const QRect & dirty_area, AdobeRenderConf * render_task)
{
    if( dirty_area.isValid() )
    {
        // process all of the pending events before repainting.
        QApplication::processEvents();

        // clear and scale the surface
        if (surface()->image().size() != dirty_area.size())
        {
            surface()->scale( dirty_area.size() );
        }

        surface()->clear( dirty_area );

        bool need_hide_margin = (!need_restore_ && render_conf_.getZoomSetting() == ZOOM_HIDE_MARGIN &&
                                 !isInFontIndexMode());
        renderer()->paint( dirty_area.left(),
                           dirty_area.top(),
                           dirty_area.right(),
                           dirty_area.bottom(),
                           surface_.data() );
        if (need_hide_margin && render_task->status() == TASK_RUN)
        {
            // get content area
            QRect content_area = surface()->getContentArea();
            QPoint position = content_area.topLeft();
            position += dirty_area.topLeft();
            content_area.moveTo(position);

            // update primary render configuration
            render_conf_ = *render_task;
            selectZoom(content_area);
        }
    }
}

void AdobeRendererClient::repaintThumbnail(const QRect & dirty_area, const double & zoom_value)
{
    if( dirty_area.isValid() )
    {
        AdobeSurface thumb_surface( render_conf_, dirty_area.size() );
        thumb_surface.clear( dirty_area );
        renderer()->paint( dirty_area.left(),
                           dirty_area.top(),
                           dirty_area.right(),
                           dirty_area.bottom(),
                           thumb_surface.data() );

        // save the thumbnail
        if (!thumb_surface.image().isNull())
        {
            emit thumbnailReady(thumb_surface.image(), zoom_value);
        }
    }
}

void AdobeRendererClient::updateRenderConf(const AdobeRenderConf & conf)
{
    // update the render request
    render_conf_ = conf;

    if (isInFontIndexMode() || render_conf_.getZoomSetting() != ZOOM_HIDE_MARGIN || need_restore_)
    {
        // notify the host that the rendering configuration has been updated
        emit renderConfigurationUpdated();
    }

    dpdoc::Matrix cur_nav_matrix;
    renderer()->getNavigationMatrix( &cur_nav_matrix );
    if ( renderer_private_->lastNavigationMatrix().a != cur_nav_matrix.a ||
         renderer_private_->lastNavigationMatrix().b != cur_nav_matrix.b ||
         renderer_private_->lastNavigationMatrix().c != cur_nav_matrix.c ||
         renderer_private_->lastNavigationMatrix().d != cur_nav_matrix.d )
    {
        // notify the host that the navigation matrix has been updated
        emit navigationMatrixUpdated();
    }
}

void AdobeRendererClient::setViewport( const QSize & size )
{
    viewport_ = size;
    if (renderer() != 0)
    {
        renderer()->setViewport( viewport_.width(), viewport_.height(), false );
    }
}

AdobeViewportLocations AdobeRendererClient::viewportLocation()
{
    return render_conf_.viewportLocation();
}

QRect AdobeRendererClient::displayArea()
{
    return rect();
}

bool AdobeRendererClient::getScreenRange(Range & range)
{
    if ( renderer() == 0 ) return false;

    dp::ref<dpdoc::Location> screen_begin = renderer()->getScreenBeginning();
    if ( screen_begin == 0 )
    {
        qDebug("Invalid screen begin");
        return false;
    }
    range.start.reset( new AdobeLocation( screen_begin ) );

    dp::ref<dpdoc::Location> screen_end   = renderer()->getScreenEnd();
    if ( screen_end == 0 )
    {
        qDebug("Invalid screen end");
        return false;
    }
    range.end.reset( new AdobeLocation( screen_end ) );
    return true;
}

bool AdobeRendererClient::gotoPosition( AdobeLocationPtr pos )
{
    bool ret = (pos != render_conf_.getPagePosition());
    render_conf_.setOperation( RENDER_NAVIGATE_TO_LOCATION );
    render_conf_.setPagePosition( pos );

    sendRenderRequest( &render_conf_ );
    return ret;
}

bool AdobeRendererClient::navigateToLink( AdobeLocationPtr target )
{
    render_conf_.setOperation( RENDER_NAVIGATE_TO_HYPERLINK_TARGET );
    render_conf_.setPagePosition( target );

    sendRenderRequest( &render_conf_ );
    return true;
}

void AdobeRendererClient::nextScreen()
{
    if (renderer()->getPagingMode() == dpdoc::PM_SCROLL_PAGES && zooming_mode_ != ZOOM_HIDE_MARGIN)
    {
        render_conf_.setOperation(RENDER_NAVIGATE_SCROLLING);
        render_conf_.setScrollOffset(0, (parent_->height() - OVERLAP_DISTANCE));
    }
    else
    {
        // steve: Do NOT check whether the renderer comes to end of document
        // because the function Document::isAtEnd() seems only check the page number
        render_conf_.setOperation(RENDER_NAVIGATE_NEXT_SCREEN);
    }
    sendRenderRequest(&render_conf_);
}

void AdobeRendererClient::prevScreen()
{
    if (renderer()->getPagingMode() == dpdoc::PM_SCROLL_PAGES && zooming_mode_ != ZOOM_HIDE_MARGIN)
    {
        render_conf_.setOperation(RENDER_NAVIGATE_SCROLLING);
        render_conf_.setScrollOffset(0, -(parent_->height() - OVERLAP_DISTANCE));
    }
    else
    {
        render_conf_.setOperation( RENDER_NAVIGATE_PREV_SCREEN );
    }
    sendRenderRequest( &render_conf_ );
}

bool AdobeRendererClient::scaleSurface( double zoom_setting )
{
    bool ret = (zoom_setting != render_conf_.getZoomSetting());
    zooming_mode_ = zoom_setting;
    if (renderer()->getPagingMode() == dpdoc::PM_FLOW_PAGES)
    {
        render_conf_.setViewport( size() );
        render_conf_.setPagingMode( dpdoc::PM_HARD_PAGES );
        render_conf_.setFontRatio( 1.0f );
    }

    render_conf_.setOperation( RENDER_SCALE_ZOOM );
    render_conf_.setZoomSetting( zoom_setting );
    sendRenderRequest( &render_conf_ );
    return ret;
}

bool AdobeRendererClient::switchPagingMode( int paging_mode )
{
    bool ret = (paging_mode != renderer()->getPagingMode());
    render_conf_.setOperation( RENDER_SWITCH_PAGE_MODE );
    render_conf_.setViewport( size() );
    render_conf_.setPagingMode( paging_mode );

    if ( paging_mode == dpdoc::PM_HARD_PAGES )
    {
        if (zooming_mode_ != ZOOM_HIDE_MARGIN)
        {
            // If it is not in hide-margin mode, set to zoom-to-best
            render_conf_.setZoomSetting( ZOOM_TO_PAGE );
        }
        else
        {
            render_conf_.setZoomSetting( ZOOM_HIDE_MARGIN );
        }
        render_conf_.setFontRatio( 1.0f );
    }
    else if ( paging_mode == dpdoc::PM_SCROLL_PAGES )
    {
        if (zooming_mode_ != ZOOM_HIDE_MARGIN)
        {
            // If it is not in scroll-pages mode, set to zoom-to-width
            render_conf_.setZoomSetting( ZOOM_TO_WIDTH );
        }
        else
        {
            render_conf_.setZoomSetting( ZOOM_HIDE_MARGIN );
        }
        render_conf_.setFontRatio( 1.0f );
    }

    sendRenderRequest( &render_conf_ );
    return ret;
}

bool AdobeRendererClient::scaleFontSize( int font_size )
{
    if (font_size < 0)
    {
        qDebug("Invalid Font Size");
        return false;
    }

    bool ret = (font_size != render_conf_.getFontSize());
    render_conf_.setOperation( RENDER_SCALE_FONT_SIZE );
    render_conf_.setFontSize( font_size );
    sendRenderRequest( &render_conf_ );
    return ret;
}

bool AdobeRendererClient::scaleFontRatio( double font_ratio )
{
    bool ret = (font_ratio != render_conf_.getFontRatio());
    if ( document_client_->type() == FIX_PAGE_DOCUMENT )
    {
        if (qAbs(font_ratio - 1.0f) <= 0.01)
        {
            render_conf_.setOperation( RENDER_SWITCH_PAGE_MODE );
            render_conf_.setPagingMode( dpdoc::PM_HARD_PAGES );
            render_conf_.setViewport( size() );
            render_conf_.setZoomSetting( ZOOM_TO_PAGE );
        }
        else if ( renderer()->getPagingMode() != dpdoc::PM_FLOW_PAGES )
        {
            render_conf_.setOperation( RENDER_SWITCH_PAGE_MODE );
            render_conf_.setPagingMode( dpdoc::PM_FLOW_PAGES );
        }
        else
        {
            render_conf_.setOperation( RENDER_SCALE_FONT_INDEX );
        }
    }
    else
    {
        render_conf_.setOperation( RENDER_SCALE_FONT_INDEX );
    }
    render_conf_.setFontRatio( font_ratio );
    sendRenderRequest( &render_conf_ );
    return ret;
}

void AdobeRendererClient::rotate()
{
    // DO NOT use the native rotation now
    if (renderer()->getPagingMode() != dpdoc::PM_HARD_PAGES &&
        renderer()->getPagingMode() != dpdoc::PM_HARD_PAGES_2UP )
    {
        // other mode does not support rotation
        qDebug("Only Hard Page Mode supports rotation");
        return;
    }

    render_conf_.setOperation( RENDER_ROTATION );
    render_conf_.setRotateDegree((render_conf_.getRotateDegree() + 90) % 360);
    sendRenderRequest( &render_conf_ );
}

void AdobeRendererClient::selectZoom( const QRect & area )
{
    // Move viewport to the center of area at first, then zoom to the apt value
    // TODO. Get the overlap area between selection rectangle and display area
    QPoint new_center = area.center();

    // get the zoom factor
    double hor_zoom = static_cast<double>(parent_->width()) / static_cast<double>(area.width());
    double ver_zoom = static_cast<double>(parent_->height()) / static_cast<double>(area.height());
    double zoom = std::min( hor_zoom, ver_zoom ) * ZOOM_ACTUAL;
    if (zooming_mode_ == ZOOM_HIDE_MARGIN && renderer()->getPagingMode() == dpdoc::PM_SCROLL_PAGES)
    {
        zoom = hor_zoom * ZOOM_ACTUAL;
    }
    zooming_mode_ = zoom;

    render_conf_.setOperation( RENDER_SCALE_AREA );
    render_conf_.setCenterPoint( new_center );
    render_conf_.setZoomSetting( zoom );
    sendRenderRequest( &render_conf_ );
}

void AdobeRendererClient::moveViewport( int h_offset, int v_offset )
{
    if ( isInFontIndexMode() )
    {
        qDebug("Not Support Pan in reflowable mode");
        return;
    }

    // move the viewport by rerendering
    render_conf_.setOperation( RENDER_NAVIGATE_SCROLLING );
    render_conf_.setScrollOffset( h_offset, v_offset );
    sendRenderRequest( &render_conf_ );
}

void AdobeRendererClient::saveOptions()
{
    // save reading history
    vbf::Configuration & conf = document_client_->getConf();
    render_conf_.exportToConfiguration(conf);
}

double AdobeRendererClient::getRealZoomFactor()
{
    dpdoc::Matrix nav_matrix;
    renderer()->getNavigationMatrix( &nav_matrix );
    return nav_matrix.a;
}

void AdobeRendererClient::sendRenderRequest( AdobeRenderConf * render_conf )
{
    if (document() != 0 && renderer() != 0)
    {
        // update the last navigation matrix
        dpdoc::Matrix nav_matrix;
        renderer()->getNavigationMatrix(&nav_matrix);
        renderer_private_->lastNavigationMatrix() = nav_matrix;

        if (zooming_mode_ == ZOOM_HIDE_MARGIN && !isInFontIndexMode() &&
            (render_conf->getOperation() >= RENDER_NAVIGATE_TO_LOCATION && render_conf->getOperation() <= RENDER_NAVIGATE_SCROLLING))
        {
            render_conf->setZoomSetting(ZOOM_HIDE_MARGIN);
        }

        AdobeRenderConf *new_request = new AdobeRenderConf( *render_conf );
        document()->tasksHandler()->addTask( new_request, false );
        emit renderRequestSent();
    }
}

bool AdobeRendererClient::isInFontIndexMode()
{
    int doc_type = document_client_->type();
    return ( ( doc_type == FIX_PAGE_DOCUMENT && //renderer_->getPagingMode() == embed::PM_FLOW_PAGES) ||
               render_conf_.getPagingMode() == dpdoc::PM_FLOW_PAGES ) ||
               doc_type == REFLOWABLE_DOCUMENT );
}

bool AdobeRendererClient::isValid()
{
    return renderer() != 0;
}

bool AdobeRendererClient::isAtEnd()
{
    return renderer()->isAtEnd();
}

bool AdobeRendererClient::isAtBeginning()
{

    return renderer()->isAtBeginning();
}

int AdobeRendererClient::getLinkCount()
{
    return renderer()->getLinkCount();
}

bool AdobeRendererClient::getLinkInfo(int link_index, Range & range, AdobeLocationPtr & target)
{
    dpdoc::LinkInfo link_info;
    if ( renderer()->getLinkInfo( link_index, &link_info ) )
    {
        range.start.reset(new AdobeLocation(link_info.beginning));
        range.end.reset(new AdobeLocation(link_info.end));
        target.reset(new AdobeLocation(link_info.target));
        return true;
    }
    return false;
}

AdobeRangeInfo* AdobeRendererClient::getRangeInfo(const Range & from)
{
    AdobeRangeInfo * ret = 0;
    dpdoc::RangeInfo * range_info = renderer()->getRangeInfo(from.start->getData(), from.end->getData());
    if (range_info != 0)
    {
        ret = new AdobeRangeInfo(range_info);
    }
    return ret;
}

int AdobeRendererClient::getPagingMode()
{
    return renderer()->getPagingMode();
}

AdobeLocationPtr AdobeRendererClient::getCurrentLocation()
{
    dp::ref<dpdoc::Location> location = renderer_private_->renderer()->getCurrentLocation();
    if (location != 0)
    {
        AdobeLocationPtr cur_loc(new AdobeLocation(location));
        return cur_loc;
    }
    return AdobeLocationPtr();
}

AdobeLocationPtr AdobeRendererClient::getScreenBeginning()
{
    dp::ref<dpdoc::Location> location = renderer_private_->renderer()->getScreenBeginning();
    if (location != 0)
    {
        AdobeLocationPtr beginning(new AdobeLocation(location));
        return beginning;
    }
    return AdobeLocationPtr();
}

AdobeLocationPtr AdobeRendererClient::getScreenEnd()
{
    dp::ref<dpdoc::Location> location = renderer_private_->renderer()->getScreenEnd();
    if (location != 0)
    {
        AdobeLocationPtr end(new AdobeLocation(location));
        return end;
    }
    return AdobeLocationPtr();
}

void AdobeRendererClient::onDocumentReady()
{
    if (renderer_private_->initializeByDocument(document_client_->document()))
    {
        resetPageDecoration( renderer_private_->renderer() );
        need_restore_ = render_conf_.loadFromConfiguration( document_client_->getConf() );
        if (need_restore_)
        {
            zooming_mode_ = render_conf_.getZoomSetting();
        }

        disableRepaint();
        sendRenderRequest( &render_conf_ );
    }
}

void AdobeRendererClient::onDocumentClose()
{
    // clear all of the cached surfaces
    mem_controller_.clear();
    render_conf_.clear();
    renderer_private_->clear();
}

void AdobeRendererClient::handleMouseEvent(AdobeMouseEvent *e)
{
    renderer()->handleEvent((dpdoc::Event*)e->realEvent());
}

void AdobeRendererClient::handleKeyboardEvent(AdobeKeyboardEvent *e)
{
    renderer()->handleEvent((dpdoc::Event*)e->realEvent());
}

void AdobeRendererClient::handleTextEvent(AdobeTextEvent *e)
{
    renderer()->handleEvent((dpdoc::Event*)e->realEvent());
}

bool AdobeRendererClient::hitTest(const QPoint & pos, unsigned int flags, AdobeLocationPtr & loc_res)
{
    dp::ref<dpdoc::Location> loc = renderer()->hitTest(pos.x(), pos.y(), flags);
    if ( loc != 0)
    {
        loc_res = AdobeLocationPtr(new AdobeLocation(loc));
        return true;
    }
    return false;
}

}
