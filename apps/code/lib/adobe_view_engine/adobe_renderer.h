#ifndef ADOBE_RENDERER_H_
#define ADOBE_RENDERER_H_

#include "adobe_utils.h"
#include "adobe_document.h"
#include "adobe_mem_controller.h"
#include "adobe_events.h"

namespace dpdoc
{
    class Renderer;
};

namespace adobe_view
{

class AdobeRendererClientPrivate;
class AdobeRendererClient : public QObject
{
    Q_OBJECT
public:
    AdobeRendererClient(QWidget *parent);
    ~AdobeRendererClient();

    inline AdobeDocumentClient* document();
    inline AdobeSurface* surface();
    inline AdobeMemController* memoryController();
    inline void attachWidget(QWidget *parent);
    inline QSize viewport();
    inline bool isLocked();
    inline void lock();
    inline void unlock();
    inline bool needRestore();
    inline void setRestoreFinished( bool finished );
    inline AdobeRenderConf & renderConf();
    bool isInFontIndexMode();
    bool isValid();
    bool isAtEnd();
    bool isAtBeginning();

    // Initialize
    void attachDocumentClient(AdobeDocumentClient* client);
    void deattachDocumentClient();

    // Rendering
    void requestRepaint( AdobeSurfacePtr surface );
    void repaintCurrentSurface( const QRect & dirty_area, AdobeRenderConf * render_task );
    void repaintThumbnail( const QRect & dirty_area, const double & zoom_value );
    void updateRenderConf( const AdobeRenderConf & conf );

    // View options
    void setViewport( const QSize & size );
    AdobeViewportLocations viewportLocation();
    QRect displayArea();
    int getPagingMode();
    void saveOptions();
    double getRealZoomFactor();

    // Screen
    bool getScreenRange(Range & range);

    // Operations
    bool gotoPosition( AdobeLocationPtr pos );
    bool navigateToLink( AdobeLocationPtr target );
    void nextScreen();
    void prevScreen();
    bool scaleSurface( double zoom_setting );
    bool scaleFontSize( int font_size );
    bool scaleFontRatio( double font_ratio );
    void rotate();
    void selectZoom( const QRect & area );
    void moveViewport( int h_offset, int v_offset );
    bool switchPagingMode( int paging_mode );

    // Asyncronise requests
    void sendRenderRequest( AdobeRenderConf * render_conf );

    // Links
    int getLinkCount();
    bool getLinkInfo(int link_index, Range & range, AdobeLocationPtr & target);
    AdobeRangeInfo* getRangeInfo(const Range & from);

    // Locations
    AdobeLocationPtr getCurrentLocation();
    AdobeLocationPtr getScreenBeginning();
    AdobeLocationPtr getScreenEnd();

    // Events
    void handleMouseEvent(AdobeMouseEvent *e);
    void handleKeyboardEvent(AdobeKeyboardEvent *e);
    void handleTextEvent(AdobeTextEvent *e);

    // Hit Test
    bool hitTest(const QPoint & pos, unsigned int flags, AdobeLocationPtr & loc_res);

    // Set Zooming Mode
    inline double zoomingMode() const { return zooming_mode_; }

    void enableRepaint();
    void disableRepaint();

Q_SIGNALS:
    void externalHyperlinkClicked(const QString & url);
    void thumbnailReady(QImage thumbnail, const double & zoom_value);
    void renderConfigurationUpdated();
    void renderRequestSent();
    void navigationMatrixUpdated();

public Q_SLOTS:
    void onDocumentReady();
    void onDocumentClose();

private:
    inline QRect rect();
    inline QSize size();

    dpdoc::Renderer* renderer();
    void notifyExternalHyperlinkClicked(const QString & url);

private:
    QWidget                                 *parent_;
    scoped_ptr<AdobeRendererClientPrivate>  renderer_private_;
    AdobeDocumentClient                     *document_client_;

    AdobeRenderConf                         render_conf_;
    AdobeSurface                            surface_;
    AdobeMemController                      mem_controller_;
    QSize                                   viewport_;
    double                                  zooming_mode_;

    bool                                    need_restore_;
    bool                                    is_locked_;

    friend class AdobeRenderConf;
    friend class AdobeSketchClient;
    friend class AdobeRendererClientPrivate;
};

inline AdobeDocumentClient* AdobeRendererClient::document()
{
    return document_client_;
}

inline AdobeSurface* AdobeRendererClient::surface()
{
    return &surface_;
}

inline AdobeMemController* AdobeRendererClient::memoryController()
{
    return &mem_controller_;
}

inline void AdobeRendererClient::attachWidget(QWidget *parent)
{
    parent_ = parent;
}

inline QSize AdobeRendererClient::viewport()
{
    return viewport_;
}

inline bool AdobeRendererClient::isLocked()
{
    return is_locked_;
}

inline void AdobeRendererClient::lock()
{
    is_locked_ = true;
}

inline void AdobeRendererClient::unlock()
{
    is_locked_ = false;
}

inline bool AdobeRendererClient::needRestore()
{
    return need_restore_;
}

inline void AdobeRendererClient::setRestoreFinished( bool finished )
{
    need_restore_ = !finished;
}

inline QRect AdobeRendererClient::rect()
{
    return parent_->rect();
}

inline QSize AdobeRendererClient::size()
{
    return parent_->size();
}

inline AdobeRenderConf & AdobeRendererClient::renderConf()
{
    return render_conf_;
}

};

#endif
