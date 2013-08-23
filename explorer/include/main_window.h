#ifndef EXPLORER_MAINWINDOW_H_
#define EXPLORER_MAINWINDOW_H_

#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "model_tree.h"
#include "system_controller.h"
#include "model_view.h"

namespace explorer
{

using namespace controller;

namespace view
{

/// Main window of explorer. It manages all its child windows.
/// Switch between these windows.
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(model::ModelTree & model);
    ~MainWindow();

public Q_SLOTS:
    void updateAll();

    void onSdCardChanged(bool insert);
    void handleMountTreeEvent(bool inserted, const QString &mount_point);
    void handleBatteryEvent(const int, int);
    void onMusicPlayerStateChanged(int);
    void triggerOnlineService(void);
    void onDownloadStateChanged(const QString &, int, bool);

    void onAboutToSuspend();
    void suspend();
    void onAboutToShutdown();
    void shutdown();

    void onForceQuit();

    void onViewerStateChanged(ServiceState new_state);
    void onScreenSizeChanged(int);

    void onOutOfRoot();
    void onNodeClicked(explorer::model::NodeType node);

    void onConnectToPC(bool);

    void saveResumeDocument(const QString & path);
    void openResumeDocument();
    QString resumeDocument();

protected:
    bool event(QEvent * event);
    void resizeEvent(QResizeEvent *);
    void changeEvent(QEvent *event);

private:
    void installSystemSlots();

    void createLayout();
    void updateMainWindow(const int id);

    void showModelView(bool);

private:
    SysStatus & sys_proxy_;     ///< Device state watcher.
    QVBoxLayout layout_;
    ModelView model_view_;
};

}  // namespace view

}  // namespace explorer

#endif
