
#include <QtGui/QtGui>
#include "frame.h"
#include "onyx/base/device.h"
#include "onyx/screen/screen_proxy.h"

using namespace ui;

namespace reader
{

ReaderFrame::ReaderFrame(QWidget *parent)
    : QWidget(parent)
    , layout_(this)
    , view_(0)
    , sys_status_(SysStatus::instance())
    , status_bar_(0, MENU|PROGRESS|MESSAGE|CLOCK|BATTERY)
    , need_gc_in_loading_(true)
{
#ifdef Q_WS_QWS
    setWindowFlags(Qt::FramelessWindowHint);
#endif
    createLayout();

    // Setup connections.
    connect(&view_, SIGNAL(progressChangedSignal(const int, const int)),
            this, SLOT(onProgressChanged(const int, const int)));
    connect(&view_, SIGNAL(rotateScreen()), this, SLOT(onRotateScreen()));
    connect(&view_, SIGNAL(viewportRangeChangedSignal(const int, const int, const int)),
            this, SLOT(onRangeChanged(const int, const int, const int)));
    connect(&view_, SIGNAL(clockClicked()), this, SLOT(onClockClicked()));

    connect(&status_bar_, SIGNAL(menuClicked()), &view_, SLOT(popupMenu()));
    connect(&status_bar_, SIGNAL(progressClicked(const int, const int)),
            this, SLOT(onProgressClicked(const int, const int)));

#ifdef Q_WS_QWS
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif
    connect(&sys_status_, SIGNAL(sdCardChangedSignal(bool)), this, SLOT(onSdCardChanged(bool)));
    connect(&sys_status_, SIGNAL(mountTreeSignal(bool, const QString &)),
            this, SLOT(handleMountTreeEvent(bool, const QString &)));

    connect(&sys_status_, SIGNAL(aboutToSuspend()), this, SLOT(onAboutToSuspend()));
    connect(&sys_status_, SIGNAL(wakeup()), this, SLOT(onWakeup()));
    connect(&sys_status_, SIGNAL(aboutToShutdown()), this, SLOT(onAboutToShutdown()));
}

ReaderFrame::~ReaderFrame()
{
}

bool ReaderFrame::open(const QString &path)
{
    document_ = path;
    return view_.open(path);
}

bool ReaderFrame::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip ||
        e->type() == QEvent::HoverMove ||
        e->type() == QEvent::HoverEnter ||
        e->type() == QEvent::HoverLeave)
    {
        e->accept();
        return true;
    }

    bool ret = QWidget::event(e);
    if (e->type() == QEvent::UpdateRequest)
    {
        if (sys::SysStatus::instance().isSystemBusy())
        {
            sys::SysStatus::instance().setSystemBusy(false);
        }
        // Check region.
        QRect rc = view_.updateRect();
        rc = rc.intersected(view_.rect());

        view_.resetUpdateRect();
        int area = rc.width() * rc.height();
        if (area > 0 && onyx::screen::instance().isUpdateEnabled())
        {
            qDebug("update rectange:(%d, %d, %d, %d)", rc.left(), rc.top(), rc.width(), rc.height());
            int max = view_.rect().width() * view_.rect().height();
            /*if (area < (max >> 3))
            {
                qDebug("Ignored the request");
                return true;
            }
            else*/
            if (rc.width() < (view_.width() - (view_.width() >> 2)) ||
                rc.height() < (view_.height() - (view_.height() >> 2)))
            {
                qDebug("using gu waveform");
                onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
                return true;
            }

            if (area > (max - (max >> 2)))
            {
                if (!view_.isLoadingFinished() && need_gc_in_loading_)
                {
                    qDebug("using gc waveform");
                    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
                    need_gc_in_loading_ = false;
                    return true;
                }
            }
        }

        qDebug("using default waveform");
        onyx::screen::instance().updateWidget(this);
        return true;
    }
    return ret;
}

void ReaderFrame::createLayout()
{
    layout_.setContentsMargins(0, 0, 0, 0);
    layout_.setSpacing(1);

    layout_.addWidget(&view_);
    layout_.addWidget(&status_bar_);
}


void ReaderFrame::onRotateScreen()
{
    sys_status_.rotateScreen();
}

/// Desktop size changed slot. When screen is rotated, the frame can not receive
/// any resize event. Have to use the signal of desktop widget to change that.
/// John: I guess a better way is to make it top level widget. TODO
void ReaderFrame::onScreenSizeChanged(int)
{
    onyx::screen::instance().enableUpdate(false);
    resize(qApp->desktop()->screenGeometry().size());
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC);
}

void ReaderFrame::onSdCardChanged(bool inserted)
{
    if (!inserted && view_.doc_path().startsWith(SDMMC_ROOT))
    {
        view_.returnToLibrary();
    }
}

void ReaderFrame::onClockClicked()
{
    status_bar_.onClockClicked();
}

/// Handle mount tree event.
void ReaderFrame::handleMountTreeEvent(bool inserted, const QString &mount_point)
{
    qDebug("got mount tree event inserted %d path %s", inserted, qPrintable(mount_point));
    qDebug("doc path %s", qPrintable(view_.doc_path()));

    if (!inserted && view_.doc_path().startsWith(mount_point))
    {
        view_.returnToLibrary();
    }
}

void ReaderFrame::onAboutToSuspend()
{
    // Store options at first.
    view_.saveOptions();
    document_ = view_.doc_path();

    // If the document is opened from sd card, we'd better to close the document.
}

void ReaderFrame::onWakeup()
{
    // It could happen when user removed SD card.
    // To fix the SD card re-mount issue, we should try it several times.
    for(int i = 0; i < 2; ++i)
    {
        if (view_.open(document_))
        {
            return;
        }

        // Wait for a little bit.
        QTime t;
        t.start();
        while (t.elapsed() <= 1000)
        {
            QApplication::processEvents();
        }
    }
    view_.returnToLibrary();
}

void ReaderFrame::onAboutToShutdown()
{
    view_.returnToLibrary();
}

void ReaderFrame::onProgressClicked(const int percent,
                                     const int value)
{
    if (view_.isVisible())
    {
        view_.onRangeClicked(percent, value);
    }
}

void ReaderFrame::onProgressChanged(const int current,
                                    const int total)
{
    if (!view_.isVisible())
    {
        return;
    }

    if (current == 0 || view_.isLoadingFinished())
    {
        need_gc_in_loading_ = true;
    }

    if (current >= 0)
    {
        QString message(tr("Loading... %1%"));
        message = message.arg(current);
        status_bar_.showItem(ui::PROGRESS, false);
        status_bar_.setMessage(message);
    }
    else
    {
        QString message(tr("Failed to load page."));
        status_bar_.showItem(ui::PROGRESS, false);
        status_bar_.setMessage(message);
    }
}

void ReaderFrame::onRangeChanged(const int current,
                                 const int step,
                                 const int total)
{
    if (!view_.isVisible())
    {
        return;
    }

    status_bar_.showItem(ui::PROGRESS);
    status_bar_.setProgress((current + step) * 100 / total, 100, false);
    QString message(tr("%1%"));
    message = message.arg((current + step) * 100 / total);
    status_bar_.setMessage(message);
}

}
