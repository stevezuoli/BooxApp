
#ifndef WEB_BROWSER_FRAME_H_
#define WEB_BROWSER_FRAME_H_

#include <QtGui/QtGui>
#include "view.h"
#include "keyboard_dialog.h"
#include "network_service/download_view.h"
#include "onyx/ui/ui.h"
#include "onyx/ui/thumbnail_view.h"
#include "network_service/dm_manager.h"

#include "bookmark_model.h"

using namespace ui;
using namespace network_service;
namespace webbrowser
{

struct BrowserKeyboardPrivate
{
    QString form_id;
    QString form_name;
    QString form_action;
    QString input_type;
    QString input_id;
    QString input_name;
};

enum BrowserKeyboardStatus
{
    KEYBOARD_FREE = -1,
    FORM_FOCUSED = 0,
    URL_INPUTTING
};

class BrowserFrame : public QWidget
{
    Q_OBJECT

public:
    BrowserFrame(QWidget *parent = 0);
    ~BrowserFrame();

    void attachBookmarkModel(BookmarkModel * model);

public Q_SLOTS:
    void load(const QString & url_str);
    void configNetwork();
    void scan();
    void connectTo(const QString &ssid, const QString &psk);
    void onScreenSizeChanged(int);
    void onLoadFinished(const QString &file_name);

protected:
    void keyPressEvent(QKeyEvent * ke);
    void keyReleaseEvent(QKeyEvent *ke);
    bool event(QEvent *e);
    void closeEvent(QCloseEvent *e);

private Q_SLOTS:
    void showThumbnailView(bool show = true);
    void onThumbnailClicked(QStandardItem *selected_item);
    void onThumbnailEscape();
    void showHomePage();
    void onConnectionChanged(WifiProfile&, WpaConnection::ConnectionState);
    void onProgressChanged(const int, const int);
    void onRangeChanged(const int, const int, const int);
    void onProgressClicked(const int, const int);
    void onThumbnailPositionChanged(const int, const int);

    void onInputFormFocused(const QString & form_id,
                            const QString & form_name,
                            const QString & form_action,
                            const QString & input_type,
                            const QString & input_id,
                            const QString & input_name);
    void onTextFinished(const QString & text);
    void onMusicPlayerStateChanged(int);

    void onDownloadItemAdded(DownloadItem *item);
    void onRequestOTA(const QUrl & url);
    void onRequestOTA(const QString & acsm, const QUrl & url);
    void onRequestDRMUserInfo(const QString & url_str, const QString & param);
    void onWebViewFocusOut();
    void onFulfillmentFinished(const QString & file_path);
    void onReportWorkflowError(const QString & workflow, const QString & error_code);

    void onACSMDownloaded();
    void onACSMItemDeleted();

    void onInputUrl();
    void onInputText();

private:
    void createLayout();
    void setupToolbar();
    void loadThumbnails();
    void thumbnailModel(QStandardItemModel & model);

private:
    QVBoxLayout            layout_;
    OnyxToolBar            tool_bar_;
    BrowserView            view_;
    KeyboardDialog         keyboard_;
    ui::KeyBoard           input_widget_;
    ThumbnailView          thumbnail_view_;
    QStandardItemModel     model_;
    SysStatus &            sys_;
    StatusBar              status_bar_;
    BrowserKeyboardPrivate keyboard_priv_;

    DownloadView           download_view_;
    bool                   need_gc_in_loading_;
    BrowserKeyboardStatus  keyboard_status_;
};

}   // namespace webbrowser

#endif  // WEB_BROWSER_FRAME_H_
