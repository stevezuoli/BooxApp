#ifndef ONYX_TOUCH_EVENT_LISTENER_H_
#define ONYX_TOUCH_EVENT_LISTENER_H_

#include <QObject>
#include <QLocalSocket>
#include "touch_data.h"


class TouchEventListener : public QObject
{
    Q_OBJECT

public:
    TouchEventListener();
    ~TouchEventListener();

public:
    bool addWatcherWidget(QObject *wnd);
    bool removeWatcherWidget(QObject *wnd);

public Q_SLOTS:
    void onReadyRead();

Q_SIGNALS:
    void touchData(TouchData & data);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    bool connect();
    bool disconnect();

    void enableBroadcast(bool e = true) { enable_ = e; }
    bool isEnabled() { return enable_; }

private:
    QLocalSocket socket_;
    bool enable_;
};


#endif
