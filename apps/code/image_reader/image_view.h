#ifndef IMAGE_VIEW_H_
#define IMAGE_VIEW_H_

#include "image_utils.h"
#include "image_item.h"
#include "image_model.h"
#include "image_render_proxy.h"
#include "image_thumbnail_view.h"

using namespace ui;
using namespace vbf;
using namespace sketch;

namespace image
{

// container of all displaying pages
class DisplayImages
{
public:
    DisplayImages();
    ~DisplayImages();

    void add(shared_ptr<ImageItem> p);
    void clear();
    size_t size();
    shared_ptr<ImageItem> getImage(int num);

private:
    typedef QVector< shared_ptr<ImageItem> > Images;
    typedef Images::iterator ImagesIter;
    Images images_;
};

class ImageView : public BaseView
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = 0);
    virtual ~ImageView(void);

    // Implement the interface.
    virtual void attachModel(BaseModel *model);
    virtual void deattachModel();

    void attachMainWindow(MainWindow *main_window);
    void deattachMainWindow(MainWindow *main_window);
    void attachThumbnailView(ThumbnailView *thumb_view);
    void deattachThumbnailView(ThumbnailView *thumb_view);

    void gotoPage(const int page_number);

Q_SIGNALS:
    // Enter/Exit full screen mode
    void needFullScreen(bool enable);
    void currentPageChanged(const int, const int);
    void itemStatusChanged(const StatusBarItemType, const int);
    void rotateScreen();
    void requestUpdateParent(bool update_now);
    void popupJumpPageDialog();
    void clockClicked();

public Q_SLOTS:
    void onWakeUp();

private Q_SLOTS:
    void onPagebarClicked(const int percent, const int value);
    void onPopupContextMenu();
    void onModelReady(const int init_page_num);
    void onSaveViewOptions();
    void onImageReady(shared_ptr<ImageItem> image, ImageStatus status, bool update_screen);
    void onLayoutDone();
    void onNeedPage(const int page_number);
    void onNeedThumbnail(const int image_idx, const QRect &rect);
    void onThumbnailReady(shared_ptr<BaseThumbnail> thumb, const QRect &bounding_rect);
    void onThumbnailClear();
    void onThumbnailReturn(const int image_idx);
    void onNeedContentArea(const int page_number);
    void onModelClosing();
    void onPrerendering();
#ifdef DISPLAY_SYSTEM_BUSY
    void onRenderBusy();
#endif
    void onStylusChanges(const int type);
    void onRequestUpdateScreen();

    // Handlers of actions
    void returnToLibrary(bool);
    void openThumbnailView(bool);
    void slideShowNextImage();
    void setSketchMode(const SketchMode mode, bool selected);
    void setSketchShape(const SketchShape shape);
    void setSketchColor(const SketchColor color);

private:
    /// GUI event handlers.
    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);
    void mouseMoveEvent(QMouseEvent *me);
    void keyReleaseEvent(QKeyEvent *ke);
    void paintEvent(QPaintEvent *pe);
    void resizeEvent(QResizeEvent *);

#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *we);
#endif

    void updateActions();
    void popupMenu();

    void paintImage(QPainter &painter, shared_ptr<ImageItem> image);
    void paintComment(QPainter &painter, shared_ptr<ImageItem> image);
    void paintSketches(QPainter &painter, shared_ptr<ImageItem> image);

    bool hitTest(const QPoint &point);

    // sketch
    void attachSketchProxy();
    void deattachSketchProxy();
    void updateSketchProxy();

    void initViewSetting();
    void initLayout();
    void resetLayout();

    void openMusicPlayer();

    void scroll(int offset_x, int offset_y);
    void zoomIn(const QRect &zoom_rect);
    void zoomToBestFit();
    void zoomToWidth();
    void zoomToHeight();
    void selectionZoom();
    void setZoomValue(ZoomFactor value);
    void setPan();
    void pan(int offset_x, int offset_y);
    void jump(int page_number);
    void rotate();
    void switchLayout(PageLayoutType mode);
    void startSlideShow();
    void stopSlideShow();
    void onRenderSettingChanged(ImageRenderSetting setting, bool checked);

    /// clear the visible pages
    void clearVisibleImages();

    /// sender render request for one page
    void sendRenderRequest(vbf::PagePtr page);
    void sendPrerenderRequest(const int image_index);

    /// set image to be boot splash
    bool setCurrentImageToBeBootSplash();

    // check the view mode: portrait or landscape
    bool isLandscape() { return (view_setting_.rotate_orient == ROTATE_90_DEGREE ||
                                 view_setting_.rotate_orient == ROTATE_270_DEGREE); }

    // full screen
    bool isFullScreenCalculatedByWidgetSize();

private:
    ImageModel              *model_;                    ///< reference of ImageModel instance
    ImageRenderProxy        render_proxy_;              ///< Image render proxy

    scoped_ptr<PageLayout>  layout_;                    ///< pages layout
    PageLayoutType          read_mode_;                 ///< current reading mode
    vbf::MarginArea         cur_margin_;                ///< content margins

    SketchProxy             sketch_proxy_;              ///< sketch proxy

    DisplayImages           display_images_;            ///< rendered images
    VisiblePages            layout_pages_;              ///< layout pages

    StrokeArea              stroke_area_;               ///< stroke area
    PanArea                 pan_area_;                  ///< pan area
    StatusManager           status_mgr_;                ///< status manager

    ViewSetting             view_setting_;              ///< current view setting
    int                     cur_image_index_;           ///< index of current image
    QVector<int>            rendering_pages_;           ///< list of pages' indexes for rendering
#ifdef DISPLAY_SYSTEM_BUSY
    bool                    need_set_rendering_busy_;   ///< flag of setting system busy
#endif

    // Popup menu actions
    ZoomSettingActions      zoom_setting_actions_;
    ViewActions             view_actions_;
    ReadingToolsActions     reading_tools_actions_;
    SketchActions           sketch_actions_;
    SystemActions           system_actions_;

    QTimer                  slide_timer_;               ///< time controller of the slides show
    scoped_ptr<QRubberBand> rubber_band_;               ///< Rubber band is used in zoom-in mode

    // current waveform
    onyx::screen::ScreenProxy::Waveform  current_waveform_;
private:
    NO_COPY_AND_ASSIGN(ImageView);
};

};
#endif
