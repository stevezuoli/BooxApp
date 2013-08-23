#include "explorer/include/main_window.h"
#include "explorer_conf.h"
#include "system_controller.h"
#include "onyx/base/device.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/cms/user_db.h"
#include "explorer/include/startup_dialog.h"

static const QString RESUME_TAG = "resume_book";

namespace explorer
{

using controller::SystemController;

namespace view
{

MainWindow::MainWindow(model::ModelTree & model)
#ifndef Q_WS_QWS
    : QWidget(0, 0)
#else
    : QWidget(0, Qt::FramelessWindowHint)
#endif
    , sys_proxy_(SysStatus::instance())
    , layout_(this)
    , model_view_(0, model)
{
    // Initialize size
#ifndef Q_WS_QWS
    setFixedSize(600, 800);
#else
    setFixedSize(qApp->desktop()->screenGeometry().size());
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif
    // Auto fill background.
    // setAutoFillBackground(true);
    // setBackgroundRole(QPalette::Base);
    createLayout();

    // Setup connection.
    installSystemSlots();
    connect(&SystemController::instance(),
            SIGNAL(viewerStateChanged(ServiceState)),
            this,
            SLOT(onViewerStateChanged(ServiceState)));

    connect(&model_view_, SIGNAL(outOfRoot()), this, SLOT(onOutOfRoot()));
}

MainWindow::~MainWindow()
{
}

void MainWindow::updateAll()
{
    show();
    model_view_.updateAll(true);
    QApplication::processEvents();
    openResumeDocument();
}

void MainWindow::createLayout()
{
    layout_.setSpacing(2);
    layout_.setContentsMargins(2, 2, 2, 2);

    layout_.addWidget(&model_view_);

    // You are allowed to receive keyboard message now.
    showModelView(true);
}

/// Service state handler.
/// When a service can not open a document, main window may give an error
/// message to end user. It's also necessary to lock user interfacw when
/// a service is opening a document.
void MainWindow::onViewerStateChanged(ServiceState new_state)
{
    // It's better for viewer to emit a signal.
    // We launch a timer here to check if viewer can launch
    // in specified time.
    if (new_state == SERVICE_LAUNCHING)
    {
        // Disable screen update now.
        sys_proxy_.setSystemBusy(true);
        onyx::screen::instance().enableUpdate(false);
    }


    // If viewer crashed or closed, we need to update the recent document list.
    if (new_state == SERVICE_CLOSED)
    {
        // Not necessary now.
        QProcess::startDetached("explorer_cleanup.sh");

        // Always sync filesystem.
        SystemController::instance().syncFileSystem();

        // Enable screen update now.
        onyx::screen::instance().enableUpdate(false);
        model_view_.resort();
        onyx::screen::instance().flush();

        sys_proxy_.setSystemBusy(false);

        if (isActiveWindow())
        {
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC);
        }

        QTimer::singleShot(0, &model_view_, SLOT(onViewerClosed()));
    }
}

void MainWindow::installSystemSlots()
{
    connect(&sys_proxy_,
            SIGNAL(mountTreeSignal(bool, const QString &)),
            this,
            SLOT(handleMountTreeEvent(bool, const QString &)));

    connect(&sys_proxy_,
            SIGNAL(sdCardChangedSignal(bool)),
            this,
            SLOT(onSdCardChanged(bool)));

    connect(&sys_proxy_,
            SIGNAL(batterySignal(const int, int)),
            this,
            SLOT(handleBatteryEvent(const int, int)));

    connect(&sys_proxy_,
            SIGNAL(aboutToSuspend()),
            this,
            SLOT(onAboutToSuspend()));

    connect(&sys_proxy_,
            SIGNAL(aboutToShutdown()),
            this,
            SLOT(onAboutToShutdown()));

    connect(&sys_proxy_,
            SIGNAL(forceQuit()),
            this,
            SLOT(onForceQuit()));

    connect(&sys_proxy_,
            SIGNAL(musicPlayerStateChanged(int)),
            this,
            SLOT(onMusicPlayerStateChanged(int)));

    connect(&sys_proxy_,
            SIGNAL(onlineServiceRequest(void)),
            this,
            SLOT(triggerOnlineService(void)));

    connect(&sys_proxy_,
            SIGNAL(downloadStateChanged(const QString &,int, bool)),
            this,
            SLOT(onDownloadStateChanged(const QString &,int, bool)));

    connect(&sys_proxy_,
            SIGNAL(connectToPC(bool)),
            this,
            SLOT(onConnectToPC(bool)));
}

bool MainWindow::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    if (event->type() == QEvent::UpdateRequest)
    {
        if (onyx::screen::instance().isUpdateEnabled() &&
            controller::SystemController::instance().runningViewers() <= 0 &&
            isActiveWindow())
        {
            static int count = 0;
            if (onyx::screen::instance().defaultWaveform() == onyx::screen::ScreenProxy::DW)
            {
                qDebug("Explorer screen ScreenProxy::DW update %d", count++);
                onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW, true);
                onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
            }
            else
            {
                qDebug("Explorer screen full update %d", count++);
                onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::INVALID);
            }
        }
    }
    return ret;
}

