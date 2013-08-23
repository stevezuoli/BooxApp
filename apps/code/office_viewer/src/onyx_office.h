#ifndef ONYX_OFFICE_H_
#define ONYX_OFFICE_H_

#include <QtCore/QtCore>
#include <QImage>

#include "onyx/data/search_context.h"
#include "onyx/data/bookmark.h"

namespace onyx {

enum OfficeAction {
    INVALID_ACTION = 0,

    INCREASE_FONT_SIZE,
    DECREASE_FONT_SIZE,
    SET_FONT_SIZE,
    SET_FONT_FAMILY,

    NEXT_PAGE,
    PREV_PAGE,
    GOTO_PAGE,

    FIT_TO_PAGE,
    FIT_TO_WIDTH,
    FIT_TO_HEIGHT,

    ZOOM_IN,
    ZOOM_OUT,
    SET_ZOOM_RATIO,

    TO_REFLOW_MODE,
    TO_PAGE_MODE,

    PAN,
    SMART_PAN_LEFT,
    SMART_PAN_RIGHT,
    PAN_LEFT,
    PAN_RIGHT,

    OPEN_LINK,

    SCROLL_UP,
    SCROLL_DOWN,

    SEARCH_NEXT,
    SEARCH_PREV,
    CANCEL_SEARCH,

    ADD_BOOKMARK,
    DELETE_BOOKMARK,

    RESIZE,
};

enum OfficeFontSizeIndex
{
    Minimum,
    Small,
    Medium,
    Large,
    Maximum
};

enum OfficeSerachState
{
    Found,
    NotFound,
    EndOfDocument,
    Error,
    Progressing,
    SnapToResultDone,
};

struct ReadingContext
{
    bool page_mode_;
    double zoom_value_;
    OfficeFontSizeIndex font_index_;
};


class OfficeReader : public QObject {
    Q_OBJECT
public:
    static OfficeReader & instance() {
        static OfficeReader instance_;
        return instance_;
    }
    ~OfficeReader();

public Q_SLOTS:
    bool initialize(const QSize &);

    bool open(const QString &path, int ms = 10 * 60 * 1000);
    bool isOpened();
    const QString & path();
    bool close();

    QImage & image();
    bool doAction(const OfficeAction action, QVariant parameter = QVariant());
    bool waitForJobFinished(int ms = 5000);

    ReadingContext & reading_context() { return reading_context_; }

    int currentPage();
    int totalPage();
    int xScroll();
    int yScroll();
    bool isInPageMode();
    bool isInReflowMode();

    OfficeFontSizeIndex fontSize(); ///< return font index
    double zoomRatio();

    void setPageRepeat(const int repeat);
    int pageRepeat();

    unsigned int panLimits();
    void panLimits(bool &up, bool &down, bool &left, bool &right);
    bool allLimits();
    bool leftLimit();
    bool rightLimit();
    bool topLimit();
    bool bottomLimit();

    bool initSearch();
    void search(BaseSearchContext &);
    bool cancelSearch();
    BaseSearchContext & searchContext();

    vbf::Bookmarks & bookmarks();
    bool hasBookmark(int);

    QString title();

    QByteArray viewState();
    bool restoreViewState(const QByteArray & data);
    bool restoreViewState();

    bool setZoomRatio(double);
    bool setFontSize(OfficeFontSizeIndex index);
    void setPanPos(const QPoint & pt);

    bool openLink(const QPoint &pt);

private:
    bool resize(const QSize &);
    void resetImage(const QSize &);
    void resizeBackendScreen(const QSize &);

    bool increaseFontSize();
    bool decreaseFontSize();
    void setFontfamily(QString& string);

    bool toPageMode();
    bool toReflowMode();

    bool nextPage();
    bool prevPage();
    bool gotoPage(const int page);

    bool zoomIn();
    bool zoomOut();
    bool fitToPage();
    bool fitToWidth();
    bool fitToHeight();

    bool pan(const QSize & offset);
    bool smartPanLeft();
    bool smartPanRight();
    bool panLeft();
    bool panRight();
    bool scrollUp();
    bool scrollDown();

    bool searchForward();
    bool searchBack();
    //int cancelSearch();

    bool loadBookmarks();
    bool saveBookmarks();
    bool addBookmark(int page);
    bool deleteBookmark(int page);

    int bytesToInt(const char *binary, const int &offset);
    char* intToBytes(char *result, const int &intValue);
    void saveViewState(void *eventData);
    void loadViewState();

Q_SIGNALS:
    void splashScreenDone();
    void passwordRequest(QString & password);
    void documentOpened();
    void pageChanged(int current, int total);
    void imageRenderFinished();
    void jobFinished();
    void panInfo(unsigned int);
    void insufficientMemory();
    void searchStateChanged(int);

    // slots for OnyxAlien.
private Q_SLOTS:
    void onPageChanged(int current, int total);
    void updateImage(void *, unsigned int, unsigned int, unsigned int,
                     unsigned int, unsigned int, unsigned int,
                     unsigned int, unsigned int, unsigned int);
    void informationEvent(int, void *);
    void onInsufficientMemory();
    void reportError(int, void *);
    void userRequest(void *);
    void timerRequest(unsigned long  *reference, unsigned long ms);
    void onRequestTimeout();
    void cancelTimer(unsigned long  *reference);
    void configReady();
    void screenConfiguration(void *);

    void *backendContext();
    void broadcastJobFinish();

private:
    OfficeReader();
    OfficeReader(OfficeReader & ref);

    int timeSlice();
    bool backendProcess(bool & cond, int ms = 500);

    const char * getFontsDir();
    OfficeFontSizeIndex getFontSize();
    const int getFlowMode();

    const bool ableToBroadcast();
    void enableBroadcast(const bool);

    void onViewStateReady(void *);

private:
    bool opened_;           ///< Is document opened.
    QString current_file_;  ///< Current document path.
    QImage* image_;         ///< Backend image.
    int timer_count_;
    bool job_finished_;

    QString font_dir_;  ///< the prefix path of font directory
    QString sub_font_dir_; ///< will be set by change font family action

    QPoint pan_pos_;
    QSize screen_size_;
    int page_repeat_;
    unsigned int pan_limits_;

    int current_page_;
    int total_page_;

    BaseSearchContext search_context_;
    vbf::Bookmarks bookmarks_;

    QByteArray view_state_;

    bool search_init_;
    bool broadcast_;

    bool view_state_process_;
    bool page_mode_process_;    ///< Does picsel change page mode successfully.
    bool screen_resize_finished_;       ///< Backend has been successfully resized.
    bool zooming_process_;      ///< Zooming request processed.

    QString title_;
    ReadingContext reading_context_;
    friend class OnyxAlien;
};

};


#endif
