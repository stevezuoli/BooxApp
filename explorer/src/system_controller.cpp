#include "node.h"
#include "system_controller.h"
#include "explorer_conf.h"
#include "onyx/sys/sys_utils.h"

#ifdef BUILD_FOR_ARM
#include <QtGui/qscreen_qws.h>
#endif

using namespace sys;

namespace explorer
{

namespace controller
{

static bool has_touch = true;

// The default initial state may be read from settings file.
// It would be possible to read it from config.xml file,
// so we are able to provide feature like "first language settings" etc.
SystemController::SystemController()
: handle_input_(true)
, sd_card_dirty_(false)
, connection_(QDBusConnection::systemBus())
{
    initializeServiceTable();
    setupConnection();
    has_touch = sys::SysStatus::instance().hasTouchScreen();
}

SystemController::~SystemController()
{
    clearServiceTable();
    stopAllApplications(false);
}

/// Load all services.
bool SystemController::initializeServiceTable()
{
    SystemConfig conf;
    conf.loadAllServices(all_services_);
    conf.calibrationService(calibration_service_);
    conf.musicService(music_service_);
    conf.noteService(note_service_);
    conf.networkService(network_service_);
    conf.webBrowserService(browser_service_);
    conf.DRMService(drm_service_);
    conf.metService(met_service_);
    conf.writePadService(write_pad_service_);
    conf.dictionaryService(dict_service_);
    conf.rssService(rss_service_);
    conf.sudokuService(sudoku_service_);
    return true;
}

/// Retrieve service name and executable path for the extension name.
/// @param ext The extension name associated with the service.
/// @param service The service name.
/// @param path The executable file path name.
Service & SystemController::service(const QString& ext)
{
    static Service g_dummy_service;
    for(ServicesIter iter = all_services_.begin(); iter != all_services_.end(); ++iter)
    {
        if (iter->extensions().contains(ext))
        {
            return *iter;
        }
    }
    return g_dummy_service;
}

void SystemController::setupConnection()
{
    if (connection_.isConnected())
    {
        QDBusConnectionInterface *iface = connection_.interface();
        connect(iface, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                this, SLOT(serviceOwnerChanged(QString,QString,QString)));
    }
    else
    {
        qWarning("Cannot connect to D-Bus: %s" , qPrintable(connection_.lastError().message()));
    }

    connect(&met_process_, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onMetFinished(int, QProcess::ExitStatus)));
}


/// Search in the running table to check if calibration is running.
bool SystemController::isCalibrationRunning()
{
    ApplicationTableIter iter = applications_.find(calibration_service_.app_name());
    return (iter != applications_.end());
}

void SystemController::onServiceLaunched(const QString &name)
{
    // Search in all running applications.
    for(ApplicationTableIter iter = applications_.begin(); iter != applications_.end(); ++iter)
    {
        if (iter->second->service.service_name() == name)
        {
            // Setup connection.
            connection_.connect(iter->second->service.service_name(), QString(),
                iter->second->service.interface_name(),
                "documentOpened",
                this,
                SLOT(onFileOpened(const QDBusMessage &)));
            connection_.connect(iter->second->service.service_name(), QString(),
                iter->second->service.interface_name(),
                "documentClosed",
                this,
                SLOT(onFileOpened(const QDBusMessage &)));
            return;
        }
    }
}

void SystemController::onServiceStopped(const QString &name)
{
}

void SystemController::clearServiceTable()
{
    all_services_.clear();
}

/// When this function is invoked, the service could be launched or
/// could also be killed.
/// @param name The service name.
void SystemController::serviceOwnerChanged(const QString &name,
                                           const QString &oldOwner,
                                           const QString &newOwner)
{
    // We can move the following code to OnError and OnReply.
    // We still need some test.
    // So far, we can always
    if (oldOwner.size() <= 0 && newOwner.size() > 0)
    {
        onServiceLaunched(name);
    }
    else if (oldOwner.size() > 0 && newOwner.size() <= 0)
    {
        onServiceStopped(name);
    }
}


/// Get suffix from the file info.
/// For compressed file, it tries to detect the raw file inside the archive
/// and returns the file suffix directly.
QString SystemController::suffix(const QFileInfo & info)
{
    return info.suffix().toLower();
}

void SystemController::setupDisplayParameters(QStringList &args)
{
#ifdef BUILD_FOR_ARM
    args << "-display";
    QString disp("Transformed:Rot%1:OnyxScreen:/dev/mem");
    int degree = QScreen::instance()->transformOrientation() * 90;
    disp = disp.arg(degree);
    qDebug("disp %s", qPrintable(disp));
    args << disp;
#endif
}

/// The listener may lock user interface when it's launching.
void SystemController::onProcessStateChanged(QProcess::ProcessState state)
{
    ServiceState service_state = SERVICE_INVALID;
    if (state == QProcess::NotRunning)
    {
        // It does not run at all.
        service_state = SERVICE_CLOSED;
    }
    else if (state == QProcess::Starting || state == QProcess::Running)
    {
        service_state = SERVICE_LAUNCHING;
    }
    emit viewerStateChanged(service_state);
}

/// Event handler. It's called when a process is finished either closed or
/// crashed. This function updates the running table.
void SystemController::onProcessFinished(int exitCode,
                                         QProcess::ExitStatus exitStatus)
{
    // Not necessary to emit again here. As the signal is emitted
    // by onStateChanged.
    for(ApplicationTableIter iter = applications_.begin(); iter != applications_.end(); ++iter)
    {
        if (iter->second->process.exitStatus() == exitStatus &&
            sender() == &iter->second->process)
        {
            // Should disconnect signal, otherwise, this function will be invoked again.
            disconnect(&iter->second->process, SIGNAL(stateChanged(QProcess::ProcessState)),
                this, SLOT(onProcessStateChanged(QProcess::ProcessState)));
            disconnect(&iter->second->process, SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));

            postStop(iter->second);

            delete iter->second;
            applications_.erase(iter);
            return;
        }
    }
}