void MainWindow::resizeEvent(QResizeEvent *)
{
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
    }
}

void MainWindow::onScreenSizeChanged(int)
{
    // Preserve updatability.
    bool enabled = onyx::screen::instance().isUpdateEnabled();
    onyx::screen::instance().enableUpdate(false);

    setFixedSize(qApp->desktop()->screenGeometry().size());
    QApplication::processEvents();

    onyx::screen::instance().enableUpdate(enabled);

    if (isActiveWindow())
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC);
    }
}

/// When SD card is gone, we mark the sd is dirty.
/// The dirty flag is used in syncFileSystem. The reason is that
/// sometimes, the mount tree can not be correctly updated. So we
/// have to make a flag here and check it later.
void MainWindow::onSdCardChanged(bool insert)
{
    SystemController::instance().markSdDirty(!insert);
}

/// Handle mount tree changed signal.
void MainWindow::handleMountTreeEvent(bool inserted, const QString &mount_point)
{
    // Update model
    if (mount_point == SDMMC_ROOT)
    {
        SystemController::instance().markSdDirty(false);
        model_view_.fileSystemChanged(NODE_TYPE_SD, inserted);
    }
    else if (mount_point == USB_ROOT)
    {
        model_view_.fileSystemChanged(NODE_TYPE_USB, inserted);
    }
    else if (mount_point == LIBRARY_ROOT)
    {
        model_view_.fileSystemChanged(NODE_TYPE_LIBRARY, inserted);
    }
}

/// This signal is emitted from viewer.
/// Viewer -> SysStatus -> SystemManager -> SysStatus -> Explorer
/// When receive the signal, explorer needs to start music player.
void MainWindow::onMusicPlayerStateChanged(int cmd)
{
    if (cmd == sys::START_PLAYER)
    {
        controller::SystemController::instance().requestMusicService(cmd);
    }
    else if (cmd == sys::HIDE_PLAYER)
    {
        onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
    }
}

void MainWindow::triggerOnlineService(void)
{
    model_view_.triggerOnlineService();
}

void MainWindow::onDownloadStateChanged(const QString &url,
                                        int state,
                                        bool open)
{
    model_view_.onDownloadStateChanged(url, state, open);
}

void MainWindow::handleBatteryEvent(const int current, int level)
{
    // Maybe not necessary to handle the event here.
    if (level == BATTERY_STATUS_DANGEROUS)
    {
        model_view_.shutdown(SHUTDOWN_REASON_LOW_BATTERY);
    }
}

