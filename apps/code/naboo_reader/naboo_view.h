#ifndef NABOO_VIEW_H_
#define NABOO_VIEW_H_

#include "naboo_model.h"
#include "naboo_hyperlinks_assist.h"
#include "naboo_utils.h"
#include "naboo_thumbnail_view.h"

using namespace ui;
using namespace vbf;
using namespace sketch;
using namespace tts;
using namespace adobe_view;

namespace naboo_reader
{

//#define MAIN_WINDOW_TOC_ON

class NabooView : public BaseView
{
    Q_OBJECT
public:
    explicit NabooView(QWidget *parent = 0);
    virtual ~NabooView(void);

    // Interfaces of BaseView
    virtual void attachModel(BaseModel *model);
    virtual void deattachModel();

public:
    inline NabooModel* model();
    inline AdobeRendererClient* renderer();

    // Attach necessary views
    void attachMainWindow(MainWindow *main_window);
    void deattachMainWindow(MainWindow *main_window);
    void attachTreeView(TreeViewDialog *tree_view);
    void deattachTreeView(TreeViewDialog *tree_view);
    void attachThumbnailView(ThumbnailView *thumb_view);
    void deattachThumbnailView(ThumbnailView *thumb_view);

    // Popup menu
    void popupMenu();

    // Return
    void returnToLibrary();

    // Flipping
    bool flip( int direction );

#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *we);
#endif

Q_SIGNALS:
    void currentPageChanged(const int, const int);
    void rotateScreen();
    void itemStatusChanged(const StatusBarItemType, const int);
    void onClickedExternalHyperlink(const QString & url);
    void requestUpdateParent(bool update_now);
    void popupJumpPageDialog();
    void fullScreen(bool need_full_screen);
    void clockClicked();

    void testSuspend();
    void testWakeUp();

private Q_SLOTS:
    // Document
    void onDocumentReady();
    void onDocumentError(const QString & error);
    void onDocumentClose();
    void onDocumentRequestLicense(const QString & type,
                                  const QString & resource_id,
                                  const QByteArray & request_data);
    void onSearchSucceeded();
    void onSearchNoMoreResults();
    void onRequestPassword();

    // Renderer
    void onThumbnailReady(QImage thumbnail, const double & zoom_value);
    void onRenderConfigurationUpdated();
    void onRenderRequestSent();
    void onNavigationMatrixUpdated();
    void onExternalHyperlinkClicked(const QString & url);

    // View
    void onSaveViewOptions();
    void onPagebarClicked( const int, const int );
    void onPopupContextMenu();
    void onSearchViewClose();
    void onSearchViewIsHighlightAll();
    void onSearchNext( BaseSearchContext & );
    void onDictClosed();
    void onDictKeyReleased(int);
    void onStylusChanged( const int );
    void onRequestFastUpdateScreen();
    void onRequestFullUpdateScreen();

    // annotation
    void onAnnotationFinished();

    // auto flip
    void autoFlip();
    void autoFlipMultiplePages();

    // tts
    void playVoiceOnCurrentScreen();
    void onTTSPlayFinished();

    // thumbnail
    void onNeedThumbnailForNewPage(const QString bookmark, const QSize &size);
    void onNeedNextThumbnail(const QString current_bookmark, const QSize &size);
    void onNeedPreviousThumbnail(const QString current_bookmark, const QSize &size);
    void onThumbnailReturnToReading(const QString bookmark);

private:
    // navigation
    void prevScreen();
    void nextScreen();
    bool gotoPosition(AdobeLocationPtr pos);
    bool canMoveViewport();
    bool moveViewportByScreenSize(AdobeViewportLocation location);
    bool paintViewportMarks(QPainter & painter);
    bool hitTestViewportMarks(const QPoint & pos, AdobeViewportLocation & location);

    // sketch
    void attachSketchProxy();
    void deattachSketchProxy();
    void updateSketchProxy();

    // Actions
    bool updateActions();
    void generateZoomSettings( std::vector<ZoomFactor> & zoom_settings );
    void textSelectionDone();

    // Retrieve word
    bool penClick( QMouseEvent *me );
    bool retrieveWord( const QPoint & pos, Range & range );
    bool nextWord( const Range & current_word, Range & result );
    bool previousWord( const Range & current_word, Range & result );
    bool upWord( const Range & current_word, Range & result );
    bool downWord( const Range & current_word, Range & result );
    bool firstWord( Range & result );
    void moveToNextWord();
    void moveToPreviousWord();
    void moveToUpWord();
    void moveToDownWord();

    // Copy Content
    bool copyToClipboard();

    // GUI event handlers.
    void mousePressEvent( QMouseEvent *me );
    void mouseReleaseEvent( QMouseEvent *me );
    void mouseMoveEvent( QMouseEvent *me );
    void keyPressEvent( QKeyEvent *ke );
    void keyReleaseEvent( QKeyEvent *ke );
    void paintEvent( QPaintEvent *pe );
    void resizeEvent( QResizeEvent *re );
    void fastRepaint( const QRect & area );
    bool eventFilter(QObject *obj, QEvent *event);

    // Reading tools
    void displayTOC( bool );
    void displayAnnotations( bool );
    void displaySearchView( bool );
    void displayDictionary( bool );
    void enterWordRetrievingMode();

    // Zoom in
    bool zooming( double zoom_setting );
    void zoomInPress( QMouseEvent *me );
    void zoomInMove( QMouseEvent *me );
    void zoomInRelease( QMouseEvent *me );

