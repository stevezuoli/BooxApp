#include "onyx/ui/languages.h"
#include "image_application.h"
#include "image_view.h"
#include "notes_view.h"

namespace image
{

void ImageApplication::onCreateView(int type, MainWindow* main_window, QWidget*& result)
{
    QWidget* view = 0;
    switch (type)
    {
    case IMAGE_VIEW:
        view = new ImageView(main_window);
        break;
    case NOTES_VIEW:
        view = new NotesView(main_window);
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

void ImageApplication::onAttachView(int type, QWidget* view, MainWindow* main_window)
{
    switch (type)
    {
    case IMAGE_VIEW:
        down_cast<ImageView*>(view)->attachMainWindow(main_window);
        break;
    case NOTES_VIEW:
        down_cast<NotesView*>(view)->attachMainWindow(main_window);
        break;
    case TOC_VIEW:
        break;
    case THUMBNAIL_VIEW:
        down_cast<ThumbnailView*>(view)->attachMainWindow(main_window);
        break;
    default:
        break;
    }
}

void ImageApplication::onDeattachView(int type, QWidget* view, MainWindow* main_window)
{
    switch (type)
    {
    case IMAGE_VIEW:
        down_cast<ImageView*>(view)->deattachMainWindow(main_window);
        break;
    case NOTES_VIEW:
        down_cast<NotesView*>(view)->deattachMainWindow(main_window);
        break;
    case TOC_VIEW:
        break;
    case THUMBNAIL_VIEW:
        down_cast<ThumbnailView*>(view)->deattachMainWindow(main_window);
        break;
    default:
        break;
    }
}

ImageApplication::ImageApplication(int argc, char **argv)
    : QApplication(argc, argv)
    , main_window_(this)
    , app_type_(IMAGE_APP)
{
    int args_num = QCoreApplication::arguments().size();
    if (args_num >= 1)
    {
        ui::loadTranslator(QLocale::system().name());
        main_window_.handleSetStatusBarFunctions(MENU | PROGRESS | MESSAGE| STYLUS);
        if (args_num == 1)
        {
            // no additional argument, open notes view
            app_type_ = NOTES_APP;
        }
        else if (args_num >= 2)
        {
            path_ = QString::fromLocal8Bit(argv[1]);
            if (args_num == 2)
            {
                app_type_ = IMAGE_APP;
            }
            else
            {
                QString type = QString::fromLocal8Bit(argv[2]);
                if (type.toLower() == "notes")
                {
                    app_type_ = NOTES_APP;
                }
            }
        }

        if (app_type_ == NOTES_APP)
        {
            main_window_.setContentMargins(10, 0, 10, 0);
        }

        if (open(path_))
        {
            main_window_.attachModel(&model_);
        }
    }
}

ImageApplication::~ImageApplication(void)
{
    close();
}

BaseView* ImageApplication::activateNotes()
{
    NotesView *notes_view = down_cast<NotesView*>(main_window_.activateView(NOTES_VIEW));
    assert(notes_view != 0);

    // connect the signals of view
    SysStatus & sys_status = SysStatus::instance();
    connect(&sys_status, SIGNAL(wakeup()), notes_view, SLOT(onWakeUp()));
    connect(notes_view, SIGNAL(rotateScreen()), this, SLOT(onRotateScreen()));

    // attach sketch proxy and notes manager
    shared_ptr<NotesDocumentManager> notes_mgr(new NotesDocumentManager());
    notes_view->setNotesManager(notes_mgr);

    shared_ptr<SketchProxy> sketch_proxy(new SketchProxy());
    notes_view->setSketchProxy(sketch_proxy);

    QString data_path = path_;
    if (data_path.isEmpty())
    {
        data_path = QCoreApplication::tr("note_1");
    }
    notes_mgr->load(data_path, sketch_proxy.get());
    return notes_view;
}

BaseView* ImageApplication::activateImage()
{
    ImageView *image_view = down_cast<ImageView*>(main_window_.activateView(IMAGE_VIEW));
    assert(image_view != 0);

    // connect the signals of view
    SysStatus & sys_status = SysStatus::instance();
    connect(&sys_status, SIGNAL(wakeup()), image_view, SLOT(onWakeUp()));
    connect(image_view, SIGNAL(rotateScreen()), this, SLOT(onRotateScreen()));
    return image_view;
}

bool ImageApplication::open(const QString & path)
{
    main_window_.show();
    BaseView *view = 0;
    QString data_path = path_;
    if (app_type_ == IMAGE_APP)
    {
        view = activateImage();
    }
    else if (app_type_ == NOTES_APP)
    {
        view = activateNotes();

        NotesView *notes_view = down_cast<NotesView*>(main_window_.getView(NOTES_VIEW));
        data_path = notes_view->notesMgr()->defaultBackground();
    }

    if (view == 0)
    {
        return false;
    }

    // connect the signals with sys_state_
    SysStatus & sys_status = SysStatus::instance();
    connect( &sys_status, SIGNAL( sdCardChangedSignal( bool ) ), this, SLOT( onSDCardChangedSignal( bool ) ) );
    connect( &sys_status, SIGNAL( aboutToShutdown() ), this, SLOT( onAboutToShutDown() ) );
    connect( &sys_status, SIGNAL( wakeup() ), this, SLOT( onWakeUp() ) );

#ifdef Q_WS_QWS
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif

    view->attachModel(&model_);
    if (model_.open(data_path, path))
    {
        return true;
    }

    if ( sys::SysStatus::instance().isSystemBusy() )
    {
        // if loading fails, set busy to be false
        sys::SysStatus::instance().setSystemBusy( false );
    }
    view->deattachModel();
    return false;
}

bool ImageApplication::close()
{
    model_.save();
    return model_.close();
}

bool ImageApplication::suspend()
{
    // save all of the options for waking up
    return model_.save();
}

void ImageApplication::onWakeUp()
{
    model_.save();
}

void ImageApplication::onRotateScreen()
{
    SysStatus::instance().rotateScreen();
}

void ImageApplication::onScreenSizeChanged(int)
{
    onyx::screen::instance().enableUpdate(false);
    main_window_.resize(qApp->desktop()->screenGeometry().size());
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(&main_window_, onyx::screen::ScreenProxy::GC);
}

void ImageApplication::onUSBSignal(bool inserted)
{
    qDebug("USB %s", inserted ? "inserted" : "disconnect");
    if (model_.initPath().startsWith( USB_ROOT ) && !inserted)
    {
        qApp->exit();
    }
}

void ImageApplication::onAboutToShutDown()
{
    qDebug("System is about to shut down");
    model_.close();
    qApp->exit();
}

void ImageApplication::onMountTreeSignal(bool inserted, const QString &mount_point)
{
    qDebug( "Mount point:%s %s",
            qPrintable( mount_point ),
            inserted ? "inserted" : "disconnect" );
    if (!inserted && model_.initPath().startsWith( mount_point ))
    {
        qApp->exit();
    }
}

void ImageApplication::onSDCardChangedSignal(bool inserted)
{
    qDebug("SD %s", inserted ? "inserted" : "disconnect");
    if (model_.initPath().startsWith( SDMMC_ROOT ) && !inserted)
    {
        qApp->exit();
    }
}

void ImageApplication::onConnectToPCSignal(bool connected)
{
    qDebug("Connection to PC%s", connected ? "connected" : "disconnected");
    if ( connected )
    {
        qApp->exit();
    }
}

void ImageApplication::onBatterySignal(const int, const int, bool)
{
    qDebug("Battery");
    // TODO. Implement Me
}

void ImageApplication::onSystemIdleSignal()
{
    qDebug("System Idle");
    // TODO. Implement Me
}


};
