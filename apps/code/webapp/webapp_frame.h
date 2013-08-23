#ifndef WEB_FRAME_H_
#define WEB_FRAME_H_

#include "onyx/ui/ui.h"
#include "onyx/sys/sys_status.h"
#include "network_service/download_view.h"
#include "webapp_view.h"
#include "webapp_test.h"
#include "webapp_onyx_db.h"
#include "webapp_security_manager.h"
#include "webapp_download_manager.h"

using namespace ui;

namespace webapp
{

class WebFrame : public QWidget
{
    Q_OBJECT

public:
    WebFrame(QWidget *parent = 0);
    ~WebFrame();

    WebView * view() { return &web_view_; }

public Q_SLOTS:
    void onScreenSizeChanged(int);
    void populateJavaScriptWindowObject();
    void updateRegion(int x, int y, int width, int height, const QString & update_type);
    void onLoadFinished(const QString & file_path);

    void onDownloadItemAdded(DownloadItem *item);

protected:
    void keyPressEvent(QKeyEvent * ke);
    void keyReleaseEvent(QKeyEvent *ke);
    bool event(QEvent *e);
    void closeEvent(QCloseEvent *e);

private Q_SLOTS:

private:
    void createLayout();

private:
    WebAppView                          web_view_;
    DownloadView                        download_view_;
    QRect                               web_view_update_rect_;
    onyx::screen::ScreenProxy::Waveform web_view_waveform_;
    QVBoxLayout                         layout_;
    SysStatus &                         sys_;
    StatusBar                           status_bar_;

    // objects
    OnyxDB                              onyx_db_;
    SecurityManager                     security_manager_;
    OnyxDownloadManager                 download_manager_;
};

}   // namespace webapp

#endif  // WEB_FRAME_H_
