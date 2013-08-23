#ifndef EXPLORER_SYSTEM_CONTROLLER_H_
#define EXPLORER_SYSTEM_CONTROLLER_H_

#include "onyx/base/dbus.h"
#include "onyx/sys/sys.h"
#include "file_node.h"

using namespace sys;

namespace explorer
{

namespace model
{

class Node;

}

namespace controller
{

/// Define all states. NOT finished.
enum SystemState
{
    STATE_INVALID             = -1,
    STATE_SYSTEM_SETTINGS     = 0,    ///< System setting.
    STATE_PASSWORD,                   ///< Password authentication.
    STATE_BROWSING,                   ///< Browsing content.
    STATE_READING,                    ///< In content reading. Viewer launched.
    STATE_STAND_BY,                   ///< Stand by.
    STATE_USB_CONNECTED,              ///< Connected to PC or other host.
    STATE_SHUT_DOWN
};

enum ServiceState
{
    SERVICE_INVALID,
    SERVICE_LAUNCHING,          ///< Launching service, not running yet.
    SERVICE_ERROR,              ///< Error occus. DBus error.
    SERVICE_TIMEOUT,            ///< Service timeout.
    SERVICE_FAILED,             ///< The service could not open the document.
    SERVICE_RUNNING,            ///< DBus method call succeeded.
    SERVICE_CLOSED,             ///< Release dbus connection.
    SERVICE_CRASHED,            ///< Crashed.
};


/// Explorer controller.
/// - Maintain system state.
/// - Manage state transmission.
/// - Report state changed.
/// - Receive signals from DBus.
class SystemController : public QObject
{
    Q_OBJECT

public:
    static SystemController & instance()
    {
        static SystemController _instance;
        return _instance;
    }
    ~SystemController(void);

    bool openContentFile(const QString &path);
    const QString & lastDocument();

    bool navigateTo(const QString &url);
    bool startCalibration();
    bool isCalibrationRunning();
    bool startNetworkManager();
    void stopAllApplications(bool wait_for_finished);
    int runningViewers();

    bool startFeedReader();
    bool startSudoku();

    void requestMusicService(int);
    void openDictionary();

    void createNote(const QString &note_name);
    void editNote(const QString &name);

    void returnLoanBook(const QString & path);

    void markSdDirty(bool dirty);
    bool isSdDirty() { return sd_card_dirty_; }
    void syncFileSystem();
    bool formatFlash();

    void suspend(SysStatus &ref);
    void shutdown(SysStatus &ref, int r = SHUTDOWN_REASON_USER_REQUEST);

    bool extractMetadata(const QString &path);
    void stopExtracting();

    bool startWritePad(const QString &path);
    bool hasTouch();

    bool startVCOMManager();
    Service & officeViewerService();
    Service & onyxReaderService();
    Service & nabooReaderService();

    Service & getEpubService();
    Service & getDocService();
    void setDocService(Service & service);
    void setEpubService(Service & service);

Q_SIGNALS:
    /// Viewer state changed signal. This signal is emitted when
    /// a viewer is launched, launchs failed or succeed.
    void viewerStateChanged(ServiceState new_state);

    /// Content metadata ready.
    void metadataReady(const QString & path);

private Q_SLOTS:
    // Internal helper functions.
    void onProcessStateChanged(QProcess::ProcessState);
    void onProcessFinished(int, QProcess::ExitStatus);

    void onFileOpened(const QDBusMessage &message);
    void onFileClosed(const QDBusMessage &message);
    void onMethodTimeOut();

    void serviceOwnerChanged(const QString &name,
                             const QString &oldOwner,
                             const QString &newOwner);

    void suspendAllApplications();

    void onMetFinished(int, QProcess::ExitStatus);

private:
    struct Viewer
    {
        Service service;
        ServiceState state;
        QProcess process;
    };
    typedef Viewer * ViewerPtr;
    typedef std::map<QString, ViewerPtr> ApplicationTable;
    typedef ApplicationTable::iterator ApplicationTableIter;

private:
    SystemController(void);
    SystemController(SystemController&);

    bool initializeServiceTable();
    void clearServiceTable();
    Service & service(const QString & extension_name);
    Service & findService(const Service & service);
    Service & calibration_service() { return calibration_service_; }
    Service & music_service() { return music_service_; }
    Service & note_service() { return note_service_; }
    Service & networkService() { return network_service_; }
    Service & browserService() { return browser_service_; }
    Service & DRMService() { return drm_service_; }
    Service & metService() { return met_service_; }
    Service & writePadService() { return write_pad_service_; }
    Service & dictService() { return dict_service_; }
    Service & rssService() { return rss_service_; }
    Service & sudokuService() { return sudoku_service_; }

    void onServiceLaunched(const QString &name);
    void onServiceStopped(const QString &name);

    void setupConnection();

    ViewerPtr startViewer(Service & service, const QString &path, const QStringList &additional_args = QStringList());
    int maxDocuments();

    QString suffix(const QFileInfo & info);
    void setupDisplayParameters(QStringList &args);

    void postStart(ViewerPtr ptr);
    void postStop(ViewerPtr ptr);

private:
    bool handle_input_;   ///< Enable input or not.
    bool sd_card_dirty_;
    QString last_document_;
    Services all_services_;
    Service calibration_service_;
    Service music_service_;
    Service note_service_;
    Service network_service_;
    Service browser_service_;
    Service drm_service_;
    Service met_service_;
    Service write_pad_service_;
    Service dict_service_;
    Service rss_service_;
    Service sudoku_service_;

    QDBusConnection connection_;
    ApplicationTable applications_;     ///< Running instances.

    QProcess met_process_;  ///< To extract metadata.
    QString met_doc_path_;
};

}  // namespace model

}  // namespace explorer

#endif
