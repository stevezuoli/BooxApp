
#ifndef READER_VIEW_H_
#define READER_VIEW_H_

#include "onyx/base/base.h"
#include <QtWebKit/QtWebKit>
#include "onyx/ui/ui.h"
#include "onyx/data/configuration.h"
#include "onyx/screen/screen_proxy.h"
#include "network_reply.h"
#include "model_interface.h"
#include "onyx/dictionary/dictionary_manager.h"
#include "onyx/dictionary/dict_widget.h"
#include "onyx/sound/sound.h"
#include "onyx/tts/tts_widget.h"
#include "doc_map_actions.h"

using namespace ui;
using namespace tts;
using namespace vbf;

namespace reader
{

class ReaderView : public QWebView
{
    Q_OBJECT

public:
    ReaderView(QWidget *parent = 0);
    ~ReaderView();

public:
    const QRect & updateRect() { return update_rect_; }
    void resetUpdateRect() { update_rect_.setRect(0, 0, 0, 0); }
    const QString & doc_path() { return path_; }
    bool isLoadingFinished() { return (progress_ >= 100 || progress_ < 0); }

public Q_SLOTS:
    bool open(const QString &path);
    bool close();

    void returnToLibrary();
    void saveOptions();

    void onRangeClicked(const int percentage, const int value);

protected:
    virtual QWebView *createWindow(QWebPage::WebWindowType type);
    virtual void mousePressEvent (QMouseEvent * );
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mouseReleaseEvent (QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent*);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *ke);
    virtual void contextMenuEvent(QContextMenuEvent * event);
    virtual void resizeEvent(QResizeEvent *);
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent(QWheelEvent *we);
#endif


Q_SIGNALS:
    void progressChangedSignal(const int, const int);
    void viewportRangeChangedSignal(const int, const int, const int);
    void rotateScreen();
    void clockClicked();

private Q_SLOTS:
    void updateActions();
    void popupMenu();

    void onLinkClicked(const QUrl &);
    void onSelectionChanged();

    void onLoadStarted(void);
    void onLoadProgress(int);
    void onLoadFinished(bool);
    void onRepaintRequested(const QRect&);

    void startDictLookup();
    void stopDictLookup();

    void showSearchWidget();
    bool updateSearchCriteria();
    void onSearch(BaseSearchContext&);
    void onSearchClosed();
    bool doSearch();

    void clearSelection();
    void onMusicPlayerStateChanged(int);

private:
    vbf::Configuration & conf() { return conf_;}
    QUrl & current_location() { return current_location_; }
    void loadOptions(const QString &);
    void saveOptions(const QString &);

    void gotoHome();
    void showTableOfContents();

    void fastUpdateWidget(QRect &rect);
    bool isWidgetVisible(QWidget * wnd);
    void hideHelperWidget(QWidget * wnd);

    void changeFontFamily(const QString&);

    void enableTextSelection(bool enable = true);
    bool isTextSelectionEnabled();

    QPointF viewport();
    void setViewport(QPointF percentage);

    void myScroll(int dx, int dy);
    void myScrollTo(const QPoint &pt);
    QPointF currentOffset();
    void updateViewportRange();

    bool atBeginning();
    bool atEnd();

    void scrollNext();
    void scrollPrev();

    bool nextUrl(QUrl & url);
    bool prevUrl(QUrl & url);

    void hideGif();
    void hideScrollbar();

private:
    QPoint position_;
    QPoint offset_;
    QRect  update_rect_;
    int    progress_;

    int scrollbar_hidden_;         ///< Flag to hide scrollbar.
    onyx::screen::ScreenProxy::Waveform update_type_;
    onyx::screen::ScreenProxy::Waveform system_update_type_;
    QString selected_text_;
    bool enable_text_selection_;
    bool change_view_port_;     ///< Need to change view port when page is loaded.

    FontFamilyActions font_family_actions_;
    TextSizeActions text_size_actions_;
    BrowserNavigationActions navigation_actions_;
    DocMapActions doc_map_actions_;
    ReadingToolsActions reading_tool_actions_;
    SystemActions system_actions_;

    scoped_ptr<ModelInterface> model_;
    scoped_ptr<ReaderNetworkManager> manager_;
    cms::ContentManager database_;
    vbf::Configuration conf_;
    QUrl current_location_; ///< Location in document.
    QString path_;      ///< Document path.

    QRect selected_rect_;
    scoped_ptr<Sound> sound_;

    scoped_ptr<DictionaryManager> dicts_;
    scoped_ptr<DictWidget> dict_widget_;

    scoped_ptr<TTS> tts_engine_;
    scoped_ptr<TTSWidget> tts_widget_;

    BaseSearchContext search_context_;
    QWebPage::FindFlags search_flags_;  ///< Flags used by QWebView.
    scoped_ptr<SearchWidget> search_widget_;

    QString jquery_;    ///< jQuery libarary
};

}   // namespace reader

#endif  // READER_VIEW_H_
