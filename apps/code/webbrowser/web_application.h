#ifndef WEB_BROWSER_APPLICATION_H_
#define WEB_BROWSER_APPLICATION_H_

#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "onyx/base/dbus.h"
#include "network_service/access_manager.h"
#include "network_service/dm_manager.h"

#include "frame.h"
#include "bookmark_model.h"

using namespace network_service;

namespace webbrowser
{

class WebApplication : public QApplication
{
    Q_OBJECT;

public:
    WebApplication(int &argc, char **argv);
    ~WebApplication(void);

    static DownloadManager * downloadManager();
    static NetworkAccessManager * accessManager();

public Q_SLOTS:
    bool open(const QString & path_name);
    bool close(const QString & path_name);
    void suspend();

    void configNetwork();
    void scan();
    void connectTo(const QString &ssid, const QString &psk);

private:
    void initTheme();
    void loadExternalFonts();
    void loadSettings();

private:
    BrowserFrame* main_window_;
    scoped_ptr<BookmarkModel> bookmark_model_;

    NO_COPY_AND_ASSIGN(WebApplication);
};

/// Please place all dbus related sutff in this class.
/// TODO: Put it into baseframework so that all viewers can
/// use it more easily.
class WebApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.webbrowser");

public:
    WebApplicationAdaptor(WebApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().registerService("com.onyx.service.webbrowser");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/webbrowser", app_);
    }

public Q_SLOTS:
    bool open(const QString & path) { return app_->open(path); }
    bool close(const QString & path) { return app_->close(path); }
    void suspend() { return app_->suspend(); }

    void configNetwork() { app_->configNetwork(); }
    void scan() { app_->scan(); }
    void connectTo(const QString &ssid, const QString &psk) { app_->connectTo(ssid, psk); }

private:
    WebApplication *app_;
    NO_COPY_AND_ASSIGN(WebApplicationAdaptor);
};

}  // namespace webbrowser


#endif
