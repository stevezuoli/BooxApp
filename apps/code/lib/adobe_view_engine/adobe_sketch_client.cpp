#include "dp_all.h"

#include "adobe_sketch_client.h"
#include "adobe_renderer.h"
#include "adobe_render_conf.h"

namespace adobe_view
{

static const unsigned int MERGE_ERROR = 5;

static bool isPointInDisplayRegion( const QList<QRect> & display_region, const QPoint & pos )
{
    QList<QRect>::const_iterator iter = display_region.begin();
    for ( ; iter != display_region.end(); ++iter )
    {
        if ( (*iter).contains( pos ) )
        {
            return true;
        }
    }
    return false;
}

AdobeSketchClient::AdobeSketchClient(AdobeRendererClient *renderer_client)
    : renderer_client_(renderer_client)
{
}

AdobeSketchClient::~AdobeSketchClient()
{
}

bool AdobeSketchClient::canSketch()
{
    dpdoc::Renderer * renderer = renderer_client_->renderer();
    return ( renderer != 0 &&
             ( renderer->getPagingMode() == dpdoc::PM_HARD_PAGES ||
               //renderer->getPagingMode() == dpdoc::PM_HARD_PAGES_2UP ||
               renderer->getPagingMode() == dpdoc::PM_SCROLL_PAGES ) );
}

void AdobeSketchClient::clearCache()
{
    page_offsets_.clear();
}

bool AdobeSketchClient::hitTest( const QPoint & click_point,
                                 QPoint & position_in_page,
                                 QRect & page_display_area,
                                 AdobeLocationPtr & page_loc )
{
    dpdoc::Renderer * renderer = renderer_client_->renderer();
    if (!canSketch())
    {
        return false;
    }

    dp::ref<dpdoc::Location> start = renderer->getScreenBeginning();
    if ( start == 0 )
    {
        return false;
    }
    AdobeLocationPtr start_loc( new AdobeLocation( start ) );

    dp::ref<dpdoc::Location> end = renderer->getScreenEnd();
    if ( end == 0 )
    {
        return false;
    }
    AdobeLocationPtr end_loc( new AdobeLocation( end ) );

    AdobeLocationPtr page = start_loc;
    bool ret = false;
    while ( page <= end_loc )
    {
        QRect region;
        if ( getPageDisplayRegion( page, region ) &&
             region.contains( click_point ) )
        {
            page_display_area = region;
            page_loc = page;
            ret = true;
            break;
        }

        int page_number = static_cast<int>(page->getPagePosition()) + 1;
        if ( page_number >= static_cast<int>(renderer_client_->document()->getPageCount()) )
        {
            break;
        }

        AdobeLocationPtr curr = renderer_client_->document()->getLocationFromPagePosition( page_number );
        if ( curr == 0 )
        {
            break;
        }
        page = curr;
    }

    if ( !ret )
    {
        return false;
    }

    // get offset of page
    QPoint page_offset;
    if ( !getPageOffset( page_loc, page_offset ) )
    {
        return false;
    }

    page_offset *= -1;

    // get offset of mouse position( in page )
    QPoint mouse_position;
    if ( !getMouseOffset( page_offset,
                          click_point,
                          mouse_position ) )
    {
        return false;
    }

    position_in_page = mouse_position;
    return true;
}

bool AdobeSketchClient::getPageOffset( AdobeLocationPtr page_loc,
                                       QPoint & offset )
{
    dpdoc::Renderer * renderer = renderer_client_->renderer();
    if ( renderer->getPagingMode() == dpdoc::PM_HARD_PAGES )
    {
        // if it is in page mode, set the offset to be 0
        offset = QPoint( 0, 0 );
    }
    else
    {
        int page_number = static_cast<int>(page_loc->getPagePosition());
        PageOffsetIter cached_page_pos = page_offsets_.find( page_number );
        if ( cached_page_pos != page_offsets_.end() )
        {
            offset = cached_page_pos.value();
            return true;
        }

        // if it is in page flow mode, get the offset of page
        // store current render context
        dpdoc::Matrix cur_matrix;
        renderer->getNavigationMatrix( &cur_matrix );

        // begin calculation the offset of page
        // get the offset of current location in document
        renderer->navigateToLocation( page_loc->getData() );
        dpdoc::Matrix nav_matrix_in_scroll_mode;
        renderer->getNavigationMatrix( &nav_matrix_in_scroll_mode );

        // get the offset of current location in page
        renderer->setPagingMode( dpdoc::PM_HARD_PAGES );
        dpdoc::Matrix nav_matrix_in_page_mode;
        renderer->getNavigationMatrix( &nav_matrix_in_page_mode );

        // get offset of navigation
        double offset_x = nav_matrix_in_scroll_mode.e - nav_matrix_in_page_mode.e;
        double offset_y = nav_matrix_in_scroll_mode.f - nav_matrix_in_page_mode.f;

        // restore to the previous render context
        renderer->setPagingMode( dpdoc::PM_SCROLL_PAGES );
        renderer->setNavigationMatrix( cur_matrix );

        offset = QPoint( static_cast<int>( offset_x ), static_cast<int>( offset_y ) );

        // cache the calculated result
        page_offsets_[page_number] = offset;
    }
    return true;
}


bool AdobeSketchClient::hitTestPointInRange( const Range & ref_range,
                                             const AdobeLocationPtr & hit_loc,
                                             const QPoint & pos )
{
    if ( !ref_range.isValid() )
    {
        // if there is nothing being selected before, take this point
        return true;
    }

    bool is_contained = false;
    Range r1( ref_range.start, hit_loc );
    is_contained = isPointInDisplayRegion( getDisplayRegionOfRange( r1 ), pos );

    if ( !ref_range.isEmpty() )
    {
        Range r2( ref_range.end, hit_loc );
        is_contained = ( is_contained ||
                         isPointInDisplayRegion( getDisplayRegionOfRange( r2 ), pos ) );
    }

    //qDebug( "Hit Point:(%d, %d)", pos.x(), pos.y() );
    return is_contained;
}

// get the dirty area of a range
QRect AdobeSketchClient::getDirtyAreaOfRange( const Range & range )
{
    if ( range.start == 0 || range.end == 0 )
    {
        return QRect();
    }

    QRect dirty_area;
    QList<QRect> boxes = getDisplayRegionOfRange( range );
    for ( QList<QRect>::iterator iter = boxes.begin();
          iter != boxes.end();
          ++iter )
    {
        dirty_area = dirty_area.united( *iter );
    }
    return dirty_area;
}

// Get the merged display area of a range
// TODO. We don't take the vertical text layout into consideration now,
// need support it later.
QList<QRect> AdobeSketchClient::getMergedDisplayRegionOfRange( const Range & range )
{
    QList<QRect> boxes = getDisplayRegionOfRange( range );
    QList<QRect> merged_boxes;
    if ( !boxes.empty() )
    {
        QList<QRect>::const_iterator idx = boxes.begin();
        QRect merged_rect(*idx);
        for ( ; idx != boxes.end(); ++idx )
        {
            QRect cur_rect( *idx );
            if ( ( cur_rect.top() >= merged_rect.top() &&
                   cur_rect.bottom() <= merged_rect.bottom() ) ||
                 ( cur_rect.top() <= merged_rect.top() &&
                   cur_rect.bottom() >= merged_rect.bottom() ) ||
                 ( abs( cur_rect.top() - merged_rect.top() ) < MERGE_ERROR &&
                   abs( cur_rect.bottom() - merged_rect.bottom() ) < MERGE_ERROR ) )
            {
                // within same line
                merged_rect = merged_rect.united( cur_rect );
            }
            else
            {
                // come to a new line, push back the word rectangle
                merged_boxes.push_back( merged_rect );
                merged_rect = cur_rect;
            }
        }
        // add the last one
        merged_boxes.push_back(merged_rect);
    }
    return merged_boxes;
}

bool AdobeSketchClient::hitTest( const QPoint & click_point,
                                 const Range & ref_range,
                                 AdobeLocationPtr & hit_loc )
{
    hit_loc = AdobeLocationPtr();
    dpdoc::Renderer * renderer = renderer_client_->renderer();
    QPoint real_hit_point = click_point;

    // get current navigation matrix for restoring
    bool need_restore = false;
    dpdoc::Matrix cur_matrix;
    renderer->getNavigationMatrix( &cur_matrix );

    if ( renderer->getPagingMode() == dpdoc::PM_SCROLL_PAGES )
    {
        // Adjust the navigation matrix in scroll page mode
        QPoint             pos_in_page;
        QRect              display_area;
        AdobeLocationPtr   page_loc;
        if ( !hitTest( click_point, pos_in_page, display_area, page_loc ) )
        {
            return false;
        }

        // begin calculation the offset of page
        // get the offset of current location in document
        renderer->navigateToLocation( page_loc->getData() );
        renderer->setPagingMode( dpdoc::PM_HARD_PAGES );
        dpdoc::Matrix nav_matrix_in_page_mode;
        renderer->getNavigationMatrix( &nav_matrix_in_page_mode );

        // get offset of navigation
        double offset_x = nav_matrix_in_page_mode.e;
        double offset_y = nav_matrix_in_page_mode.f;
        QPoint offset( static_cast<int>( offset_x ), static_cast<int>( offset_y ) );

        // plus the offset of environment matrix
        offset += QPoint( static_cast<int>( renderer_client_->renderConf().getEnvMatrix()->e ),
                          static_cast<int>( renderer_client_->renderConf().getEnvMatrix()->f ) );

        // update the click position
        real_hit_point = pos_in_page + offset;
        need_restore = true;
    }

    // get current loction for hit test
    dp::ref<dpdoc::Location> cur_location = renderer->hitTest( real_hit_point.x(),
                                                               real_hit_point.y(),
                                                               dpdoc::HF_SELECT );
    AdobeLocationPtr cur_location_ptr;
    if ( cur_location != 0 )
    {
        cur_location_ptr.reset( new AdobeLocation( cur_location ) );
    }
    else if ( renderer->getPagingMode() != dpdoc::PM_HARD_PAGES )
    {
        // we found the "Force" hit test is only needed in non-hard-page modes
        cur_location = renderer->hitTest( real_hit_point.x(),
                                          real_hit_point.y(),
                                          dpdoc::HF_FORCE | dpdoc::HF_SELECT );
        AdobeLocationPtr test_hit_loc( new AdobeLocation( cur_location ) );
        if ( test_hit_loc != 0 && hitTestPointInRange( ref_range, test_hit_loc, click_point ) )
        {
            cur_location_ptr = test_hit_loc;
        }
    }
    else
    {
        qDebug("Hit Test(%d, %d) Failed", click_point.x(), click_point.y());
    }

    if ( cur_location_ptr != 0 )
    {
        Range screen_range;
        if ( renderer_client_->getScreenRange( screen_range ) &&
             cur_location_ptr >= screen_range.start &&
             cur_location_ptr <= screen_range.end )
        {
            hit_loc = cur_location_ptr;
        }
        else
        {
            qDebug("Invalid Hit Point");
        }
    }

    if ( need_restore )
    {
        // restore to the previous render context
        renderer->setPagingMode( dpdoc::PM_SCROLL_PAGES );
        renderer->setNavigationMatrix( cur_matrix );
    }

    return ( hit_loc != 0 );
}

// get the display region of a range
QList<QRect> AdobeSketchClient::getDisplayRegionOfRange( const Range & range )
{
    dpdoc::Renderer * renderer = renderer_client_->renderer();
    QList<QRect> boxes;
    dpdoc::RangeInfo * range_info = renderer->getRangeInfo( range.start->getData(),
                                                            range.end->getData() );
    if ( range_info == 0 )
    {
        return boxes;
    }

    int count = range_info->getBoxCount();
    if ( count > 0 )
    {
        for ( int i = 0; i < count; ++i )
        {
            dpdoc::Rectangle rect;
            range_info->getBox( i, false, &rect );

            int adjusted_x_min = static_cast<int>( rect.xMin );
            int adjusted_y_min = static_cast<int>( rect.yMin );
            int adjusted_x_max = static_cast<int>( rect.xMax + 1.0 );
            int adjusted_y_max = static_cast<int>( rect.yMax + 1.0 );
            QRect area( adjusted_x_min, adjusted_y_min,
                        ( adjusted_x_max - adjusted_x_min + 1 ),
                        ( adjusted_y_max - adjusted_y_min + 1 ) );

            // validate the visibale area
            QRect renderer_area( QPoint(0, 0), renderer_client_->size() );
            area = area.intersected( renderer_area );

            boxes.push_back( area );
        }
    }
    else if (renderer->getPagingMode() == dpdoc::PM_SCROLL_PAGES)
    {
        // Get offset and refresh the range info
        double page = range.start->getPagePosition();
        AdobeLocationPtr page_loc = renderer_client_->document()->getLocationFromPagePosition( page );

        // if it is in page flow mode, get the offset of page
        // store current render context
        dpdoc::Matrix cur_matrix;
        renderer->getNavigationMatrix( &cur_matrix );

        // begin calculation the offset of page
        // get the offset of current location in document
        renderer->navigateToLocation( page_loc->getData() );
        dpdoc::Matrix nav_matrix_in_scroll_mode;
        renderer->getNavigationMatrix( &nav_matrix_in_scroll_mode );

        // get the offset of current location in page
        renderer->setPagingMode( dpdoc::PM_HARD_PAGES );
        dpdoc::Matrix nav_matrix_in_page_mode;
        renderer->getNavigationMatrix( &nav_matrix_in_page_mode );

        // get offset of navigation
        double offset_x = nav_matrix_in_scroll_mode.e - nav_matrix_in_page_mode.e;
        double offset_y = nav_matrix_in_scroll_mode.f - nav_matrix_in_page_mode.f;
        QPoint offset( static_cast<int>( offset_x ), static_cast<int>( offset_y ) );
        offset -= QPoint( static_cast<int>( cur_matrix.e ), static_cast<int>( cur_matrix.f ) );
        offset *= -1;

        // plus the offset of environment matrix
        offset += QPoint( static_cast<int>( renderer_client_->renderConf().getEnvMatrix()->e ),
                          static_cast<int>( renderer_client_->renderConf().getEnvMatrix()->f ) );

        // get range info
        dpdoc::RangeInfo * currect_range_info = renderer->getRangeInfo( range.start->getData(),
                                                                        range.end->getData() );
        count = currect_range_info->getBoxCount();
        for ( int i = 0; i < count; ++i )
        {
            dpdoc::Rectangle rect;
            range_info->getBox( i, false, &rect );

            int adjusted_x_min = static_cast<int>( rect.xMin );
            int adjusted_y_min = static_cast<int>( rect.yMin );
            int adjusted_x_max = static_cast<int>( rect.xMax + 1.0 );
            int adjusted_y_max = static_cast<int>( rect.yMax + 1.0 );
            QRect area( adjusted_x_min, adjusted_y_min,
                        ( adjusted_x_max - adjusted_x_min + 1 ),
                        ( adjusted_y_max - adjusted_y_min + 1 ) );

            area.moveTop( area.top() + offset.y() );
            area.moveLeft( area.left() + offset.x() );

            // validate the visibale area
            QRect renderer_area( QPoint(0, 0), renderer_client_->size() );
            area = area.intersected( renderer_area );

            boxes.push_back(area);
        }
        currect_range_info->release();

        // restore to the previous render context
        renderer->setPagingMode( dpdoc::PM_SCROLL_PAGES );
        renderer->setNavigationMatrix( cur_matrix );
    }

    range_info->release();
    return  boxes;
}

bool AdobeSketchClient::getMouseOffset( const QPoint & page_offset,
                                        const QPoint & click_point,
                                        QPoint & offset )
{
    dpdoc::Renderer * renderer = renderer_client_->renderer();

    // get the offset of the viewport
    dpdoc::Matrix cur_matrix;
    renderer->getNavigationMatrix( &cur_matrix );

    // get the offset of the mouse point
    QPoint mouse_point = click_point;
    mouse_point -= QPoint( static_cast<int>( renderer_client_->renderConf().getEnvMatrix()->e ),
                           static_cast<int>( renderer_client_->renderConf().getEnvMatrix()->f ) );
    mouse_point -= QPoint( static_cast<int>( cur_matrix.e ),
                           static_cast<int>( cur_matrix.f ) );

    // get the offset of the mouse point in page
    mouse_point -= page_offset;
    offset = mouse_point;
    return true;
}

bool AdobeSketchClient::getPageDisplayRegion( AdobeLocationPtr page_loc,
                                              QRect & region )
{
    dpdoc::Renderer * renderer = renderer_client_->renderer();
    if ( !canSketch() )
    {
        return false;
    }

    // get the page offset in navigation coordination system
    QPoint page_offset;
    if ( !getPageOffset( page_loc, page_offset ) )
    {
        return false;
    }

    // calculate the offset from current viewport in navigation coordination system
    dpdoc::Matrix cur_matrix;
    renderer->getNavigationMatrix( &cur_matrix );
    page_offset -= QPoint( static_cast<int>( cur_matrix.e ), static_cast<int>( cur_matrix.f ) );
    page_offset *= -1;

    // plus the offset of environment matrix
    page_offset += QPoint( static_cast<int>( renderer_client_->renderConf().getEnvMatrix()->e ),
                           static_cast<int>( renderer_client_->renderConf().getEnvMatrix()->f ) );

    // calculate the display size of the page
    QSize display_size;
    if ( !getPageDisplaySize( page_loc, display_size ) )
    {
        return false;
    }

    // return result
    region.setTopLeft( page_offset );
    region.setSize( display_size );
    return true;
}

bool AdobeSketchClient::getPageDisplaySize( AdobeLocationPtr page_loc,
                                            QSize & size )
{
    dpdoc::Renderer *renderer = renderer_client_->renderer();
    if (renderer == 0)
    {
        return false;
    }

    // store current navigation matrix and location
    dpdoc::Matrix cur_matrix;
    renderer->getNavigationMatrix( &cur_matrix );

    dpdoc::Rectangle dimensions;
    double nature_width = 0.0f, nature_height = 0.0f;
    int page_number = static_cast<int>( page_loc->getPagePosition() );
    PageNatureSizeIter cache_idx = page_nature_sizes_.find( page_number );
    if ( cache_idx != page_nature_sizes_.end() )
    {
        // read from the cache
        QSizeF cache_size = cache_idx.value();
        nature_width  = cache_size.width();
        nature_height = cache_size.height();
    }
    else
    {
        // get the nature size of page
        bool is_pixel = false;
        int page_mode = renderer->getPagingMode();
        if ( page_mode != dpdoc::PM_HARD_PAGES )
        {
            dp::ref<dpdoc::Location> cur_loc = renderer->getCurrentLocation();
            AdobeLocationPtr cur_loc_ptr( new AdobeLocation( cur_loc ) );

            // navigate to location
            renderer->navigateToLocation( page_loc->getData() );

            // if it is not paging mode, switch to hard-page mode
            if (page_mode == dpdoc::PM_SCROLL_PAGES)
            {
                renderer->setPagingMode( dpdoc::PM_HARD_PAGES );
            }

            renderer->getNaturalSize( &dimensions );
            nature_width = dimensions.xMax - dimensions.xMin;
            nature_height = dimensions.yMax - dimensions.yMin;

            // restore to previous paging mode
            if (page_mode != renderer->getPagingMode())
            {
                renderer->setPagingMode( page_mode );
            }

            // restore the navigation matrix and position
            renderer->navigateToLocation( cur_loc_ptr->getData() );
            renderer->setNavigationMatrix( cur_matrix );
        }
        else
        {
            renderer->getNaturalSize( &dimensions );
            nature_width = dimensions.xMax - dimensions.xMin;
            nature_height = dimensions.yMax - dimensions.yMin;
        }

        page_nature_sizes_[page_number] = QSizeF( nature_width, nature_height );
    }

    size.setWidth( static_cast<int>( nature_width * cur_matrix.a ) );
    size.setHeight( static_cast<int>( nature_height * cur_matrix.d ) );
    return true;
}

}
