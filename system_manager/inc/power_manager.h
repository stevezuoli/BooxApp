#ifndef POWER_MANAGER_H_
#define POWER_MANAGER_H_

#include <QtCore/QtCore>
#include <QString>
#ifdef BUILD_FOR_ARM
#include <qwindowsystem_qws.h>
#include <sys/time.h>
#include <time.h>
#endif

#include "onyx/base/base.h"
#include "onyx/base/device.h"
#include "gpio/gpio.h"
#include "cpu_monitor.h"


/// Power manager. It reports battery status and provides interface
/// to change OS state. So far, only idle and standby are available.
/// The power manager implements necessary methods of QWSScreenSaver.
/// So that it can make full use of information from Qt system.
class PowerManager : public QObject
#ifdef BUILD_FOR_ARM
                   , public QWSScreenSaver
#endif
{
    Q_OBJECT
public:
    PowerManager(Gpio & gpio);
    ~PowerManager();

public Q_SLOTS:
    // Battery.
    void updateBattery();
    bool batteryStatus(int& left, int & status);
    int  voltage() { return voltage_; }
    bool chargeButtonCell(bool charge);
    void usbCableChanged(bool connected);
    bool isUSBConnected();

    bool canUpdateScreen();

    // Configuration.
    bool setSuspendInterval(int ms);
    int  suspendInterval();
    bool setShutdownInterval(int ms);
    int  shutdownInterval();
    void setEPITTimerInterval(const int seconds);

    // Interface to OS.
    bool shutdown();
    bool deepSleep();
    bool idle(bool force = false);

    // Idle.
    void onIdle();
    void resetIdle();
    void enableIdle(bool enable = true);
    bool isIdleEnabled() { return (idle_count_ <= 0); }

    void enableLdo123(bool enable);
    void enableUSBPhy(bool enable);

    bool epitTimerFlag() { return epit_flag_; }
    void setEpitTimerFlag(bool flag) { epit_flag_ = flag; }

public:
    virtual void restore();
    virtual bool save(int level);

Q_SIGNALS:
    void batteryChangedSignal(int left, int status);
    void batteryFull();

    void systemIdleSignal(int, bool &);
    void aboutToSuspend();
    void aboutToShutdown(bool can_update_screen);

    void wakeupByEPIT();

private:
    int type() { return type_; }

    void writeString(const char *str);

    void recordInterrupts();
    bool wakeupByEPITTimer();
    int  readInterrupts();

    bool createBatteryWatcher();
    bool checkBattery();

    void saveTime();
    bool syncSystemTimeFromRtc();
    unsigned long myMakeTime(const unsigned int year0, const unsigned int mon0,
                             const unsigned int day, const unsigned int hour,
                             const unsigned int min, const unsigned int sec);

    void writeLP3971Ctrl(int value);

private Q_SLOTS:
    void onBatteryTimeout();

    bool isButtonCellCharging() { return charge_button_cell_; }

private:
    Gpio & gpio_;
    FILE * battery_fd_;
    scoped_ptr<QSocketNotifier> watcher_;
    int voltage_;
    int left_;      ///< Cached value.
    int status_;
    int minutes_;
    bool charge_button_cell_;
    QTimer battery_timer_;

    int type_;  ///< Deep sleep or shutdown.
    int idle_count_;    ///< Use reference counting.
    CPUMonitor monitor_;
    int mxc_pm_interrupts_;
    bool epit_flag_;  ///< epit timer flag.

#ifdef BUILD_FOR_ARM
    unsigned long prev_;  ///< Time stamp.
    struct timeval old_;  ///< Old time.
#endif
};

#endif // POWER_MANAGER_H_
