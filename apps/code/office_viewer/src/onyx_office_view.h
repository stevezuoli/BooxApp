#ifndef ONYX_OFFICE_VIEW_H_
#define ONYX_OFFICE_VIEW_H_

#include <QWidget>

#include "onyx_office.h"
#include "onyx/data/configuration.h"
#include "onyx/data/bookmark.h"
#include "onyx/ui/zoom_setting_actions.h"
#include "onyx/ui/font_actions.h"
#include "onyx/ui/view_actions.h"
#include "onyx/ui/system_actions.h"
#include "onyx/ui/search_view.h"
#include "onyx/ui/reading_tools_actions.h"
#include "onyx/ui/zoom_setting_actions.h"



namespace onyx {

class MainWidget;

/// Office view serves as reading view associated with officereader instance.
/// If process user request is set to enable, it will process user event directly.
/// otherwise, it's readonly and it will forward all user input to listener.
class OfficeView : public QWidget {
    Q_OBJECT

public:
    OfficeView(OfficeReader & instance, QWidget *parent);
    ~OfficeView();

public Q_SLOTS:
    bool open(const QString &path);
    bool close();

    void showContextMenu();
    void setFont(const int index);
    void resizeBackend(const QSize& s);

Q_SIGNALS:
    void pageChanged(int, int);
    void docClosed();
    void requestGotoPageDialog();
    void requestClockDialog();
    void fullScreen(bool full);

protected:
    void keyReleaseEvent(QKeyEvent* e);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void resizeEvent(QResizeEvent * event);
    void paintEvent(QPaintEvent *e);

private Q_SLOTS:
    void onSplashScreenDone();
    void onJobFinished();
    void onDocumentOpened();
    bool addBookmark();
    bool deleteBookmark();
    void processToolActions();
    void processZoomingActions();
    void processSystemActions();

    void gotoPage();
    void showClock();

    void onInsufficientMemory();
    void onSearchStateChanged(int);

    void onSdCardChanged(bool);
    void onWakeup();
    void handleMountTreeEvent(bool inserted, const QString &mount_point);
    void onAboutToShutdown();
    void onMusicPlayerStateChanged(int);

private:
    bool updateActions();
    void updateFontSizeActions();
    void updateZoomingActions();
    void updateToolActions();
    void updateConf();
    bool restoreState();
    void useDefaultState();

    void paintPicselSplash(QPainter &painter);
    void paintPage(QPainter & painter);

    void saveThumbnail();

    bool hasBookmark();
    void getBookmarksModel(QStandardItemModel & bookmarks_model);
    void displayBookmarks();

    void showSearchWidget();

    bool showSplash();

    bool isFullScreenByWidgetSize();

private:
    OfficeReader & instance_;
    MainWidget * main_widget_;
    QPoint last_pos_;

    bool mdb_found_;

    FontIndexActions font_actions_;///< NOTE this is a FontIndexActions variable
    SystemActions system_actions_;
    ReadingToolsActions reading_tool_actions_;
    ZoomSettingActions zoom_setting_actions_;

    vbf::Configuration conf_;
    scoped_ptr<SearchWidget> search_widget_;

};  // OfficeView


};  // namespace onyx

#endif