void SystemController::onFileOpened(const QDBusMessage &message)
{
    // qDebug("document opened %s", qPrintable(message.arguments())
    emit viewerStateChanged(SERVICE_RUNNING);

    // Update the opened document list.
}

void SystemController::onFileClosed(const QDBusMessage &message)
{
    // Update the opened document list.
}

/// It will be invoked when timeout after launching.
void SystemController::onMethodTimeOut()
{
    qDebug("time out!");
    //emit serviceStateChanged(services_.currentService(),
    //                         SERVICE_LAUNCHING,
    //                         SERVICE_TIMEOUT);
}

/// Try to open content. Maybe should change name to exec.
/// \return true means the content has been successfully opened.
/// It returns false when encountering any problems.
bool SystemController::openContentFile(const QString &path)
{
    QFileInfo info(path);
    QString ext = suffix(info);
    ViewerPtr ptr = 0;

    // Remember the last open document.
    last_document_ = path;

    // Check the music service can open it or not.
    if (music_service_.extensions().contains(ext) &&
        sys::SystemConfig::isMusicPlayerAvailable())
    {
        ptr = startViewer(music_service(), path);
        if (ptr == 0)
        {
            last_document_.clear();
        }
        return (ptr != 0);
    }

    // Check if we can open more documents.
    if (runningViewers() >= maxDocuments())
    {
        last_document_.clear();
        return false;
    }

    Service & svr = service(ext);
    ptr = startViewer(svr, path);
    if (ptr == 0)
    {
        last_document_.clear();
    }
    return (ptr != 0);
}

const QString & SystemController::lastDocument()
{
    return last_document_;
}

bool SystemController::navigateTo(const QString &url)
{
    ViewerPtr ptr = startViewer(browser_service_, url);
    return (ptr != 0);
}

/// Start the calibration application.
bool SystemController::startCalibration()
{
    // Check if the viewer is in launching
    ViewerPtr ptr = startViewer(calibration_service_, "");
    return (ptr != 0);
}

bool SystemController::startNetworkManager()
{
    ViewerPtr ptr = startViewer(network_service_, "");
    return (ptr != 0);
}

/// Stop all running applications.
void SystemController::stopAllApplications(bool wait_for_finished)
{
    for(ApplicationTableIter iter = applications_.begin(); iter != applications_.end(); ++iter)
    {
        // Need to disconnect everything, otherwise the slots will be called.
        disconnect(&iter->second->process, SIGNAL(stateChanged(QProcess::ProcessState)),
            this, SLOT(onProcessStateChanged(QProcess::ProcessState)));
        disconnect(&iter->second->process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));

        iter->second->process.kill();

        // We always wait to make sure it's killed.
        if (wait_for_finished)
        {
            iter->second->process.waitForFinished();
        }
        else
        {
            iter->second->process.waitForFinished(500);
        }
        delete iter->second;
    }
    applications_.clear();

    // Also stop background applications.
    stopExtracting();

    sys::SysStatus::instance().resetIdle();
}

void SystemController::markSdDirty(bool dirty)
{
    // Make a flag
    sd_card_dirty_ = dirty;
}

