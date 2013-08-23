
#ifndef DEVICE_ADAPTOR_H_
#define DEVICE_ADAPTOR_H_

#include <QtGui/QtGui>
#include "onyx/base/dbus.h"
#include "sys_context.h"
#include "mount_entry_watcher.h"
#include "udev_watcher.h"
#include "power_manager.h"
#include "touch_screen_manager.h"
#include "sys_keyboard_filter.h"
#include "sound_manager.h"
#include "baby_sitter.h"
#include "wifi_manager.h"
#include "3G_manager.h"
#include "screen_manager_interface.h"
#include "drm_manager.h"
#include "messenger_manager.h"

/// Device manager which reports device states.
class SystemManagerAdaptor : QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.system_manager")

public:
    SystemManagerAdaptor(QApplication &app);
    ~SystemManagerAdaptor();

public Q_SLOTS:
    bool isProcessRunning(const QString & proc_name);
    bool batteryStatus(int& left, int& status);
    bool updateBatteryStatus();

    bool isUSBMounted();
    bool isSDMounted();
    bool isFlashMounted();

    bool umountUSB();
    bool umountSD();
    bool umountFlash();

    void sdCardChanged(bool insert);
    void sdioChanged(bool on);
    bool enableSdio(bool enable = true);
    bool sdioState();

    /// Called by udev.
    void udevUSBConnectToPC(bool connected);
    bool workInUSBSlaveMode();

    bool setScreenTransformation(int degree);

    bool clearCalibration();
    bool calibrate(int screen_top_left_x, int screen_top_left_y,
                   int screen_bottom_left_x, int screen_bottom_left_y,
                   int screen_bottom_right_x, int screen_bottom_right_y,
                   int screen_top_right_x, int screen_top_right_y,
                   int screen_center_x, int screen_center_y,
                   int dev_top_left_x, int dev_top_left_y,
                   int dev_bottom_left_x, int dev_bottom_left_y,
                   int dev_bottom_right_x, int dev_bottom_right_y,
                   int dev_top_right_x, int dev_top_right_y,
                   int dev_center_x, int dev_center_y);

    bool requestMusicPlayer(int);
    void reportMusicPlayerState(int);

    // Power management.
    bool setSuspendInterval(int ms);
    int  suspendInterval();
    bool setShutdownInterval(int ms);
    int  shutdownInterval();
    void startSingleShotHardwareTimer(const int seconds);
    void setDefaultHardwareTimerInterval();

    void resetIdle();
    void enableIdle(bool enable);
    bool isIdleEnabled();

    void enablePowerManagement(bool enable);
    bool suspend();
    void shutdown(int r);

    // Volume management.
    int volume();
    bool setVolume(int volume);

    bool mute(bool m);
    bool isMute();

    // Wi-Fi
    bool isWpaSupplicantRunning();
    bool startWpaSupplicant(const QString & path);
    void stopWpaSupplicant();
    bool acquireAddress(bool acquire);
    bool assignStaticAddress(const QString & ip,
                             const QString & mask,
                             const QString & gateway,
                             const QString & dns1,
                             const QString & dns2);

    // 3G network
    void modemNotify(const QString &message, const QString &vendor, const QString & product);
    bool connect3g(const QString & chat_file = QString(), const QString & username = QString(), const QString & password = QString());
    void disconnect3g();
    bool report3GNetworkSignal(int strength, int max, int type);
    void report3GPowerSwitch(bool on);
    bool isPowerSwitchOn();

    // System service.
    void setSystemBusy(bool busy, bool show_indicator);
    void snapshot(const QString &path);

    // Download and DRM
    void reportDownloadState(const QString &path, int percentage, bool open);
    void downloadFinished(const QString & url, int);
    void triggerOnlineService();
    bool startDRMService(const QStringList &strings);
    bool stopDRMService();

    // Messenger
    bool startMessenger();
    bool stopMessenger();

    bool setGrayScale(int colors);
    int grayScale();

    static void resetTouchScreen();
    bool hasTouchScreen();

    void reportUSBConnectionChanged(bool connected);

public Q_SLOTS:
    void dbgUpdateBattery(int left, int status);
    void dbgChangeUSBCable(bool);

Q_SIGNALS:
    void mountTreeSignal(bool inserted, const QString &mount_point);
    void sdCardChangedSignal(bool insert);
    void sdioChangedSignal(bool on);

    void batterySignal(int left, int status);
    void systemIdleSignal();
    void screenRotated(const int);

    // USB connection event. connectToPC means the cable is connected
    // but PC does not recognize it as a disk
    void connectToPC(bool connected);
    void inUSBSlaveMode();

    // it means the USB protocol is running.
    void USBConnectionChanged(bool connected);

    void aboutToSuspend();
    void wakeup();
    void aboutToShutdown();

    void addressAcquired(bool ok);
    void onlineServiceSignal();
    void forceQuit();

    void pppConnectionChanged(const QString &, int);
    void signalStrengthChanged(int strength, int total, int type);
    void imeiAvailable(const QString &);

    void volumeChanged(int new_volume, bool is_mute);
    void volumeUpPressed();
    void volumeDownPressed();

    void wifiDeviceChanged(bool enabled);
    void stylusChanged(bool inserted);

    /// The signal also serves as a command to listeners.
    /// The listener will start music player if necessary.
    void musicPlayerStateChanged(int);

    void requestDRMUserInfo(const QString &string, const QString & param);
    void fulfillmentFinished(const QString & string);
    void loanReturnFinished(const QString & string);
    void reportWorkflowError(const QString & workflow, const QString & error_code);
    void downloadStateChanged(const QString &path, int percentage, bool);

    void hardwareTimerTimeout();
