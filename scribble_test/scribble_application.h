#ifndef SCRIBBLE_APPLICATION_H_
#define SCRIBBLE_APPLICATION_H_

#include "scribble_frame.h"

class ScribbleApplication : public QApplication
{
    Q_OBJECT
public:
    ScribbleApplication(int &argc, char **argv);
    ~ScribbleApplication(void);

    const QString & currentPath() { return current_path_; }

public Q_SLOTS:
    bool open(const QString &path_name);
    bool close(const QString &path_name);

    void onWakeUp();
    void onUSBSignal(bool inserted);
    void onSDChangedSignal(bool inserted);
    void onMountTreeSignal(bool inserted, const QString &mount_point);
    void onConnectToPCSignal(bool connected);
    void onBatterySignal(const int, const int, bool);
    void onSystemIdleSignal();
    void onAboutToShutDown();

    void onRotateScreen();
    void onScreenSizeChanged(int);

private:
    scoped_ptr<ScribbleWidget>  view_;
    QString                     current_path_;     // path of the current document

    NO_COPY_AND_ASSIGN(ScribbleApplication);
};

#endif