/// This function is called when document is closed.
void SystemController::syncFileSystem()
{
    if (isSdDirty())
    {
        markSdDirty(false);
        QString cmd("umount %1");
        cmd = cmd.arg(SDMMC_ROOT);
        QProcess::startDetached(cmd);
    }
    QProcess::startDetached("sync");
}

bool SystemController::formatFlash()
{
    // TODO, need a dialog. Also need to update the model view.
    sys::SysStatus::instance().setSystemBusy(true);
    bool ok = (sys::runScript("format_flash.sh") == 0);
    sys::SysStatus::instance().setSystemBusy(false);
    return ok;
}

void SystemController::suspend(SysStatus &sys)
{
    // Ensure all applications suspend.
    suspendAllApplications();
    sys.suspend();
}

void SystemController::suspendAllApplications()
{
    ViewerPtr ptr = 0;
    for(ApplicationTableIter iter = applications_.begin(); iter != applications_.end(); ++iter)
    {
        ptr = iter->second;

        // Check if the application provides any service or not.
        if (ptr->service.service_name().isEmpty() ||
            ptr->service.object_path().isEmpty() ||
            ptr->service.interface_name().isEmpty())
        {
            continue;
        }

        // Check the service too.
        QDBusConnectionInterface *iface = connection_.interface();
        if (iface && iface->isServiceRegistered(ptr->service.service_name()))
        {
            // Now, tell the applications to suspend.
            QDBusMessage message = QDBusMessage::createMethodCall(ptr->service.service_name(),
                ptr->service.object_path(),
                ptr->service.interface_name(),
                "suspend");
            connection_.call(message, QDBus::Block, 1000);
        }
    }
}

/// Shutdown the device. First of all, make sure the viewer is closed.
/// Shutdown the device by calling system manager.
void SystemController::shutdown(SysStatus &sys, int reason)
{
    sys.shutdown(reason);
}

bool SystemController::extractMetadata(const QString &path)
{
    met_doc_path_.clear();

    // Before starting application.
    QFileInfo info(path);
    if (!cms::isImage(info.suffix()) /*&& !cms::couldContainMetadata(path)*/)
    {
        return false;
    }

    stopExtracting();
    QStringList args;
    setupDisplayParameters(args);
    if (!path.isEmpty())
    {
        args << path;
    }


    met_doc_path_ = path;
    met_process_.start(met_service_.app_name(), args);
    if (!met_process_.waitForStarted())
    {
        qWarning("Could not start application %s", qPrintable(met_service_.app_name()));
        return false;
    }
    return true;
}

bool SystemController::startWritePad(const QString & path)
{
    // Check if the viewer is in launching
    ViewerPtr ptr = startViewer(write_pad_service_, path);
    return (ptr != 0);
}

bool SystemController::hasTouch()
{
    return has_touch;
}

void SystemController::stopExtracting()
{
    met_process_.kill();
}

SystemController::ViewerPtr SystemController::startViewer(Service & service,
                                                          const QString &path,
                                                          const QStringList &additional_args)
{
    // Not possible to find a viewer.
    if (service.app_name().isEmpty() && !service.runnable())
    {
        return 0;
    }

    // If it's runnable file, we can execute it directly.
    if (service.runnable())
    {
        service.mutable_app_name() = path;
    }

    // Search in the running table.
    ViewerPtr ptr = 0;
    ApplicationTableIter iter = applications_.find(service.app_name());
    if (iter == applications_.end())
    {
        // Create the process. Although we can use dbus activation to create the process,
        // we have to create the process ourselves. The reason is that we have to pass
        // the display parameter to the process. Make sure the viewer can be found
        // in the $PATH.
        QStringList args;
        setupDisplayParameters(args);
        if (!path.isEmpty())
        {
            args << path;
        }

        if (additional_args.size() > 0)
        {
            args << additional_args;
        }

        ptr = new Viewer;
        ptr->service = service;
        ptr->state = SERVICE_LAUNCHING;
        ptr->process.setEnvironment(QProcess::systemEnvironment());

        connect(&ptr->process, SIGNAL(stateChanged(QProcess::ProcessState)),
            this, SLOT(onProcessStateChanged(QProcess::ProcessState)));
        connect(&ptr->process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));

        applications_[service.app_name()] = ptr;
        ptr->process.start(service.app_name(), args);

        // Need to cleanup if we can not start the process.
        if (ptr->process.error() == QProcess::FailedToStart)
        {
            onProcessFinished(0, QProcess::NormalExit);
            ptr = 0;
        }

        // Add idle reference.
        postStart(ptr);
    }
    else
    {
        // There is already this kind of service now.
        // Check if we can use dbus to open the document.
        ptr = iter->second;

        // Now, use dbus service to open it. Make sure the service is listed in /etc/dbus-1/system.conf
        // for security reason.
        QDBusMessage message = QDBusMessage::createMethodCall(service.service_name(),
            service.object_path(),
            service.interface_name(),
            service.method());
        message << path;

        connection_.call(message, QDBus::NoBlock);
    }
    return ptr;
}

