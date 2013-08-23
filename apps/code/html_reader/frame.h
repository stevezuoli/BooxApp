
#ifndef READER_FRAME_H_
#define READER_FRAME_H_

#include <QtGui/QtGui>
#include "view.h"
#include "onyx/ui/ui.h"
#include "onyx/sys/sys.h"

using namespace ui;

namespace reader
{

class ReaderFrame : public QWidget
{
    Q_OBJECT

public:
    ReaderFrame(QWidget *parent = 0);
    ~ReaderFrame();

public Q_SLOTS:
    bool open(const QString &path);
    void onAboutToSuspend();

protected:
    bool event(QEvent *e);

private Q_SLOTS:
    void onRotateScreen();
    void onScreenSizeChanged(int);
    void onSdCardChanged(bool insert);
    void handleMountTreeEvent(bool inserted, const QString &mount_point);
    void onWakeup();
    void onAboutToShutdown();
    void onProgressChanged(const int, const int);
    void onRangeChanged(const int, const int, const int);
    void onProgressClicked(const int percent, const int value);
    void onClockClicked();

private:
    void createLayout();

private:
    QVBoxLayout layout_;
    ReaderView view_;
    sys::SysStatus & sys_status_;
    StatusBar status_bar_;
    QString document_;
    bool need_gc_in_loading_;
};

}   // namespace reader

#endif  // READER_FRAME_H_
