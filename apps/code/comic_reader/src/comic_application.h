
#ifndef COMIC_APPLICATION_H_
#define COMIC_APPLICATION_H_

#include "onyx/base/base.h"
#include "onyx/base/dbus.h"
#include "onyx/ui/ui.h"

#include "comic_model.h"
#include "comic_main_window.h"

namespace comic_reader
{

class ComicApplication : public QApplication
{
    Q_OBJECT

public:
    ComicApplication(int &argc, char **argv);
    ~ComicApplication(void);

    const QString & currentPath();

public Q_SLOTS:
    bool open(const QString & path);
    bool close(const QString & path);

    void onWakeUp();
    void onUSBSignal(bool inserted);
    void onSDChangedSignal(bool inserted);
    void onMountTreeSignal(bool inserted, const QString &mount_point);
    void onAboutToShutDown();

private:
    ComicModel model_;      ///< comic reader model
    ComicMainWindow main_window_; ///< main window of the comic reader
    QString    current_path_;     ///< path of the current document
};

class ComicApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;

    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.comic_reader");

public:
    ComicApplicationAdaptor(ComicApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().registerService("com.onyx.service.comic_reader");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/comic_reader", app_);
    }

public Q_SLOTS:
    bool open(const QString & path) { return app_->open(path); }
    bool close(const QString & path) { return app_->close(path); }

private:
    ComicApplication *app_;
};

}   // namespace comic_reader

#endif
