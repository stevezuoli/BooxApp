// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef BROADSHEET_SCREEN_MANAGER_H_
#define BROADSHEET_SCREEN_MANAGER_H_

#include <QColor>
#include <QtNetwork/QtNetwork>

#include "onyx/base/base.h"
#include "onyx/screen/screen_proxy.h"
#include "screen_manager_interface.h"

/// Broad sheet screen manager.
class BSScreenManager : public ScreenManagerInterface
{
    Q_OBJECT
public:
    BSScreenManager();
    ~BSScreenManager();

public:
    void enableUpdate(bool enable);
    bool isUpdateEnabled() { return enable_; }

    bool setGrayScale(int colors);
    int grayScale();

    bool isBusy() { return busy_; }
    void setBusy(bool busy = true, bool show_indicator = true);

    void refreshScreen(onyx::screen::ScreenProxy::Waveform waveform);
    void drawImage(const QImage &image);
    void fadeScreen();
    void showUSBScreen();
    void showCurrentDeepSleepScreen();
    void showDeepSleepScreen();

    void blit(const QRect & rc, const unsigned char *src, onyx::screen::ScreenProxy::Waveform waveform = onyx::screen::ScreenProxy::GU);
    void fillScreen(unsigned char color);
    bool sleep();
    void shutdown();

    void reset();

    bool dbgStateTest();

    void snapshot(const QString &path);
    void drawLine(onyx::screen::ScreenCommand & command, int index );
    void drawLines(onyx::screen::ScreenCommand & command);

private Q_SLOTS:
    void onReadyRead();
    void onBusyTimeout();

private:
    void start();
    void stop();

    bool ensureRunning();

    void ensureUpdateFinished();
    void sync(onyx::screen::ScreenCommand & command);
    void updateWidget(onyx::screen::ScreenCommand & command);
    void updateWidgetRegion(onyx::screen::ScreenCommand & command);
    void updateScreen(onyx::screen::ScreenCommand & command);

    void fillScreen(onyx::screen::ScreenCommand & command);

    QVector<QRgb> & colorTable();
    QImage imageFromScreen();
    QImage busyImage(int index = 0);

    QDir screenSaverDir();
    QImage nextScreenSaverImage();
    QImage currentScreenSaverImage();
    int screenSaverCount();

private:
    bool busy_;
    int busy_index_;
    scoped_ptr<QImage> busy_canvas_;

    int screen_saver_index_;

    bool sleeping_;
    bool enable_;

    uchar *data_;       ///< The base address.

    int screen_width_;
    int screen_height_;
    QUdpSocket socket_;
    QTimer busy_timer_;
};

#endif
