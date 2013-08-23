
#ifndef SCREEN_MANAGER_INTERFACE_H_
#define SCREEN_MANAGER_INTERFACE_H_

#include <QColor>
#include <QtNetwork/QtNetwork>

#include "onyx/base/base.h"
#include "onyx/screen/screen_proxy.h"

class ScreenManagerInterface : public QObject
{
    Q_OBJECT
public:
    ScreenManagerInterface() {}
    ~ScreenManagerInterface() {}

public:
    virtual void enableUpdate(bool enable) = 0;
    virtual bool isUpdateEnabled() = 0;

    virtual bool setGrayScale(int colors) = 0;
    virtual int grayScale() = 0;

    virtual bool isBusy() = 0;
    virtual void setBusy(bool busy = true, bool show_indicator = true) = 0;

    virtual void refreshScreen(onyx::screen::ScreenProxy::Waveform waveform) = 0;
    virtual void drawImage(const QImage &image) = 0;
    virtual void fadeScreen() = 0;
    virtual void showUSBScreen() = 0;
    virtual void showCurrentDeepSleepScreen() = 0;
    virtual void showDeepSleepScreen() = 0;

    virtual void blit(const QRect & rc, const unsigned char *src,
                    onyx::screen::ScreenProxy::Waveform waveform =
                    onyx::screen::ScreenProxy::GU) = 0;
    virtual void fillScreen(unsigned char color) = 0;
    virtual bool sleep() = 0;
    virtual void shutdown() = 0;

    virtual void reset() = 0;

    virtual bool dbgStateTest() = 0;

    virtual void snapshot(const QString &path) = 0;
    virtual void drawLine(onyx::screen::ScreenCommand & command,  int index ) = 0;
};

#endif      // SCREEN_MANAGER_INTERFACE_H_