private Q_SLOTS:
    void onBatteryChanged(int, int);
    void onBatteryFull();
    void onWakeupByEPIT();

    void onUSBChanged(bool mounted, const QString &mount_point);
    void onSDChanged(bool mounted, const QString &mount_point);
    void onFlashChanged(bool mounted, const QString &mount_point);
    void onSystemIdle(int, bool &);

    void preSuspend();
    void postSuspend();

    void onAboutToSuspend();
    void onAboutToShutdown(bool can_update_screen);
    void onShutdownTimeout();
    void cleanSuspend();

    void onUSBCableChanged(bool connected);
    bool isConnectedToPC();
    void onPreConnectToPC(bool);
    void changeUSBState(const QString & state);

    void onStandbyPressed();
    void onShutdownPressed();
    void onWifiKeyChanged(bool on);
    void onStylusChanged(bool inserted);
    void onStylusDistanceChanged(bool press);

    void onVolumeUpPressed();
    void onVolumeDownPressed();
    void onVolumeChanged(int new_volume, bool is_mute);

    void onAddressAcquired(bool);
    void onlineServicePressed();
    void onForceQuitPressed();
    void onPrintScreen();
    void startGuiShell(QApplication & app);

    void broadcastInitialSignals();

    QTimer & ledTimer();
    void led(bool on = true);
    void blinkLed(int ms = 5000);
    void stopLedBlinking();
    void onLedTimeout();
    void toggleLed();
    void chargeLed(bool on = true);

    void onPppdExit(int);
    void onPppConnectionChanged(const QString &, int);
    void onImeiAvailable(const QString &);

    void onRequestDRMUserInfo(const QString &string, const QString & param);
    void onFulfillmentFinished(const QString & string);
    void onLoanReturnFinished(const QString & string);
    void onReportWorkflowError(const QString & workflow, const QString & error_code);

private:
    void enableInput(bool enable);
    void enableKeyboard(bool enable);
    void enableStandbySignal(bool enable);
    void enableMouseHandler(bool enable);

    bool isWirelessOn();

    void broadcastSuspendSignal();

    SoundManager & soundManager();
    WifiManager & wifiManager();
    ThreeGManager & threeGManager();
    DRMManager & drmManager();
    MessengerManager & messengerManager();

    void refreshScreen();
    void screenSaverActivate(bool activate);

    void powerOff();

private:
    static Gpio gpio_;

    scoped_ptr<BabySitter> baby_sitter_;
    scoped_ptr<WifiManager> wifi_manager_;
    scoped_ptr<ThreeGManager> threeg_manager_;
    scoped_ptr<MessengerManager> messenger_manager_;

    MountEntryWatcher mount_entry_watcher_;
    scoped_ptr<SoundManager> sound_manager_;
    PowerManager power_manager_;
    SysKeyboardFilter keyboard_filter_;

    scoped_ptr<ScreenManagerInterface> screen_manager_;

    scoped_ptr<DRMManager> drm_manager_;

    scoped_ptr<QTimer> led_timer_;

    // UdevWatcher udev_watcher_;
    Context context_;
};

inline bool SystemManagerAdaptor::isUSBMounted()
{
    return mount_entry_watcher_.isUSBMounted();
}

inline bool SystemManagerAdaptor::isSDMounted()
{
    return mount_entry_watcher_.isSDMounted();
}

inline bool SystemManagerAdaptor::isFlashMounted()
{
    return mount_entry_watcher_.isFlashMounted();
}

inline bool SystemManagerAdaptor::umountUSB()
{
    return mount_entry_watcher_.umountUSB();
}

inline bool SystemManagerAdaptor::umountSD()
{
    return mount_entry_watcher_.umountSD();
}

inline bool SystemManagerAdaptor::umountFlash()
{
    return mount_entry_watcher_.umountFlash();
}

inline bool SystemManagerAdaptor::isConnectedToPC()
{
    return !mount_entry_watcher_.isFlashMounted();
}

inline bool SystemManagerAdaptor::setSuspendInterval(int ms)
{
    return power_manager_.setSuspendInterval(ms);
}

inline int  SystemManagerAdaptor::suspendInterval()
{
    return power_manager_.suspendInterval();
}

inline bool SystemManagerAdaptor::setShutdownInterval(int ms)
{
    return power_manager_.setShutdownInterval(ms);
}

inline int  SystemManagerAdaptor::shutdownInterval()
{
    return power_manager_.shutdownInterval();
}

inline void SystemManagerAdaptor::resetIdle()
{
    power_manager_.resetIdle();
}

inline void SystemManagerAdaptor::enableIdle(bool enable)
{
    power_manager_.enableIdle(enable);
}

inline bool SystemManagerAdaptor::isIdleEnabled()
{
    return power_manager_.isIdleEnabled();
}


#endif // DEVICE_ADAPTOR_H_

