

#include "onyx/touch/touch_listener.h"


TouchEventListener::TouchEventListener()
: socket_(this)
, enable_(true)
{
    QObject::connect(&socket_, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect();
}

TouchEventListener::~TouchEventListener()
{
    disconnect();
}

bool TouchEventListener::connect()
{
    socket_.connectToServer(TOUCH_SERVER_ADDRESS);
    return true;
}

bool TouchEventListener::disconnect()
{
    socket_.disconnectFromServer();
    return true;
}

void TouchEventListener::onReadyRead()
{
    int bytes = socket_.bytesAvailable();
    int time = bytes / sizeof(TouchData);
    for(int i = 0; i < time; ++i)
    {
        TouchData d;
        socket_.read(reinterpret_cast<char *>(&d), sizeof(d));
        if (isEnabled())
        {
            emit touchData(d);
        }
     }
}

bool TouchEventListener::addWatcherWidget(QObject *wnd)
{
    if (wnd)
    {
        wnd->installEventFilter(this);
        enableBroadcast(true);
    }
    return true;
}

bool TouchEventListener::removeWatcherWidget(QObject *wnd)
{
    if (wnd)
    {
        wnd->removeEventFilter(this);
        enableBroadcast(false);
    }
    return true;
}

bool TouchEventListener::eventFilter(QObject *obj,
                                     QEvent *event)
{
    if (event->type() == QEvent::FocusIn ||
        event->type() == QEvent::Show)
    {
        enableBroadcast(true);
    }
    else if (event->type() == QEvent::FocusOut ||
             event->type() == QEvent::Hide)
    {

        enableBroadcast(false);
    }
    return QObject::eventFilter(obj, event);
}
