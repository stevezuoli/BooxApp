#include "adobe_mem_controller.h"

#include "onyx/sys/sys_utils.h"

using namespace vbf;
namespace adobe_view
{

static const int SIZE_LIMITATION = 10 * 1024 * 1024;

AdobeMemController::AdobeMemController(void)
    : cache_()
    , size_limit_( SIZE_LIMITATION )
    , total_length_( 0 )
{
}

AdobeMemController::~AdobeMemController(void)
{
    clear();
}

/// Clear the cache
void AdobeMemController::clear()
{
    cache_.clear();
    total_length_ = 0;
}

/// Add a new surface
void AdobeMemController::addSurface(AdobeSurfacePtr surf)
{
    QString page_position = surf->renderConf().getPagePosition()->getBookmark();
    cache_.insertMulti( page_position, surf );
}

/// Get a surface by location
AdobeSurfacePtr AdobeMemController::getSurface(const AdobeLocationPtr & loc)
{
    QString page_position = loc->getBookmark();
    CacheIter iter = cache_.find( page_position );
    if (iter != cache_.end())
    {
        return iter.value();
    }
    return AdobeSurfacePtr();
}

/// Get a surface by render request
AdobeSurfacePtr AdobeMemController::getSurface(const AdobeRenderConf & conf)
{
    QString page_position = conf.getPagePosition()->getBookmark();
    QList< AdobeSurfacePtr > surfaces = cache_.values( page_position );
    QList< AdobeSurfacePtr >::iterator iter = surfaces.begin();
    for ( ; iter != surfaces.end(); ++iter )
    {
        if ( (*iter)->renderConf() == conf )
        {
            return *iter;
        }
    }
    return AdobeSurfacePtr();
}

/// Remove a surface
bool AdobeMemController::removeSurface( const AdobeSurfacePtr surf )
{
    QString page_position = surf->renderConf().getPagePosition()->getBookmark();
    CacheIter iter = cache_.find( page_position );
    if ( iter == cache_.end() )
    {
        return false;
    }
    cache_.erase( iter );
    return true;
}

/// Remove a surface for given location
bool AdobeMemController::removeSurface( const AdobeLocationPtr & loc )
{
    // remove the surface with lowest priority based on the remove strategy
    CacheIter begin       = cache_.begin();
    CacheIter end         = cache_.end();
    CacheIter iter        = begin;
    CacheIter remove_iter = iter;
    int ret = 0;
    while( iter != end )
    {
        ret = AdobeSurface::comparePriority( iter.value().get(),
                                             remove_iter.value().get(),
                                             &policy_ );
        if (ret < 0)
        {
            remove_iter = iter;
        }
        iter++;
    }

    if ( remove_iter == cache_.end() )
    {
        return false;
    }

    AdobeSurfacePtr surface = getSurface( loc );
    if ( surface != 0 && surface == remove_iter.value() )
    {
        // if it is the same surface, return
        return false;
    }

    total_length_ -= remove_iter.value()->length();

    // remove the bitmap and other records, reset the status at the time
    qDebug( "Remove Page:%p", remove_iter.value().get() );
    cache_.erase( remove_iter );
    return true;
}

/// Recalculate length of all pages.
/// The total length won't be updated unless calling this function
void AdobeMemController::recalcTotalLength()
{
    total_length_ = 0;
    CacheIter begin = cache_.begin();
    CacheIter end   = cache_.end();
    CacheIter iter  = begin;
    for(; iter != end; ++iter)
    {
        total_length_ += iter.value()->length();
    }
}

/// Return the total memory consumed by the cache manager
int AdobeMemController::bytesConsume()
{
    return total_length_;
}

/// Set the size limit
void AdobeMemController::setSizeLimit( const int limit )
{
    size_limit_ = limit;
}

/// Make enough memory
bool AdobeMemController::makeEnoughMemory( const int delta,
                                           const AdobeLocationPtr & loc )
{
    if ( size_limit_ > 0 )
    {
        if (delta >= size_limit_)
        {
            return false;
        }

        recalcTotalLength();
        if ( delta + bytesConsume() <= size_limit_ )
        {
            return true;
        }

        int updated_limit = ( size_limit_ >> 2 );
        while ( delta + bytesConsume() > updated_limit )
        {
            if ( !removeSurface( loc ) )
            {
                //qDebug("Skipped Page");
                return false;
            }
        }
    }
    else
    {
      while (sys::needReleaseMemory())
        {
            if ( !removeSurface( loc ) )
            {
                return false;
            }
        }
    }
    return true;
}

}
