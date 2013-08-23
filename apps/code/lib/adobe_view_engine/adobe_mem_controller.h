#ifndef ADOBE_MEMORY_CONTROLLER_H_
#define ADOBE_MEMORY_CONTROLLER_H_

#include "adobe_utils.h"
#include "adobe_surface.h"
#include "adobe_render_policy.h"

using namespace vbf;

namespace adobe_view
{

class AdobeMemController
{
public:
    AdobeMemController();
    ~AdobeMemController();

    inline RenderPolicy* renderPolicy() { return &policy_; }

    void clear();
    void addSurface( AdobeSurfacePtr surf );
    AdobeSurfacePtr getSurface( const AdobeRenderConf & conf );
    AdobeSurfacePtr getSurface( const AdobeLocationPtr & loc );
    bool removeSurface( const AdobeSurfacePtr surf );

    void setSizeLimit( const int limit );
    bool makeEnoughMemory( const int delta,
                           const AdobeLocationPtr & loc );

private:
    void recalcTotalLength();
    int  bytesConsume();
    bool removeSurface( const AdobeLocationPtr & loc );

private:
    typedef QHash< QString, AdobeSurfacePtr > Cache;
    typedef Cache::iterator CacheIter;

private:
    Cache cache_;

    // the size limit
    int size_limit_;

    // the memory cost of current cached pages
    int total_length_;

    // render policy
    AdobeRenderPolicy policy_;
};

};

#endif