void MainWindow::onAboutToSuspend()
{
    qDebug("Process onAboutToSuspend signal.");

    // Not necessary to wait now.
    // Need to make sure that viewers have been stored their options.
    //int count = controller::SystemController::instance().runningViewers();
    //if (count > 0)
    //{
    //    QTimer::singleShot(1500, this, SLOT(suspend()));
    //}
    suspend();
}

void MainWindow::onAboutToShutdown()
{
    // Check if there is any applications running.
    int count = controller::SystemController::instance().runningViewers();
    if (count > 0)
    {
        // Store the book that we're opening.
        saveResumeDocument(controller::SystemController::instance().lastDocument());

        // Give applications time to store options.
        QTimer::singleShot(1500, this, SLOT(shutdown()));
    }
    else
    {
        shutdown();
    }
}

void MainWindow::shutdown()
{
    int current,status = BATTERY_STATUS_NORMAL;
    sys_proxy_.batteryStatus(current,status);
    if (status == BATTERY_STATUS_DANGEROUS)
    {
        model_view_.shutdown(SHUTDOWN_REASON_LOW_BATTERY);
    }
    else
    {
        model_view_.shutdown(SHUTDOWN_REASON_USER_REQUEST);
    }
}

void MainWindow::suspend()
{
    qDebug("Explorer: going to suspend.");
    model_view_.suspend();
}

/// Basically, we need a dialog in the system manager. So far, just kill
/// current application.
void MainWindow::onForceQuit()
{
    // Enable explorer to receive GUI events.
    sys_proxy_.setSystemBusy(false);

    /// Don't kill calibration.
    if (controller::SystemController::instance().isCalibrationRunning())
    {
        return;
    }

    // Always enable update now, as not all viewer implement the
    // dbus notification correctly.
    onyx::screen::instance().enableUpdate(true);
    controller::SystemController::instance().stopAllApplications(false);
}

void MainWindow::onOutOfRoot()
{

}

void MainWindow::onNodeClicked(explorer::model::NodeType node)
{
    model_view_.model().cdDesktop();
    switch (node)
    {
    case NODE_TYPE_ROOT:
        break;
    case NODE_TYPE_SYS_SETTINGS:
        model_view_.model().cdBranch(NODE_TYPE_SYS_SETTINGS);
        break;
    case NODE_TYPE_APPLICATIONS:
        model_view_.model().cdBranch(NODE_TYPE_APPLICATIONS);
        break;
    case NODE_TYPE_LIBRARY:
        model_view_.model().cdBranch(NODE_TYPE_LIBRARY);
        break;
    }

    model_view_.updateAll();
    showModelView(true);
}

void MainWindow::showModelView(bool show)
{
    for(int i = 0; i < layout_.count(); ++i)
    {
        QWidget *wnd = layout_.itemAt(i)->widget();
        if (wnd != &model_view_)
        {
            wnd->setVisible(!show);
        }
    }
    model_view_.setVisible(show);
    if (show)
    {
        model_view_.setFocus();
    }
}

void MainWindow::onConnectToPC(bool connected)
{
    // For connected, system manager will broadcast mount tree changed signal.
    if (!connected)
    {
        model_view_.updateAll(true);
    }
}

void MainWindow::saveResumeDocument(const QString & path)
{
    cms::UserDB db;
    db.store(RESUME_TAG, path);
}

QString MainWindow::resumeDocument()
{
    cms::UserDB db;
    QVariant value;
    db.load(RESUME_TAG, value);
    QString path  = value.toString();
    db.store(RESUME_TAG, "");
    return path;
}

void MainWindow::openResumeDocument()
{
    SystemConfig cfg;
    StartupDialog::StartupSettingType type = StartupDialog::getStartupSettingType(
            cfg.miscValue(StartupDialog::getKeyForMiscConf()));
    if (type != StartupDialog::OPEN_MOST_RECENT_DOC)
    {
        return;
    }

    QString path = resumeDocument();
    if (!path.isEmpty() && QFile::exists(path))
    {
        SystemController::instance().openContentFile(path);
    }
}


}  // namespace view

}  // namespace explorer

