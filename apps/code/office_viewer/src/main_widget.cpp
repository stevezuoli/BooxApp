#include "assert.h"
#include <QtCore/QLocale>
#include <QtCore/QDir>
#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QToolBar>
#include <QtGui/QFileDialog>
#include <QtGui/QVBoxLayout>
#include <QtCore/QPoint>


#include <onyx/ui/base_thumbnail.h>
#include <onyx/sys/sys.h>
#include "onyx/base/base.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/ui.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/ui/message_dialog.h"
#include "onyx/ui/status_bar.h"

#ifdef BUILD_FOR_ARM
#include <QtGui/qwsdisplay_qws.h>
#include <QtGui/qscreen_qws.h>
#endif


#include "onyx_office.h"
#include "main_widget.h"
namespace onyx {

#ifndef BUILD_FOR_ARM
    static const QSize my_client_size(600, 800);
#endif

MainWidget::MainWidget(QWidget *parent)
#ifndef Q_WS_QWS
: QWidget(0, 0)
#else
: QWidget(0, Qt::FramelessWindowHint)
#endif
, layout_(this)
, view_(OfficeReader::instance(), this)
, status_bar_(0,  MENU | PROGRESS | CONNECTION | BATTERY | MESSAGE | CLOCK | SCREEN_REFRESH)
{
#ifndef BUILD_FOR_ARM
    setMaximumSize( QSize(700, 900));
#endif
    createLayout();
    // setup connections
    connect(&view_, SIGNAL(pageChanged(int, int)), &status_bar_, SLOT(setProgress(const int, const int)));
    connect(&status_bar_, SIGNAL(menuClicked()), &view_, SLOT(showContextMenu()));
    connect(&status_bar_,  SIGNAL(progressClicked(const int, const int)),   this, SLOT(onPagebarClicked(const int, const int)));
    connect(&view_, SIGNAL(docClosed()), qApp, SLOT(quit()));
    connect(&view_, SIGNAL(requestGotoPageDialog()), &status_bar_, SLOT(onMessageAreaClicked()));
    connect(&view_, SIGNAL(requestClockDialog()), &status_bar_, SLOT(onClockClicked()));
    connect(&view_, SIGNAL(fullScreen(bool)), this, SLOT(onFullScreen(bool)));
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);

    //initialize
    OfficeReader::instance().initialize(clientSize());
    sys::SystemConfig conf;
    onyx::screen::instance().setGCInterval(conf.screenUpdateGCInterval());
}

void MainWidget::createLayout()
{
    setContentsMargins(0, 0, 0, 0);
    layout_.setSpacing(0);
    layout_.setContentsMargins(0, 0, 0, 0);
    layout_.addWidget(&view_);
    layout_.addWidget(&status_bar_);
}

void MainWidget::onScreenSizeChanged(int) {
    onyx::screen::instance().enableUpdate(false);
    setFixedSize(qApp->desktop()->screenGeometry().size());
    view_.resizeBackend(clientSize());
    onyx::screen::instance().enableUpdate(true);
    update();
}

bool MainWidget::open(const QString& path) {
    return view_.open(path);
}

StatusBar & MainWidget::statusBar()
{
    return status_bar_;
}

bool MainWidget::event(QEvent* event)
{
    bool ret = QWidget::event(event);
    if (event->type() == QEvent::UpdateRequest)
    {
        updateScreen();
        qDebug("update screen done.");
    }
    return ret;
}

void MainWidget::updateScreen()
{
    if (onyx::screen::instance().userData() < 2)
    {
        if (1 == onyx::screen::instance().userData())
        {
            ++onyx::screen::instance().userData();
        }
        onyx::screen::instance().updateWidget(this);
    }
    else if (2 == onyx::screen::instance().userData())
    {
        onyx::screen::instance().updateWidgetWithGCInterval(
            this,
            NULL,
            onyx::screen::ScreenProxy::INVALID,
            true,
            onyx::screen::ScreenCommand::WAIT_ALL);
    }
}

void MainWidget::onPagebarClicked(const int percentage, const int value)
{
    OfficeReader::instance().doAction(GOTO_PAGE, value);
}

QSize MainWidget::clientSize()
{
#ifndef BUILD_FOR_ARM
    return my_client_size;
#endif
    QSize screen_size = qApp->desktop()->screenGeometry().size();
    if (status_bar_.isVisible())
    {
        return screen_size - QSize(0, status_bar_.frameSize().height()) ;
    }
    else
    {
        return screen_size;
    }
}

// Cannot reuse onScreensize changed directly, have to make a copy.
// The view will be updated as status bar changed. so don't invoke update again.
void MainWidget::onFullScreen(bool full)
{
    status_bar_.setVisible(!full);
    onyx::screen::instance().enableUpdate(false);
    view_.resizeBackend(clientSize());
    onyx::screen::instance().enableUpdate(true);
    update();
}

}
