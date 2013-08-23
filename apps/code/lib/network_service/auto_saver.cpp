#include "auto_saver.h"

namespace network_service
{

#define AUTOSAVE_IN  1000 * 3  // seconds
#define MAXWAIT      1000 * 15 // seconds

AutoSaver::AutoSaver(QObject *parent)
: QObject(parent)
{
    Q_ASSERT(parent);
}

AutoSaver::~AutoSaver()
{
    if (timer_.isActive())
    {
        qWarning() << "AutoSaver: still active when destroyed, changes not saved.";
    }
}

void AutoSaver::changeOccurred()
{
    if (first_change_.isNull())
    {
        first_change_.start();
    }

    if (first_change_.elapsed() > MAXWAIT)
    {
        saveIfNeccessary();
    }
    else
    {
        timer_.start(AUTOSAVE_IN, this);
    }
}

void AutoSaver::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timer_.timerId())
    {
        saveIfNeccessary();
    }
    else
    {
        QObject::timerEvent(event);
    }
}

void AutoSaver::saveIfNeccessary()
{
    if (!timer_.isActive())
    {
        return;
    }
    timer_.stop();
    first_change_ = QTime();
    if (!QMetaObject::invokeMethod(parent(), "save", Qt::DirectConnection))
    {
        qWarning() << "AutoSaver: error invoking slot save() on parent";
    }
}

}
