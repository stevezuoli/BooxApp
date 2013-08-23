#ifndef DJVU_PAGE_MANAGER_H_
#define DJVU_PAGE_MANAGER_H_

#include "djvu_utils.h"
#include "djvu_render_policy.h"
#include "djvu_page.h"

using namespace vbf;

namespace djvu_reader
{

class DjvuPageManager
{
public:
    DjvuPageManager();
    ~DjvuPageManager();

    inline RenderPolicy* renderPolicy() { return &policy_; }

    void clear();
    DjVuPagePtr getPage( int page_num );

    void setSizeLimit( const int limit );
    bool makeEnoughMemory( const int delta, int page_num );

private:
    bool clearImage( int page_num );
    void recalcTotalLength();
    int  bytesConsume();

private:
    typedef QHash< int, DjVuPagePtr > Cache;
    typedef Cache::iterator CacheIter;

private:
    Cache cache_;

    // the size limit
    int size_limit_;

    // the memory cost of current cached pages
    int total_length_;

    // render policy
    DjVuRenderPolicy policy_;
};

};

#endif