/// Return how many viewers are launched. It does not take the music
/// player into account.
int SystemController::runningViewers()
{
    ApplicationTableIter iter = applications_.find(music_service().app_name());
    int count = 0;
    if (iter != applications_.end())
    {
        count = 1;
    }
    return applications_.size() - count;
}

void SystemController::requestMusicService(int cmd)
{
    if (cmd == sys::START_PLAYER)
    {
        startViewer(music_service(), "");
        return;
    }
}

bool SystemController::startFeedReader()
{
    return startViewer(rssService(), "");
}

bool SystemController::startSudoku()
{
    return startViewer(sudokuService(), "");
}

void SystemController::openDictionary()
{
    startViewer(dictService(), "");
}

bool SystemController::startVCOMManager()
{
    Service vcom("", "", "", "", "vcom_manager");
    return startViewer(vcom, "");
}
void SystemController::createNote(const QString &note_name)
{
    QStringList args("notes");
    startViewer(note_service(), note_name, args);
}

void SystemController::editNote(const QString &name)
{
    startViewer(note_service(), name);
}

void SystemController::returnLoanBook(const QString & path)
{
    QStringList args("return");
    startViewer(DRMService(), path, args);
}

/// Return the limitaion of document that we can open now.
/// For release 1.0, only one document can be opened.
int SystemController::maxDocuments()
{
    return 1;
}

/// Post start function. Called when viewer has been launched.
void SystemController::postStart(ViewerPtr ptr)
{
    if (ptr)
    {
        if (ptr->service.service_name() == music_service().service_name() ||
            ptr->service.service_name() == browserService().service_name())
        {
            sys::SysStatus::instance().enableIdle(false);
        }
    }
}

/// Post stop function. Called when viewer has been stopped.
void SystemController::postStop(ViewerPtr ptr)
{
    if (ptr)
    {
        if (ptr->service.service_name() == music_service().service_name() ||
            ptr->service.service_name() == browserService().service_name())
        {
            sys::SysStatus::instance().enableIdle(true);
        }
    }
}

Service & SystemController::findService(const Service & service)
{
    static Service g_dummy_service;
    for(ServicesIter iter = all_services_.begin(); iter != all_services_.end(); ++iter)
    {
        if (*iter == service)
        {
            return *iter;
        }
    }
    return g_dummy_service;
}

Service & SystemController::officeViewerService()
{
    Service office_viewer;
    SystemConfig conf;
    conf.officeViewerService(office_viewer);
    return findService(office_viewer);
}

Service & SystemController::onyxReaderService()
{
    Service onyx_reader;
    SystemConfig conf;
    conf.onyxReaderService(onyx_reader);
    return findService(onyx_reader);
}

Service & SystemController::nabooReaderService()
{
    Service naboo_reader;
    SystemConfig conf;
    conf.nabooReaderService(naboo_reader);
    return findService(naboo_reader);
}

void SystemController::onMetFinished(int, QProcess::ExitStatus status)
{
    if (!met_doc_path_.isEmpty())
    {
        emit metadataReady(met_doc_path_);
    }
}

Service & SystemController::getDocService()
{
    QString ext("doc");
    return service(ext);
}

void SystemController::setDocService(Service & service)
{
    QString ext("doc");
    Service & doc_service = getDocService();
    if ( doc_service ==  service )
    {
       return;
    }

    doc_service.mutable_extensions().push_front(ext);
    service.mutable_extensions().push_front(ext);

    SystemConfig conf;
    conf.unRegisterService(doc_service);
    conf.registerService(service,QString());

    doc_service.mutable_extensions().removeAll(ext);
}

Service & SystemController::getEpubService()
{
    QString ext("epub");
    return service(ext);
}

void SystemController::setEpubService(Service & service)
{
    QString ext("epub");
    Service & epub_service = getEpubService();
    if ( epub_service ==  service )
    {
       return;
    }

    epub_service.mutable_extensions().push_front(ext);
    service.mutable_extensions().push_front(ext);

    SystemConfig conf;
    conf.unRegisterService(epub_service);
    conf.registerService(service,QString());

    epub_service.mutable_extensions().removeAll(ext);
}

} // namespace controller

}  // namespace explorer
