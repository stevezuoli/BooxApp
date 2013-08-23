#ifndef TEXT_APPLICATION_H_
#define TEXT_APPLICATION_H_

#include "text_frame.h"

using namespace ui;

namespace text_editor
{

/// Naboo Application.
class TextApplication : public QApplication
{
    Q_OBJECT
public:
    TextApplication(int &argc, char **argv);
    ~TextApplication(void);

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
    void onConnectToPCSignal(bool connected);
    void onBatterySignal(const int, const int, bool);
    void onSystemIdleSignal();
    void onAboutToShutDown();

    void onRotateScreen();
    void onScreenSizeChanged(int);

private:
    scoped_ptr<TextFrame>  frame_;
    QString                current_path_;     // path of the current document

    NO_COPY_AND_ASSIGN(TextApplication);
};


class TextApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;

    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.text_editor");

public:
    TextApplicationAdaptor(TextApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().registerService("com.onyx.service.text_editor");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/text_editor", app_);
    }

public Q_SLOTS:
    /// Must be in dbus data type, otherwise these methods will not
    /// be exported as dbus methods. So you can not use std::string
    /// here.
    bool open(const QString & path) { return app_->open(path); }
    bool close(const QString & path) { return app_->close(path); }
    bool suspend() { return app_->suspend(); }

private:
    TextApplication *app_;
    NO_COPY_AND_ASSIGN(TextApplicationAdaptor);

};  // TextApplicationAdaptor

};

#endif
