#include "onyx/ui/languages.h"
#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"
#include "comic_view.h"
#include "comic_application.h"

namespace comic_reader
{

ComicApplication::ComicApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , main_window_(this)
{
    if (argc > 1)
    {
        ui::loadTranslator(QLocale::system().name());
        current_path_ = QString::fromLocal8Bit(argv[1]);
    }
}

ComicApplication::~ComicApplication(void)
{
    close(current_path_);
}

const QString & ComicApplication::currentPath()
{
    return current_path_;
}

bool ComicApplication::open(const QString &path)
{
    main_window_.attachModel(&model_);
    main_window_.show();

    // connect the signals with sys_state_
    SysStatus & sys_status = SysStatus::instance();
    connect( &sys_status, SIGNAL( mountTreeSignal( bool, const QString & ) ),
             this, SLOT( onMountTreeSignal( bool, const QString &) ) );
    connect( &sys_status, SIGNAL( sdCardChangedSignal( bool ) ),
            this, SLOT( onSDChangedSignal( bool ) ) );
    connect( &sys_status, SIGNAL( aboutToShutdown() ),
            this, SLOT( onAboutToShutDown() ) );
    connect( &sys_status, SIGNAL( wakeup() ), this, SLOT( onWakeUp() ) );

    model_.open(path);
    bool ret = main_window_.open(path);
    if (!ret)
    {
        if (sys::SysStatus::instance().isSystemBusy())
        {
            // if loading fails, set busy to be false
            sys::SysStatus::instance().setSystemBusy(false);
        }
    }
    return ret;
}

bool ComicApplication::close(const QString & path)
{
    return true;
}

void ComicApplication::onWakeUp()
{
    qDebug("comic_reader: onWakeup");
    // set the system to be busy at this moment
    sys::SysStatus::instance().setSystemBusy(true, false);

    // save the configurations before closing
    model_.save();

    // close the document because it might be already invalid
    model_.close();
    main_window_.closeFile();

    // open the document again
    model_.open(current_path_);
    main_window_.open(current_path_);
    sys::SysStatus::instance().setSystemBusy(false, false);
}

void ComicApplication::onUSBSignal(bool inserted)
{
    qDebug("comic_reader: USB %s", inserted ? "inserted" : "disconnect");
    if (model_.path().startsWith(USB_ROOT) && !inserted)
    {
        qApp->exit();
    }
}

void ComicApplication::onSDChangedSignal(bool inserted)
{
    qDebug("comic_reader: SD %s", inserted ? "inserted" : "disconnect");
    if ( model_.path().startsWith( SDMMC_ROOT ) && !inserted )
    {
        qApp->exit();
    }
}

void ComicApplication::onMountTreeSignal(bool inserted, const QString &mount_point)
{
    qDebug("comic_reader: Mount point:%s %s", qPrintable( mount_point ),
            inserted ? "inserted" : "disconnect");
    if (!inserted && model_.path().startsWith(mount_point))
    {
        qApp->exit();
    }
}

void ComicApplication::onAboutToShutDown()
{
    qDebug("comic_reader: onAboutToShutDown");
    qApp->exit();
}

}   // namespce comic_reader
