#include "naboo_hyperlinks_assist.h"
#include "naboo_view.h"

namespace naboo_reader
{

NabooHyperlinksAssist::NabooHyperlinksAssist( NabooView * view )
: view_(view)
, current_link_idx_(0)
{
    connect(view_, SIGNAL(currentPageChanged(const int, const int)), this, SLOT(onCurrentPageChanged(const int, const int)));
}

NabooHyperlinksAssist::~NabooHyperlinksAssist()
{
}

void NabooHyperlinksAssist::onCurrentPageChanged(const int current, const int total)
{
    current_link_idx_ = 0;
}

Range NabooHyperlinksAssist::getCurrentHyperlink(AdobeLocationPtr & target)
{
    Range result;
    if (view_ == 0 || view_->renderer() == 0)
    {
        return result;
    }

    AdobeRendererClient * renderer = view_->renderer();
    int link_count = renderer->getLinkCount();
    if ( link_count > 0 && link_count > current_link_idx_)
    {
        renderer->getLinkInfo( current_link_idx_, result, target );
    }
    return result;
}

bool NabooHyperlinksAssist::nextHyperlink()
{
    if (view_ == 0 || view_->renderer() == 0)
    {
        return false;
    }

    int link_count = view_->renderer()->getLinkCount();
    if (current_link_idx_ < link_count - 1)
    {
        current_link_idx_++;
        return true;
    }
    return false;
}

bool NabooHyperlinksAssist::prevHyperlink()
{
    if (current_link_idx_ > 0)
    {
        current_link_idx_--;
        return true;
    }
    return false;
}

bool NabooHyperlinksAssist::hitTest( const QPoint & mouse_pos,
                                     Range & range,
                                     AdobeLocationPtr & target )
{
    assert( view_ != 0 );
    AdobeRendererClient * renderer = view_->renderer();
    int link_count = renderer->getLinkCount();
    if ( link_count <= 0 )
    {
        return false;
    }

    bool found = false;
    for ( int idx = 0; idx < link_count && !found; ++idx )
    {
        Range link_range;
        AdobeLocationPtr dst;
        if ( renderer->getLinkInfo( idx, link_range, dst ) )
        {
            scoped_ptr<AdobeRangeInfo> range_info(renderer->getRangeInfo(link_range));
            if (range_info != 0)
            {
                int box_count = range_info->getBoxCount();
                for ( int k = 0; k < box_count && !found; ++k )
                {
                    double x_min = 0.0f, x_max = 0.0f, y_min = 0.0f, y_max = 0.0f;
                    range_info->getBox( k, &x_min, &y_min, &x_max, &y_max, 0 );

                    int adjusted_x_min = static_cast<int>( x_min );
                    int adjusted_y_min = static_cast<int>( y_min );
                    int adjusted_x_max = static_cast<int>( x_max + 1.0 );
                    int adjusted_y_max = static_cast<int>( y_max + 1.0 );
                    QRect area( adjusted_x_min, adjusted_y_min,
                                ( adjusted_x_max - adjusted_x_min + 1 ),
                                ( adjusted_y_max - adjusted_y_min + 1 ) );

                    if ( area.contains( mouse_pos ) )
                    {
                        range  = link_range;
                        target = dst;
                        found  = true;
                    }
                }
            }
        }
    }
    return found;
}

QList<Range> NabooHyperlinksAssist::getHyperlinksInScreen()
{
    assert( view_ != 0 );
    AdobeRendererClient * renderer = view_->renderer();
    assert( renderer != 0 );

    QList<Range> results;
    int link_count = renderer->getLinkCount();
    if ( link_count > 0 )
    {
        for ( int idx = 0; idx < link_count; ++idx )
        {
            Range link_range;
            AdobeLocationPtr target;
            if ( renderer->getLinkInfo( idx, link_range, target ) )
            {
                results.push_back( link_range );
            }
        }
    }
    return results;
}

}
