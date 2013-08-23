#ifndef NOTES_VIEW_H_
#define NOTES_VIEW_H_

#include "image_utils.h"
#include "image_item.h"
#include "image_thumbnail_view.h"
#include "image_render_proxy.h"
#include "image_model.h"
#include "notes_doc_manager.h"

using namespace ui;
using namespace vbf;
using namespace sketch;

namespace image
{

class NotesPage
{
public:
    NotesPage() : index_(-1) {}
    NotesPage(shared_ptr<ImageItem> image, int index) : image_(image), index_(index) {}
    NotesPage(const NotesPage & right) { *this = right; }
    ~NotesPage() {}

    inline NotesPage & operator = ( const NotesPage & right ) { image_ = right.image_; index_ = right.index_; return *this; }
    inline shared_ptr<ImageItem> image() { return image_; }
    inline int index() { return index_; }

private:
    shared_ptr<ImageItem> image_;   // the image needed for displaying
    int                   index_;   // index of the page

    friend bool operator == ( const shared_ptr<NotesPage> & a, const shared_ptr<NotesPage> & b );
};

bool operator == ( const shared_ptr<NotesPage> & a, const shared_ptr<NotesPage> & b );

// container of all displaying pages
class DisplayPages
{
public:
    DisplayPages();
    ~DisplayPages();

    void add(shared_ptr<NotesPage> page);
    void clear();
    size_t size();
    shared_ptr<NotesPage> getPage(int num);

private:
    typedef QVector< shared_ptr<NotesPage> > Pages;
    typedef Pages::iterator PagesIter;
    Pages pages_;
};

class NotesView : public BaseView
{
    Q_OBJECT
public:
    explicit NotesView(QWidget *parent = 0);
    virtual ~NotesView(void);

    // Implement the interface.
    virtual void attachModel(BaseModel *model);
    virtual void deattachModel();

    void attachMainWindow(MainWindow *main_window);
    void deattachMainWindow(MainWindow *main_window);
    void setSketchProxy(shared_ptr<SketchProxy> proxy);
    void setNotesManager(shared_ptr<NotesDocumentManager> doc_mgr);

    inline shared_ptr<SketchProxy> sketchProxy() { return sketch_proxy_; }
    inline shared_ptr<NotesDocumentManager> notesMgr() { return notes_doc_manager_; }

    void setName();
    void gotoPage(const int page_number);
    void nextPage();
    void prevPage();

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
    void saveViewOptions();
    void loadViewOptions();

    void onImageReady(shared_ptr<ImageItem> image, const int index,
                      ImageStatus status, bool update_screen);
    void onLayoutDone();
    void onNeedPage(const int page_number);

    void onNeedNotesThumbnail(const int image_idx, const QRect &rect);
    void onNeedBackgroundThumbnail(const int image_idx, const QRect &rect);

    void onClearNotesThumbnails();
    void onClearBackgroundThumbnails();

    void onNotesThumbnailReturn(const int page_number);
    void onBackgroundThumbnailReturn(const int image_index);
    void onDefaultBackgroundThumbnailReturn(const int image_index);

    void onNotesThumbnailReady(shared_ptr<BaseThumbnail> thumb,
                               const int index, const QRect &bounding_rect);
    void onBackgroundThumbnailReady(shared_ptr<BaseThumbnail> thumb,
                                    const QRect &bounding_rect);

    void onModelClosing();
#ifdef DISPLAY_SYSTEM_BUSY
    void onRenderBusy();
#endif
    void onStylusChanges(const int type);
    void onRequestUpdateScreen();

    // Handlers of actions
    void returnToLibrary(bool);
    bool openDefaultBackgroundThumbnailView();
    bool openBackgroundThumbnailView();
    void openNotesThumbnailView();
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

    void initView();
    void updateActions();
    void popupMenu();

    int getLastPage();
    void insertPage(int dst_page);
    void removePage(int dst_page);
    void setPageBackgroundImage(const int page_number, const QString & background_image);

    void paintPage(QPainter &painter, NotesPage & page);
    void paintSketches(QPainter &painter, NotesPage & page);

    bool hitTest(const QPoint &point);

    // sketch
    void attachSketchProxy();
    void deattachSketchProxy();
    void updateSketchProxy();

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

    void clearVisiblePages();
    void render(vbf::PagePtr page);
    void renderNotesThumbnail(const int image_idx, const QRect &rect);
    QString getBackgroundImage(int dst_page);
    QString getDefaultBackgroundImage(int dst_page);

    // check the view mode: portrait or landscape
    bool isLandscape() { return (view_setting_.rotate_orient == ROTATE_90_DEGREE ||
                                 view_setting_.rotate_orient == ROTATE_270_DEGREE); }

private:
    ImageModel              *model_;                    ///< reference of ImageModel instance
    shared_ptr<SketchProxy> sketch_proxy_;              ///< sketch proxy
    shared_ptr<NotesDocumentManager> notes_doc_manager_;///< notes document manager
    ImageRenderProxy        render_proxy_;              ///< Image render proxy

    scoped_ptr<PageLayout>  layout_;                    ///< pages layout
    PageLayoutType          read_mode_;                 ///< current reading mode
    vbf::MarginArea         cur_margin_;                ///< content margins

    DisplayPages            display_pages_;             ///< rendered images
    VisiblePages            layout_pages_;              ///< layout pages

    StrokeArea              stroke_area_;               ///< stroke area
    PanArea                 pan_area_;                  ///< pan area
    StatusManager           status_mgr_;                ///< status manager

    ViewSetting             view_setting_;              ///< current view setting
    int                     current_page_;              ///< index of current page
    bool                    need_init_view_;            ///< is view initialization needed

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
    NO_COPY_AND_ASSIGN(NotesView);
};

};

#endif