    // Pan
    void panPress( QMouseEvent *me );
    void panMove( QMouseEvent *me );
    void panRelease( QMouseEvent *me );

    // Sketch
    void setSketchMode( const SketchMode mode, bool selected );
    void setSketchColor( const SketchColor color );
    void setSketchShape( const SketchShape shape );
    void paintSketches( QPainter & painter, AdobeLocationPtr page_loc );
    void paintSketchesInCurrentScreen( QPainter & painter );

    // Annotations
    void setAnnotationMode( const AnnotationMode mode, bool selected );
    bool addSelectionToAnnotations();
    bool removeAnnotationsOnLocation( AdobeLocationPtr loc );

    bool hitTestAnnotation( const QPoint & pos, Annotation & anno );
    void requestUpdateAnnotation( const Annotation & anno );
    void updateAnnotation( const Annotation & anno );

    QRect getDirtyArea( AnnotationCtx & anno_ctx );
    void clearCurrentSelection();
    bool displayCurrentSelection();

    void paintAnnotationsInScreen( QPainter & painter, QBrush & brush );
    void paintSelectings( QPainter & painter, QBrush & brush );
    void paintSelections( QPainter & painter, QBrush & brush, Range & range );
    void penPress( QMouseEvent *me );
    void penMove( QMouseEvent *me );
    void penRelease( QMouseEvent *me );

    // Bookmarks
    void paintBookmark( QPainter & painter );
    void displayBookmarks();
    bool addBookmark();
    bool deleteBookmark();

    // Search
    void paintSearchResults( QPainter & painter, QBrush & brush );
    void resetSearchResults();
    bool isSearchResultsInRange( const Range & search_result,
                                 const Range & range );
    bool searchInCurrentScreen();

    // Rotation
    void rotate();

    // Reading history
    void saveReadingContext();
    bool back();
    bool forward();

    // TTS
    void startTTS();
    void stopTTS();
    bool pauseTTS();
    bool resumeTTS();
    void requestPlayingVoice();
    void ttsGotoPage();

    // Font
    void increaseFontSize();
    void decreaseFontSize();

    // Internal Hyperlinks
    bool tryInternalHyperlink( const QPoint & pos );
    void paintHyperlinks( QPainter & painter );
    void paintSelectedHyperlink( QPainter & painter, QBrush & brush );
    void nextHyperlink();
    void previousHyperlink();
    void navigateToCurrentHyperlink();

    // thumbnail
    void displayThumbnailView();
    bool saveThumbnail();

    // Music Player
    void openMusicPlayer();

    // Slide Show
    void startSlideShow();
    void stopSlideShow();

    // Hash Password
    bool handleHashPasswordRequest(const QString & url);

    // full screen
    bool isFullScreenCalculatedByWidgetSize();

    // Error Handling
    void handleError(const QString & error);

private:
    // primary modules
    NabooModel*               model_;          ///< Reference of NabooModel instance
    AdobeRendererClient       render_client_;  ///< Reference to the renderer

    // controller of rendering and repainting
    QTimer                    repaint_timer_;
    QTimer                    slide_show_timer_;
    QTimer                    annotation_timer_;
    QTimer                    flip_page_timer_;

    // search
    scoped_ptr<SearchWidget>  search_view_;
    BaseSearchContext         search_ctx_;

    // dictionary
    scoped_ptr<DictionaryManager> dict_mgr_;
    scoped_ptr<DictWidget>        dict_widget_;

    // tts
    scoped_ptr<TTS>         tts_engine_;
    scoped_ptr<Sound>       sound_;
    scoped_ptr<TTSWidget>   tts_view_;
    Range                   tts_range_;
    QTimer                  tts_timer_;

    // reading history
    ReadingHistory          reading_history_;

    // annotation
    AnnotationCtx           anno_ctx_;
    scoped_ptr<NotesDialog> notes_dialog_;
    bool                    is_screen_anno_dirty_;

    // sketches
    AdobeSketchClient       sketch_client_;
    SketchProxy             sketch_proxy_;  ///< sketch proxy

    // hyperlinks
    NabooHyperlinksAssist   hyperlink_assist_;
    Range                   selected_hyperlink_;

    // status manager
    StatusManager           status_mgr_;

    // thumbnail content manager
    scoped_ptr<ContentThumbnail> cmt_;

    // mouse events
    StrokeArea              stroke_area_;   ///< stroke area
    PanArea                 pan_area_;      ///< pan area
    scoped_ptr<QRubberBand> rubber_band_;   ///< Rubber band is used in zoom-in mode

    // viewport marks
    scoped_ptr<QImage>      left_image_;
    scoped_ptr<QImage>      right_image_;
    scoped_ptr<QImage>      up_image_;
    scoped_ptr<QImage>      down_image_;

    // popup menu actions
    ZoomSettingActions      zoom_setting_actions_;
    TextSizeActions         font_actions_;
    ViewActions             view_actions_;
    ReadingToolsActions     reading_tools_actions_;
    SketchActions           sketch_actions_;
    SystemActions           system_actions_;

    // font values
    vector<qreal>           font_values_;

    // auto flip
    int                     auto_flip_current_page_;
    int                     auto_flip_step_;

    // current waveform
    onyx::screen::ScreenProxy::Waveform  current_waveform_;

private:
    NO_COPY_AND_ASSIGN(NabooView);
};

inline NabooModel* NabooView::model()
{
    return model_;
}

inline AdobeRendererClient* NabooView::renderer()
{
    return &render_client_;
}

};  // namespace naboo_reader

#endif
