#ifndef ADOBE_SKETCH_CLIENT_H_
#define ADOBE_SKETCH_CLIENT_H_

#include "adobe_utils.h"
#include "adobe_location.h"

namespace adobe_view
{

class AdobeRendererClient;
class AdobeSketchClient
{
public:
    AdobeSketchClient(AdobeRendererClient *renderer_client);
    ~AdobeSketchClient();

    bool canSketch();
    bool canAnnotation() { return true; }

    bool hitTest( const QPoint & click_point,
                  QPoint & position_in_page,
                  QRect & page_display_area,
                  AdobeLocationPtr & page_loc );
    bool hitTest( const QPoint & click_point,
                  const Range & ref_range,
                  AdobeLocationPtr & hit_loc );

    bool getPageDisplayRegion( AdobeLocationPtr page_loc,
                               QRect & region );
    QRect getDirtyAreaOfRange( const Range & range );
    QList<QRect> getMergedDisplayRegionOfRange( const Range & range );

    // clear the cached data
    // NOTE: this function must be called when render setting changes
    void clearCache();

private:
    bool getPageOffset( AdobeLocationPtr page_loc,
                        QPoint & offset );
    bool getMouseOffset( const QPoint & page_offset,
                         const QPoint & click_point,
                         QPoint & offset );
    bool getPageDisplaySize( AdobeLocationPtr page_loc,
                             QSize & size );

    bool hitTestPointInRange( const Range & ref_range,
                              const AdobeLocationPtr & hit_loc,
                              const QPoint & pos );

    QList<QRect> getDisplayRegionOfRange( const Range & range );

private:
    typedef QHash< int, QPoint >    PageOffsets;
    typedef PageOffsets::iterator   PageOffsetIter;

    typedef QHash< int, QSizeF >      PageNatureSizes;
    typedef PageNatureSizes::iterator PageNatureSizeIter;

private:
    AdobeRendererClient *renderer_client_;
    PageOffsets         page_offsets_;       // cache page offsets to avoid redundant calculations
    PageNatureSizes     page_nature_sizes_;  // cache the page nature sizes to avoid redundant calculations
};

};

#endif
