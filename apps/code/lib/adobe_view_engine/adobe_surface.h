#ifndef ADOBE_SURFACE_H_
#define ADOBE_SURFACE_H_

#include "adobe_utils.h"
#include "adobe_render_conf.h"

namespace dpdoc
{
    class Surface;
};

namespace adobe_view
{

class AdobeSurfacePrivate;
class AdobeSurface
{
public:
    AdobeSurface();
    AdobeSurface( const AdobeRenderConf & render_conf,
                  const QSize & size );
    ~AdobeSurface();

    dpdoc::Surface * data() const;
    AdobeSurface & operator = ( const AdobeSurface & );
    const AdobeRenderConf & renderConf() const;
    int length();

    static int comparePriority( const AdobeSurface * s1, const AdobeSurface * s2, RenderPolicy * policy );
    static int estimateLength( const QSize surf_size, QImage::Format format );
    static void intializeDitheringClipMap();
    static void setDitheringDepth(int d);

public:
#ifdef USE_PIXMAP
    QPixmap image();
#else
    QImage image();
#endif
    void clear();
    void clear( const QRect & area );
    void scale( const QSize & size );
    QRect getContentArea();

private:
    scoped_ptr<AdobeSurfacePrivate> surface_private_;
    friend bool operator == ( shared_ptr< AdobeSurface > & a,
                              shared_ptr< AdobeSurface > & b );
};

typedef shared_ptr< AdobeSurface > AdobeSurfacePtr;
bool operator == ( AdobeSurfacePtr & a, AdobeSurfacePtr & b );

};

#endif
