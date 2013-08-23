#ifndef WEB_BROWSER_VIEW_H_
#define WEB_BROWSER_VIEW_H_

#include <QtGui/QtGui>
#include <QtWebKit/QtWebKit>
#include "onyx/ui/ui.h"
#include "onyx/wireless/wifi_dialog.h"
#include "onyx/wireless/dialup_dialog.h"
#include "onyx/wireless/connecting_dialog.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/dictionary/dictionary_manager.h"
#include "onyx/dictionary/dict_widget.h"
#include "onyx/sound/sound.h"
#include "onyx/tts/tts_widget.h"

#include "page.h"
#include "bookmark_model.h"

using namespace ui;
using namespace tts;

namespace webbrowser
{

typedef QList<QWidget *> QWidgets;

class HoldAutoSaver;
class BrowserView : public QWebView
{
    Q_OBJECT

public:
    BrowserView(QWidget *parent = 0);
    ~BrowserView();

public:
    void attachBookmarkModel(BookmarkModel * model);
    void returnToLibrary();

    void scan();
    void connectTo(const QString &ssid, const QString &psk);
    WifiDialog & wifiDialog();
    DialUpDialog & dialUpDialog();
    ConnectingDialog & connectingDialog();

    const QRect & updateRect() { return update_rect_; }
    void resetUpdateRect() { update_rect_.setRect(0, 0, 0, 0); }
    bool isLoadingFinished() { return (progress_ >= 100 || progress_ < 0); }

    void onRangeClicked(const int, const int);
    void reportCurrentProcess();

    void displayFulfillmentProcessing(const QString & url);
    void displayFulfillmentAborted(const QString & url);
    void displayFulfillmentDone(const QString & path);
    void displayFulfillmentError(const QString & workflow, const QString & error_code);

protected:
    virtual QWebView *createWindow(QWebPage::WebWindowType type);
    virtual void mousePressEvent (QMouseEvent * );
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mouseReleaseEvent (QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent*);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *ke);
    virtual void contextMenuEvent(QContextMenuEvent * event);
    virtual void closeEvent(QCloseEvent * event);
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void focusOutEvent(QFocusEvent * event);

Q_SIGNALS:
    void requestOTA(const QUrl & url);
    void focusOut();

public Q_SLOTS:
    void formLostFocus (void);
    void formFocused (const QString& form_id,
                      const QString& form_name,
                      const QString& form_action,
                      const QString& input_type,
                      const QString& input_id,
                      const QString& input_name);
    void formFocusedAddValue (const QString& form_id,
                              const QString& form_name,
                              const QString& form_action,
                              const QString& input_type,
                              const QString& input_id,
                              const QString& input_name,
                              const QString& value);
    void selectMouseUp();
    void handleSelectPopup();
    void selectFocus();
    void selectBlur();
    void selectChanged();

    void configNetwork();

private Q_SLOTS:
    void updateActions();
    void popupMenu();

    void changeFontFamily(const QString&);
    void changeFontSize(qreal size);
    void changeZoomFactor(ZoomFactor zoom);
    void onLinkClicked(const QUrl &);
    void reloadCurrentUrl();

    void onLoadStarted(void);
    void onLoadProgress(int);
    void onLoadFinished(bool);
    void onRepaintRequested(const QRect&);

    void onConnectionChanged(WifiProfile&, WpaConnection::ConnectionState);
    void onDownloadRequested(const QNetworkRequest & request);
    void onSavePassword(const QByteArray & data);

    void fastUpdateWidget(QRect &rect);
    bool isWidgetVisible(QWidget * wnd);
    void hideHelperWidget(QWidget * wnd);

    void startDictLookup();
    void stopDictLookup();

    void showSearchWidget();
    bool updateSearchCriteria();
    void onSearch(BaseSearchContext&);
    void onSearchClosed();
    bool doSearch();

    void enableTextSelection(bool enable = true);
    bool isTextSelectionEnabled();
    void clearSelection();

    void storeUrl();
    void sync();
    void saveThumbnails();
    QImage thumbnail(const QSize & size);

    void myScroll(int dx, int dy);
    void myScrollTo(const QPoint & p);
    void updateViewportRange();
    QPointF currentOffset();

    void populateJavaScriptWindowObject (void);
    void hideGif();
    void hideInlineFrames();
    void hideObjectElements();
    void hideEmbeddedElements();
    void hideScrollbar();

    void clearHistory();

Q_SIGNALS:
    void connectionChanged(WifiProfile&, WpaConnection::ConnectionState);
    void progressChangedSignal(const int, const int);
    void viewportRangeChangedSignal(const int, const int, const int);
    void showHome();

    void inputFormFocused(const QString& form_id,
                          const QString& form_name,
                          const QString& form_action,
                          const QString& input_type,
                          const QString& input_id,
                          const QString& input_name);
    void inputFormLostFocus(void);

private:
    void addFormsFocusEvent(void);
    void addSelectEvents(void);
    void storeConf(const QUrl & url);
    bool needConfigNetwork();
    void clearFocusWidgets();

    // Music Player
    void openMusicPlayer();

    // Bookmarks
    void displayBookmarks();
    bool addBookmark();
    bool deleteBookmark();

    // Passwords
    bool deletePassword();

    // Proxy
    void configureProxy();

    // Clear Cookies
    void clearCookies();

    void saveThumbnailForExplorer();
private:
    QPoint position_;
    QPoint offset_;

    int scrollbar_hidden_;         ///< Flag to hide scrollbar.
    int progress_;

    onyx::screen::ScreenProxy::Waveform update_type_;
    onyx::screen::ScreenProxy::Waveform system_update_type_;
    QRect update_rect_;

    //FontFamilyActions font_family_actions_;
    ZoomSettingActions zoom_setting_actions_;
    TextSizeActions text_size_actions_;
    BrowserNavigationActions navigation_actions_;
    NetworkActions network_actions_;
    ReadingToolsActions reading_tool_actions_;
    SystemActions system_actions_;

    scoped_ptr<WifiDialog> conf_dialog_;
    scoped_ptr<DialUpDialog> dial_dialog_;
    scoped_ptr<ConnectingDialog> conn_dialog_;
    QVariantList site_list_;

    QRect selected_rect_;
    QString selected_text_;
    bool enable_text_selection_;
    scoped_ptr<Sound> sound_;

    scoped_ptr<DictionaryManager> dicts_;
    scoped_ptr<DictWidget> dict_widget_;

    scoped_ptr<TTS> tts_engine_;
    scoped_ptr<TTSWidget> tts_widget_;

    BaseSearchContext search_context_;
    QWebPage::FindFlags search_flags_;  ///< Flags used by QWebView.
    scoped_ptr<SearchWidget> search_widget_;

    bool need_save_url_;
    QUrl current_url_;

    QString jquery_;
    WebPage *page_;
    BookmarkModel *bookmark_model_;
    QWidgets focus_widgets_;

    bool hand_tool_enabled_;
    WpaConnection::ConnectionState connection_state_;

private:
    friend class HoldAutoSaver;
};

class HoldAutoSaver
{
public:
    HoldAutoSaver(BrowserView *view) : view_(view) { view_->need_save_url_ = false; }
    ~HoldAutoSaver() { view_->need_save_url_ = true; }
private:
    BrowserView * view_;
};

};   // namespace webbrowser

#endif
