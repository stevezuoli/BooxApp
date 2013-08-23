#include "onyx/ui/languages.h"
#include "naboo_application.h"
#include "naboo_view.h"

namespace naboo_reader
{

NabooApplication::NabooApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , main_window_(this)
{
    if (argc > 1)
    {
        ui::loadTranslator(QLocale::system().name());
        main_window_.handleSetStatusBarFunctions(MENU | PROGRESS | CLOCK | MESSAGE | BATTERY);
        current_path_ = QString::fromLocal8Bit(argv[1]);
    }
}

NabooApplication::~NabooApplication(void)
{
    close( current_path_ );
}

bool NabooApplication::open( const QString &path_name )
{
    main_window_.attachModel(&model_);
    main_window_.show();
    NabooView *view = down_cast<NabooView*>(main_window_.activateView(NABOO_VIEW));

    // connect the signals with view
    connect( view, SIGNAL(rotateScreen()), this, SLOT(onRotateScreen()) );
    connect( view, SIGNAL(testSuspend()), this, SLOT(suspend()) );
    connect( view, SIGNAL(testWakeUp()), this, SLOT(onWakeUp()) );

    // connect the signals with sys_state_
    SysStatus & sys_status = SysStatus::instance();
    connect( &sys_status, SIGNAL( mountTreeSignal( bool, const QString & ) ),
             this, SLOT( onMountTreeSignal( bool, const QString &) ) );
    connect( &sys_status, SIGNAL( sdCardChangedSignal( bool ) ), this, SLOT( onSDChangedSignal( bool ) ) );
    connect( &sys_status, SIGNAL( aboutToShutdown() ), this, SLOT( onAboutToShutDown() ) );
    connect( &sys_status, SIGNAL( wakeup() ), this, SLOT( onWakeUp() ) );

#ifdef Q_WS_QWS
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif

    view->attachModel(&model_);
    bool ret = model_.open(path_name);
    if ( !ret )
    {
        if ( sys::SysStatus::instance().isSystemBusy() )
        {
            // if loading fails, set busy to be false
            sys::SysStatus::instance().setSystemBusy( false );
        }
        view->deattachModel();
    }
    return ret;
}

bool NabooApplication::isOpened()
{
    return model_.isReady();
}

bool NabooApplication::errorFound()
{
    return model_.errorFound();
}

bool NabooApplication::close(const QString &path_name)
{
    model_.save();
    model_.close();
    main_window_.clearViews();
    return true;
}

bool NabooApplication::suspend()
{
    // save all of the options for waking up
    return model_.save();
}

void NabooApplication::onWakeUp()
{
    // save the configurations before closing
    model_.save();

    if (!sys::is166E())
    {
        // set the system to be busy at this moment
        sys::SysStatus::instance().setSystemBusy(true, false);

        // close the document because it might be already invalid
        model_.close();

        // open the document again
        model_.open( current_path_ );
        sys::SysStatus::instance().setSystemBusy(false, false);
    }
}

void NabooApplication::onAboutToShutDown()
{
    qDebug("System is about to shut down");
    qApp->exit();
}

void NabooApplication::onUSBSignal(bool inserted)
{
    qDebug("USB %s", inserted ? "inserted" : "disconnect");
    if ( model_.path().startsWith( USB_ROOT ) && !inserted )
    {
        qApp->exit();
    }
}

void NabooApplication::onMountTreeSignal(bool inserted, const QString &mount_point)
{
    qDebug( "Mount point:%s %s",
            qPrintable( mount_point ),
            inserted ? "inserted" : "disconnect" );
    if ( !inserted && model_.path().startsWith( mount_point ) )
    {
        qApp->exit();
    }
}

void NabooApplication::onSDChangedSignal(bool inserted)
{
    qDebug("SD %s", inserted ? "inserted" : "disconnect");
    if ( model_.path().startsWith( SDMMC_ROOT ) && !inserted )
    {
        qApp->exit();
    }
}

void NabooApplication::onBatterySignal(const int, const int, bool)
{
    qDebug("Battery");
    // TODO. Implement Me
}

void NabooApplication::onSystemIdleSignal()
{
    qDebug("System Idle");
    // TODO. Implement Me
}

void NabooApplication::onRotateScreen()
{
    SysStatus::instance().rotateScreen();
}

void NabooApplication::onScreenSizeChanged(int)
{
    onyx::screen::instance().enableUpdate(false);
    main_window_.resize(qApp->desktop()->screenGeometry().size());
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(&main_window_, onyx::screen::ScreenProxy::GC);
}

void NabooApplication::onCreateView(int type, MainWindow* main_window, QWidget*& result)
{
    QWidget* view = 0;
    switch (type)
    {
    case NABOO_VIEW:
        view = new NabooView(main_window);
        break;
    case TOC_VIEW:
        view = new TreeViewDialog(main_window);
        break;
    case THUMBNAIL_VIEW:
        view = new ThumbnailView(main_window);
        break;
    default:
        break;
    }
    result = view;
}

void NabooApplication::onAttachView(int type, QWidget* view, MainWindow* main_window)
{
    switch (type)
    {
    case NABOO_VIEW:
        {
            down_cast<NabooView*>(view)->attachMainWindow(main_window);
        }
        break;
    case TOC_VIEW:
        {
#ifdef MAIN_WINDOW_TOC_ON
            QWidget* reading_view = main_window->getView(NABOO_VIEW);
            if (reading_view == 0)
            {
                return;
            }
            down_cast<TreeViewDialog*>(view)->attachMainWindow(main_window);
            down_cast<NabooView*>(reading_view)->attachTreeView(down_cast<TreeViewDialog*>(view));
#endif
        }
        break;
    case THUMBNAIL_VIEW:
        down_cast<ThumbnailView*>(view)->attachMainWindow(main_window);
        break;
    default:
        break;
    }
}

void NabooApplication::onDeattachView(int type, QWidget* view, MainWindow* main_window)
{
    switch (type)
    {
    case NABOO_VIEW:
        down_cast<NabooView*>(view)->deattachMainWindow(main_window);
        break;
    case TOC_VIEW:
        {
#ifdef MAIN_WINDOW_TOC_ON
            QWidget* reading_view = main_window->getView(NABOO_VIEW);
            if (reading_view == 0)
            {
                return;
            }
            down_cast<TreeViewDialog*>(view)->deattachMainWindow(main_window);
            down_cast<NabooView*>(reading_view)->deattachTreeView(down_cast<TreeViewDialog*>(view));
#endif
        }
        break;
    case THUMBNAIL_VIEW:
        down_cast<ThumbnailView*>(view)->deattachMainWindow(main_window);
        break;
    default:
        break;
    }
}

bool NabooApplication::flip(int direction)
{
    QWidget* reading_view = main_window_.getView(NABOO_VIEW);
    if (reading_view == 0)
    {
        return false;
    }
    return down_cast<NabooView*>(reading_view)->flip(direction);
}

bool NabooApplicationAdaptor::flip(int direction)
{
    return app_->flip(direction);
}

}

