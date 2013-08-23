
#include <QtGui/QtGui>
#ifdef BUILD_FOR_ARM
#include <QtGui/qscreen_qws.h>
#endif

#include "onyx/screen/screen_update_watcher.h"

namespace onyx
{

namespace screen
{

ScreenUpdateWatcher & watcher()
{
    return ScreenUpdateWatcher::instance();
}


ScreenUpdateWatcher::ScreenUpdateWatcher()
: dw_enqueue_(false)
{
}

ScreenUpdateWatcher::ScreenUpdateWatcher(ScreenUpdateWatcher &ref)
: dw_enqueue_(false)
{
}

ScreenUpdateWatcher::~ScreenUpdateWatcher()
{
}

/// Added widget to watcher list.
/// \widget the widget wants to listen. When it's in watcher list, this class will
/// automatically refresh screen when paint event is processed.
/// \gc_interval. When update screen, after every gc_interval gu update, a gc update
/// will be used. If gc_interval is zero, gu is always used. If gc_interval is one
/// gc is always used.
void ScreenUpdateWatcher::addWatcher(QWidget *widget, int gc_interval)
{
    if (widget)
    {
        if (gc_interval >= 0)
        {
            UpdateCount item(gc_interval, gc_interval);
            widget_map_[widget] = item;
        }
        widget->installEventFilter(this);
    }
}

void ScreenUpdateWatcher::removeWatcher(QWidget *widget)
{
    if (widget)
    {
        widget->removeEventFilter(this);
        if (widget_map_.contains(widget))
        {
            widget_map_.remove(widget);
        }
    }
}

/// Added dw screen update to screen update queue.
void ScreenUpdateWatcher::dwEnqueueStart(QWidget *widget, const QRect & rc)
{
    useDwEnqueue(true);
    enqueue(widget, rc, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
}

void ScreenUpdateWatcher::dwEnqueueEnd(QWidget *widget, const QRect & rc)
{
    useDwEnqueue(false);
    enqueue(widget, rc, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
}

bool ScreenUpdateWatcher::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest)
    {
        if (dwEnqueue())
        {
            QTimer::singleShot(0, this, SLOT(updateScreen()));
            return QObject::eventFilter(obj, event);
        }

        QWidget * wnd = static_cast<QWidget *>(obj);
        if (widget_map_.contains(wnd))
        {
            UpdateCount & item = widget_map_[wnd];
            if (++item.current_ >= item.max_ && item.max_ > 0)
            {
                item.current_ = 0;
                QTimer::singleShot(0, this, SLOT(gcUpdateScreen()));
            }
            else
            {
                QTimer::singleShot(0, this, SLOT(guUpdateScreen()));
            }
        }
        else
        {
            QTimer::singleShot(0, this, SLOT(updateScreen()));
        }
    }
    else if (event->type() == QEvent::WindowActivate)
    {
        if (obj->isWidgetType())
        {
            QWidget * wnd = static_cast<QWidget *>(obj);
            wnd->update();
            enqueue(wnd, onyx::screen::ScreenProxy::GC);
        }
    }
    return QObject::eventFilter(obj, event);
}

bool ScreenUpdateWatcher::enqueue(UpdateItem & item,
                                  QWidget *widget,
                                  onyx::screen::ScreenProxy::Waveform w,
                                  onyx::screen::ScreenCommand::WaitMode wait,
                                  const QRect & rc)
{
    if (widget == 0)
    {
        widget = qApp->desktop();
    }

    if (!widget->isVisible())
    {
        return false;
    }

    QPoint pt = widget->mapToGlobal(rc.topLeft());
    QSize s;
    if (!rc.size().isEmpty())
    {
        s = rc.size();
    }
    else
    {
        s = widget->size();

        // Consider rotation
        if (widget == qApp->desktop())
        {
            s =  qApp->desktop()->screenGeometry().size();
        }
    }
    item.rc = QRect(pt, s);
    item.wait = wait;
    item.waveform = w;
    //qDebug() << "add new request " << item.rc;
    return true;
}

/// Add screen update request to queue.
/// \widget The widget to update.
/// \w Which kind of waveform to use to update screen.
void ScreenUpdateWatcher::enqueue(QWidget *widget,
                                  onyx::screen::ScreenProxy::Waveform w,
                                  onyx::screen::ScreenCommand::WaitMode wm)
{
    UpdateItem item;
    if (enqueue(item, widget, w, wm))
    {
        queue_.enqueue(item);
    }
}

/// Add screen update request to queue.
/// \widget The widget to update.
/// \rc The rectangle of widget needs to update. If it's empty, whole widget will be updated.
/// \w Which kind of waveform to use to update screen.
void ScreenUpdateWatcher::enqueue(QWidget *widget,
                                  const QRect & rc,
                                  onyx::screen::ScreenProxy::Waveform w,
                                  onyx::screen::ScreenCommand::WaitMode wait)
{
    UpdateItem item;
    if (enqueue(item, widget, w, wait, rc))
    {
        queue_.enqueue(item);
    }
}

void ScreenUpdateWatcher::flush(QWidget *widget,
                                onyx::screen::ScreenProxy::Waveform w,
                                onyx::screen::ScreenCommand::WaitMode wait)
{
    if (widget)
    {
        widget->update();
        enqueue(widget, w, wait);
        QApplication::processEvents();
    }
}

/// Get item from queue and decide which waveform to use.
void ScreenUpdateWatcher::updateScreen()
{
    updateScreenInternal(true);
}

void ScreenUpdateWatcher::guUpdateScreen()
{
    updateScreenInternal(false, onyx::screen::ScreenProxy::GU);
}

void ScreenUpdateWatcher::gcUpdateScreen()
{
    updateScreenInternal(false, onyx::screen::ScreenProxy::GC);
}

void ScreenUpdateWatcher::enableUpdate(bool enable)
{
    onyx::screen::instance().enableUpdate(enable);
}

void ScreenUpdateWatcher::updateScreenInternal(bool automatic,
                                               onyx::screen::ScreenProxy::Waveform waveform)
{
    onyx::screen::ScreenProxy::Waveform w = onyx::screen::ScreenProxy::DW;
    onyx::screen::ScreenCommand::WaitMode wait = onyx::screen::ScreenCommand::WAIT_NONE;
    QRect rc;
    while (!queue_.isEmpty())
    {
        UpdateItem i = queue_.dequeue();
        rc = rc.united(i.rc);
        if (automatic)
        {
            if (i.waveform > w)
            {
                w = i.waveform;
            }
        }
        else
        {
            w = waveform;
        }

        if (i.wait > wait)
        {
            wait = i.wait;
        }
    }
    if (!rc.isEmpty())
    {
        onyx::screen::instance().updateWidgetRegion(0, rc, w, false, wait);
    }
}

bool ScreenUpdateWatcher::isQueueEmpty()
{
    return queue_.isEmpty();
}

};

};
