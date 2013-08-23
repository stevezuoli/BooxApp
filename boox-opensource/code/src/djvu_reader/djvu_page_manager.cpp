#include "djvu_page_manager.h"

using namespace vbf;
namespace djvu_reader
{

static const int SIZE_LIMITATION = 60 * 1024 * 1024;

// Compare the priority between two pages
int comparePriority(DjVuPagePtr p1, DjVuPagePtr p2, vbf::RenderPolicy * render_policy)
{
    // compare the existing of image
    if (p1->image()->isNull() && !p2->image()->isNull())
    {
        return 1;
    }
    else if (!p1->image()->isNull() && p2->image()->isNull())
    {
        return -1;
    }
    else if (p1->image()->isNull() && p2->image()->isNull())
    {
        return 0;
    }

    int src_pri = render_policy->getPriority(p1->getPageNumber());
    int dst_pri = render_policy->getPriority(p2->getPageNumber());

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

DjvuPageManager::DjvuPageManager(void)
    : cache_()
    , size_limit_( SIZE_LIMITATION )
    , total_length_( 0 )
{
}

DjvuPageManager::~DjvuPageManager(void)
{
    clear();
}

/// Clear the cache
void DjvuPageManager::clear()
{
    cache_.clear();
    total_length_ = 0;
}

/// Get a DjVu Page by page number
DjVuPagePtr DjvuPageManager::getPage(int page_num)
{
    CacheIter iter = cache_.find( page_num );
    if (iter != cache_.end())
    {
        return iter.value();
    }
    DjVuPagePtr page(new DjVuPage(page_num));
    cache_[page_num] = page;
    return page;
}

/// Remove a surface for given location
bool DjvuPageManager::clearImage( int page_num )
{
    // remove the surface with lowest priority based on the remove strategy
    CacheIter begin       = cache_.begin();
    CacheIter end         = cache_.end();
    CacheIter iter        = begin;
    CacheIter remove_iter = iter;
    int ret = 0;
    while( iter != end )
    {
        ret = comparePriority( iter.value(), remove_iter.value(), &policy_ );
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

    DjVuPagePtr page = getPage( page_num );
    if ( page != 0 && page == remove_iter.value() )
    {
        // if it is the same surface, return
        return false;
    }

    total_length_ -= remove_iter.value()->imageLength();

    // remove the bitmap and other records, reset the status at the time
    qDebug( "Remove Page:%p", remove_iter.value().get() );
    remove_iter.value()->clearImage();
    return true;
}

/// Recalculate length of all pages.
/// The total length won't be updated unless calling this function
void DjvuPageManager::recalcTotalLength()
{
    total_length_ = 0;
    CacheIter begin = cache_.begin();
    CacheIter end   = cache_.end();
    CacheIter iter  = begin;
    for(; iter != end; ++iter)
    {
        total_length_ += iter.value()->imageLength();
    }
}

/// Return the total memory consumed by the cache manager
int DjvuPageManager::bytesConsume()
{
    return total_length_;
}

/// Set the size limit
void DjvuPageManager::setSizeLimit( const int limit )
{
    size_limit_ = limit;
}

/// Make enough memory
bool DjvuPageManager::makeEnoughMemory( const int delta, int page_num )
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
            if ( !clearImage( page_num ) )
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
            if ( !clearImage( page_num ) )
            {
                return false;
            }
        }
    }
    return true;
}

}
