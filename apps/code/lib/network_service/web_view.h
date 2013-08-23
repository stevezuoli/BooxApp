#ifndef WEB_BROWSER_VIEW_H_
#define WEB_BROWSER_VIEW_H_

#include "ns_utils.h"
#include "web_page.h"
#include "access_manager.h"
#include "dm_manager.h"

using namespace ui;
using namespace tts;

namespace network_service
{

typedef QList<QWidget *> QWidgets;

class WebView : public QWebView
{
    Q_OBJECT

public:
    WebView(QWidget *parent = 0,
            NetworkAccessManager * access_manager = 0,
            DownloadManager * download_manager = 0);
    ~WebView();

public:
    void scan();
    void connectTo(const QString &ssid, const QString &psk);
    WifiDialog & networkDialog();

    const QRect & updateRect() { return update_rect_; }
    void resetUpdateRect() { update_rect_.setRect(0, 0, 0, 0); }
    bool isLoadingFinished() { return (progress_ >= 100 || progress_ < 0); }

    void onRangeClicked(const int, const int);
    void reportCurrentProcess();

protected:
    virtual QWebView *createWindow(QWebPage::WebWindowType type);
    virtual void contextMenuEvent(QContextMenuEvent * event);
    virtual void mousePressEvent (QMouseEvent * );
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mouseReleaseEvent (QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent*);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *ke);
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

Q_SIGNALS:
    void connectionChanged(WifiProfile&, WpaConnection::ConnectionState);
    void progressChangedSignal(const int, const int);
    void viewportRangeChangedSignal(const int, const int, const int);

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

    // Passwords
    bool deletePassword();

    // Proxy
    void configureProxy();

    // Clear Cookies
    void clearCookies();

private:
    NetworkAccessManager *access_manager_;
    DownloadManager *download_manager_;

    QPoint position_;
    QPoint offset_;

    int scrollbar_hidden_;         ///< Flag to hide scrollbar.
    int progress_;

    onyx::screen::ScreenProxy::Waveform update_type_;
    onyx::screen::ScreenProxy::Waveform system_update_type_;
    QRect update_rect_;

    scoped_ptr<WifiDialog> conf_dialog_;

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
    QWidgets focus_widgets_;

    bool hand_tool_enabled_;
    WpaConnection::ConnectionState connection_state_;
};

};   // namespace network_service

#endif
