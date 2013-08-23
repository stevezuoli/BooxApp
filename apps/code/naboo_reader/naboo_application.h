#ifndef NABOO_APPLICATION_H_
#define NABOO_APPLICATION_H_

#include "naboo_utils.h"
#include "naboo_model.h"
#include "naboo_view.h"

using namespace ui;
using namespace vbf;

namespace naboo_reader
{

/// Naboo Application.
class NabooApplication : public QApplication
{
    Q_OBJECT
public:
    NabooApplication(int &argc, char **argv);
    ~NabooApplication(void);

    const QString & currentPath() { return current_path_; }

public Q_SLOTS:
    bool open(const QString &path_name);
    bool close(const QString &path_name);
    bool isOpened();
    bool errorFound();
    bool suspend();

    void onWakeUp();
    void onUSBSignal(bool inserted);
    void onSDChangedSignal(bool inserted);
    void onMountTreeSignal(bool inserted, const QString &mount_point);
    void onBatterySignal(const int, const int, bool);
    void onSystemIdleSignal();
    void onAboutToShutDown();

    void onCreateView(int type, MainWindow* main_window, QWidget*& result);
    void onAttachView(int type, QWidget* view, MainWindow* main_window);
    void onDeattachView(int type, QWidget* view, MainWindow* main_window);

    void onRotateScreen();
    void onScreenSizeChanged(int);

    bool flip(int);

private:
    MainWindow main_window_;
    NabooModel model_;            // Naboo model instance
    QString    current_path_;     // path of the current document

    NO_COPY_AND_ASSIGN(NabooApplication);
};


class NabooApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;

    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.naboo_viewer");

public:
    NabooApplicationAdaptor(NabooApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().registerService("com.onyx.service.naboo_viewer");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/naboo_viewer", app_);
    }

public Q_SLOTS:
    /// Must be in dbus data type, otherwise these methods will not
    /// be exported as dbus methods. So you can not use std::string
    /// here.
    bool open(const QString & path) { return app_->open(path); }
    bool close(const QString & path) { return app_->close(path); }
    bool suspend() { return app_->suspend(); }

    bool flip(int);

private:
    NabooApplication *app_;
    NO_COPY_AND_ASSIGN(NabooApplicationAdaptor);

};  // NabooApplicationAdaptor

};

#endif
