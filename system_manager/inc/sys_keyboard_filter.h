#ifndef SYS_KEYBOARD_FILTER_H_
#define SYS_KEYBOARD_FILTER_H_

#include "onyx/base/base.h"
#include <QtCore/QtCore>
#ifdef BUILD_FOR_ARM
#include <qwindowsystem_qws.h>
#endif

class SysKeyboardFilter : public QObject
#ifdef BUILD_FOR_ARM
                        , public QWSServer::KeyboardFilter
#endif
{
    Q_OBJECT
public:
    SysKeyboardFilter();
    ~SysKeyboardFilter();

public:
    bool isStylusInserted();
    bool isWifiOn();
    bool filter(int unicode, int keycode, int modifiers,  bool isPress, bool autoRepeat);
    void enable(bool e) { enabled_ = e; }
    bool isEnabled() { return enabled_; }

    void enableStandby(bool e) { enable_standby_ = e; }
    bool isStandbyEnabled() { return enable_standby_; }

    bool isPressed() { return is_pressed_; }
    void ignoreKeyboardEvent(int count);

Q_SIGNALS:
    void shutdownPressed();
    void standbyPressed();
    void usbCableChanged(bool inserted);
    void wifiKeyChanged(bool on);
    void stylusChanged(bool inserted);
    void stylusDistanceChanged(bool in);
    void printScreenSignal();
    void volumeUpPressed();
    void volumeDownPressed();
    void onlineServicePressed();
    void forceQuitPressed();
    void powerSwitchChanged(bool on);

private:
    void onUSBEvent(bool isPress);
    void onStylusEvent(bool isPress);
    void onPowerSwitchEvent(bool isPress);

private Q_SLOTS:
    void onTimeout();

private:
    bool is_pressed_;
    int previous_key_;
    bool enabled_;
    bool enable_standby_;
    bool usb_connected_;
    bool stylus_inserted_;
    bool power_on_;
    int  count_;        ///< Left keyboard event to ignore.
    QTimer timer_;
};

#endif // SYS_KEYBOARD_FILTER_H_
