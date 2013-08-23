#ifndef READER_APPLICATION_H_
#define READER_APPLICATION_H_

#include "onyx/base/base.h"
#include "onyx/base/dbus.h"
#include "frame.h"
#include "onyx/ui/ui.h"

namespace reader
{

class ReaderApplication : public QApplication
{
    Q_OBJECT;

public:
    ReaderApplication(int &argc, char **argv);
    ~ReaderApplication(void);

public Q_SLOTS:
    bool open(const QString & path_name);
    bool close(const QString & path_name);
    void suspend();

private:
    void initTheme();
    void loadExternalFonts();

private:
    ReaderFrame main_window_;
    NO_COPY_AND_ASSIGN(ReaderApplication);
};

/// Please place all dbus related sutff in this class.
/// TODO: Put it into baseframework so that all viewers can
/// use it more easily.
class ReaderApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.htmlreader");

public:
    ReaderApplicationAdaptor(ReaderApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().registerService("com.onyx.service.htmlreader");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/htmlreader", app_);
    }

public Q_SLOTS:
    bool open(const QString & path) { return app_->open(path); }
    bool close(const QString & path) { return app_->close(path); }
    void suspend() { app_->suspend(); }

private:
    ReaderApplication *app_;
    NO_COPY_AND_ASSIGN(ReaderApplicationAdaptor);
};

}  // namespace reader

#endif  // READER_APPLICATION_H_
