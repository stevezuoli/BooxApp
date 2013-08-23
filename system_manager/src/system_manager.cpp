#include "system_manager.h"
#ifdef BUILD_FOR_ARM
#include <QtGui/qwsdisplay_qws.h>
#include <QWSPointerCalibrationData>
#include <qwindowsystem_qws.h>
#endif
#include <iostream>
#include <QRegExp>
#include "onyx/sys/sys_utils.h"
#include "bs_screen_manager.h"

static const QString USB_STATE_DETECT = "detect";
static const QString USB_STATE_CONNECT = "connect";
static const QString USB_STATE_DISCONNECT = "disconnect";
static const QString USB_3G_HOST = "3G-host";
static const QString USB_CLEANUP = "cleanup";
static const int TIMEOUT = 1000;

// Given a list of commandline arguments, find the path of GUI shell specified
// by -shell=...
static QString shellPath(const QStringList& args)
{
    QRegExp arg_regex("-shell=(.*)");
    int index = args.indexOf(arg_regex);
    if (index == -1)
    {
        std::cerr << "Please specify the path to the GUI shell executable with "
                  << "-shell=<path>."
                  << std::endl;
        return "";
    }
    else
    {
        QString arg(args.at(index));
        arg_regex.indexIn(arg);
        QString path(arg_regex.cap(1));
        return path;
    }
}

static bool s_vdd_fixed = true;
static bool isVddFixed()
{
    return s_vdd_fixed;
}

static bool shouldSleep()
{
    return !isVddFixed();
}

Gpio SystemManagerAdaptor::gpio_;

SystemManagerAdaptor::SystemManagerAdaptor(QApplication &app)
: QDBusAbstractAdaptor(&app)
, power_manager_(gpio_)
, screen_manager_(new BSScreenManager)
{
#ifndef _WINDOWS
    QDBusConnection::systemBus().registerService("com.onyx.service.system_manager");
    QDBusConnection::systemBus().registerObject("/com/onyx/object/system_manager", &app);
#else
    QDBusConnection::sessionBus().registerService("com.onyx.service.system_manager");
    QDBusConnection::sessionBus().registerObject("/com/onyx/object/system_manager", &app);
#endif

    s_vdd_fixed = (qgetenv("FIXED_VDD").toInt() > 0);

    // Battery.
    connect(&power_manager_,
        SIGNAL(batteryChangedSignal(int,int)),
        this,
        SLOT(onBatteryChanged(int,int)));

    connect(&power_manager_,
            SIGNAL(batteryFull()),
            this,
            SLOT(onBatteryFull()));

    connect(&power_manager_,
            SIGNAL(wakeupByEPIT()),
            this,
            SLOT(onWakeupByEPIT()));

    // Mount tree.
    connect(&mount_entry_watcher_,
        SIGNAL(USBChanged(bool, const QString&)),
        this,
        SLOT(onUSBChanged(bool, const QString&)));

    connect(&mount_entry_watcher_,
        SIGNAL(SDChanged(bool, const QString&)),
        this,
        SLOT(onSDChanged(bool, const QString&)));

    connect(&mount_entry_watcher_,
            SIGNAL(flashChanged(bool, const QString&)),
            this,
            SLOT(onFlashChanged(bool, const QString &)));

    // Power manager
    connect(&power_manager_,
        SIGNAL(systemIdleSignal(int, bool &)),
        this,
        SLOT(onSystemIdle(int, bool &)));

    connect(&power_manager_,
        SIGNAL(aboutToSuspend()),
        this,
        SLOT(onAboutToSuspend()));

    connect(&power_manager_,
        SIGNAL(aboutToShutdown(bool)),
        this,
        SLOT(onAboutToShutdown(bool)));

    // Enable wacom touch screen.
    TouchScreenManager::instance().enableTouchScreen(true);

    // Install keyboard filter.
#ifdef BUILD_FOR_ARM
    QWSServer::addKeyboardFilter(&keyboard_filter_);
#endif
    connect(&keyboard_filter_,
        SIGNAL(standbyPressed()),
        this,
        SLOT(onStandbyPressed()));

    connect(&keyboard_filter_,
        SIGNAL(shutdownPressed()),
        this,
        SLOT(onShutdownPressed()));

    connect(&keyboard_filter_,
        SIGNAL(wifiKeyChanged(bool)),
        this,
        SLOT(onWifiKeyChanged(bool)));
    connect(&keyboard_filter_,
        SIGNAL(stylusChanged(bool)),
        this,
        SLOT(onStylusChanged(bool)));
    connect(&keyboard_filter_,
        SIGNAL(stylusDistanceChanged(bool)),
        this,
        SLOT(onStylusDistanceChanged(bool)));
    connect(&keyboard_filter_,
        SIGNAL(volumeUpPressed()),
        this,
        SLOT(onVolumeUpPressed()));
    connect(&keyboard_filter_,
        SIGNAL(volumeDownPressed()),
        this,
        SLOT(onVolumeDownPressed()));
    connect(&keyboard_filter_,
        SIGNAL(forceQuitPressed()),
        this,
        SLOT(onForceQuitPressed()));
    connect(&keyboard_filter_,
        SIGNAL(usbCableChanged(bool)),
        this,
        SLOT(onUSBCableChanged(bool)));
    connect(&keyboard_filter_,
        SIGNAL(onlineServicePressed()),
        this,
        SLOT(onlineServicePressed()));
    connect(&keyboard_filter_,
        SIGNAL(printScreenSignal()),
        this,
        SLOT(onPrintScreen()));
    connect(&keyboard_filter_,
            SIGNAL(powerSwitchChanged(bool)),
            this,
            SLOT(report3GPowerSwitch(bool)));

    power_manager_.enableUSBPhy(true);

    startGuiShell(app);

    // Post startup.
    QTimer::singleShot(3500, this, SLOT(broadcastInitialSignals(void)));

    // Turn off led.
    led(false);

    // Ensure wireless is off when startup.
    wifiManager().enableDevice(false);

    qDebug("Ignore  3G device now.");
    //threeGManager().enableDevice(false);
}

SystemManagerAdaptor::~SystemManagerAdaptor()
{
}

bool SystemManagerAdaptor::batteryStatus(int& left,
                                         int& status)
{
    return power_manager_.batteryStatus(left, status);
}

bool SystemManagerAdaptor::updateBatteryStatus()
{
    power_manager_.updateBattery();
    return true;
}

bool SystemManagerAdaptor::isWirelessOn()
{
    // Don't use gpio any more. check file system instead.
    if (!wifiManager().isEnabled())
    {
        return false;
    }

    int value = runScript("read_wifi_state.sh");
    return (value == 1);
}

/// Change screen rotation to specified degree.
bool SystemManagerAdaptor::setScreenTransformation(int degree)
{
    qDebug("change screen transformation to %d", degree);
#ifdef BUILD_FOR_ARM
    QWSDisplay::setTransformation(degree / 90);
    // Need to write to system configuratio file.
#endif
    emit screenRotated(degree);
    return true;
}

bool SystemManagerAdaptor::clearCalibration()
{
#ifdef BUILD_FOR_ARM
    QWSServer::mouseHandler()->clearCalibration();
#endif
    return true;
}

/// The QtDBus does not support list natively, so have to list
/// these parameters.
bool SystemManagerAdaptor::calibrate(int screen_top_left_x, int screen_top_left_y,
                                     int screen_bottom_left_x, int screen_bottom_left_y,
                                     int screen_bottom_right_x, int screen_bottom_right_y,
                                     int screen_top_right_x, int screen_top_right_y,
                                     int screen_center_x, int screen_center_y,
                                     int dev_top_left_x, int dev_top_left_y,
                                     int dev_bottom_left_x, int dev_bottom_left_y,
                                     int dev_bottom_right_x, int dev_bottom_right_y,
                                     int dev_top_right_x, int dev_top_right_y,
                                     int dev_center_x, int dev_center_y)
{
#ifdef BUILD_FOR_ARM
    QWSPointerCalibrationData data;
    QPoint *points = data.screenPoints;
    points[QWSPointerCalibrationData::TopLeft] = QPoint(screen_top_left_x, screen_top_left_y);
    points[QWSPointerCalibrationData::BottomLeft] = QPoint(screen_bottom_left_x, screen_bottom_left_y);
    points[QWSPointerCalibrationData::BottomRight] = QPoint(screen_bottom_right_x, screen_bottom_right_y);
    points[QWSPointerCalibrationData::TopRight] = QPoint(screen_top_right_x, screen_top_right_y);
    points[QWSPointerCalibrationData::Center] = QPoint(screen_center_x, screen_center_y);

    points = data.devPoints;
    points[QWSPointerCalibrationData::TopLeft] = QPoint(dev_top_left_x, dev_top_left_y);
    points[QWSPointerCalibrationData::BottomLeft] = QPoint(dev_bottom_left_x, dev_bottom_left_y);
    points[QWSPointerCalibrationData::BottomRight] = QPoint(dev_bottom_right_x, dev_bottom_right_y);
    points[QWSPointerCalibrationData::TopRight] = QPoint(dev_top_right_x, dev_top_right_y);
    points[QWSPointerCalibrationData::Center] = QPoint(dev_center_x, dev_center_y);

    QWSServer::mouseHandler()->calibrate(&data);
#endif
    return true;
}

bool SystemManagerAdaptor::requestMusicPlayer(int cmd)
{
    emit musicPlayerStateChanged(cmd);
    return true;
}

void SystemManagerAdaptor::reportMusicPlayerState(int state)
{
    emit musicPlayerStateChanged(state);
}

void SystemManagerAdaptor::onBatteryChanged(int current,
                                            int status)
{
    // Broadcast the signal to all listeners.
    emit batterySignal(current, status);

    // Re-activate screen saver if necessary.
    if (!(status & BATTERY_STATUS_CHARGING))
    {
        screenSaverActivate(false);
    }

    // Check if we need to shutdown the device or not.
    // Don't need to do it here. GUI shell should handle that.
    // if (status & BATTERY_STATUS_DANGEROUS)
    // {
    //     // OK, have to shutdown the device now.
    //     onAboutToShutdown();
    // }
}

void SystemManagerAdaptor::onBatteryFull()
{
    // chargeLed(false);
}

void SystemManagerAdaptor::startSingleShotHardwareTimer(const int seconds)
{
    power_manager_.setEpitTimerFlag(true);
    power_manager_.setEPITTimerInterval(seconds);
}

void SystemManagerAdaptor::setDefaultHardwareTimerInterval()
{
    power_manager_.setEPITTimerInterval(180);
}

void SystemManagerAdaptor::onWakeupByEPIT()
{
    if (power_manager_.epitTimerFlag())
    {
        power_manager_.setEpitTimerFlag(false);
        emit hardwareTimerTimeout();
    }

    QTime t;
    t.start();
    while (QApplication::hasPendingEvents() || t.elapsed() <= TIMEOUT)
    {
        QApplication::processEvents();
    }

    // To solve white line issue of LG EPD.
    if (!shouldSleep())
    {
        screen_manager_->showCurrentDeepSleepScreen();
    }
}

void SystemManagerAdaptor::onUSBChanged(bool mounted,
                                        const QString &mount_point)
{
    emit mountTreeSignal(mounted, mount_point);
}

// TODO, change them later.
void SystemManagerAdaptor::onSDChanged(bool mounted,
                                       const QString &mount_point)
{
    if (context_.canBroadcast())
    {
        qDebug("SystemManagerAdaptor::onSDChanged. Need to reset screensaver.");
        screenSaverActivate(false);
        emit mountTreeSignal(mounted, mount_point);
    }
}

void SystemManagerAdaptor::onFlashChanged(bool mounted, const QString &mount_point)
{
    emit mountTreeSignal(mounted, mount_point);
}

/// This function is called by naboo_sd_handler through dbus-send
/// It tells the system manager that sd card is removed by user or is
/// added by user. But it does NOT mean that the mount tree is ready.
/// The reason we introduce this function is that certain application
/// may lock files on sd card, which prevent mount tree from updating.
void SystemManagerAdaptor::sdCardChanged(bool inserted)
{
    // If the device just wakes up from sleep, system manager
    // receives removed and inserted event in short time.
    context_.changeSd(inserted);
    if (context_.canBroadcast())
    {
        emit sdCardChangedSignal(inserted);
        // emit mountTreeSignal(inserted, SDMMC_ROOT);
    }
}

void SystemManagerAdaptor::sdioChanged(bool on)
{
    emit sdioChangedSignal(on);
}

bool SystemManagerAdaptor::enableSdio(bool enable)
{
    wifiManager().enableDevice(enable);
    return true;
}

bool SystemManagerAdaptor::sdioState()
{
    return isWirelessOn();
}

void SystemManagerAdaptor::onSystemIdle(int level, bool & processed)
{
    // qDebug("System idle now. Turn off controller");

    // TODO, consider to close touch screen or at least make it in standby mode.

    if (level <= 0)
    {
        // Turn controller off.
        power_manager_.onIdle();

        if (shouldSleep())
        {
            qDebug("Turn off controller.");
            screen_manager_->sleep();
        }
        emit systemIdleSignal();
    }
    else
    {
        if (!power_manager_.isUSBConnected())
        {
            processed = power_manager_.idle();
            screenSaverActivate(false);
        }
    }
}

void SystemManagerAdaptor::broadcastSuspendSignal()
{
    if (context_.inc_suspend_count() <= 1)
    {
        qDebug("system manager broadcast aboutToSuspend signal.");
        emit aboutToSuspend();
    }
    QTimer::singleShot(4500, this, SLOT(cleanSuspend()));
}

void SystemManagerAdaptor::onAboutToSuspend()
{
    // Consider ac line status.
    int left = 0, status = BATTERY_STATUS_NORMAL;
    power_manager_.batteryStatus(left, status);
    if (!(status & BATTERY_STATUS_CHARGING))
    {
        broadcastSuspendSignal();
    }
}

void SystemManagerAdaptor::onAboutToShutdown(bool can_update_screen)
{
    // Consider ac line status.
    int left = 0, status = BATTERY_STATUS_NORMAL;
    power_manager_.batteryStatus(left, status);
    if (!(status & BATTERY_STATUS_CHARGING))
    {
        if (can_update_screen)
        {
            emit aboutToShutdown();
        }
        else
        {
            // Shutdown immediately.
            powerOff();
        }
    }
}

void SystemManagerAdaptor::onShutdownTimeout()
{
    shutdown(SHUTDOWN_REASON_USER_REQUEST);
}

void SystemManagerAdaptor::cleanSuspend()
{
    if (context_.dec_suspend_count() < 0)
    {
        context_.rst_suspend_count();
    }
}

/// USB cable change event handler. It does not mean the device
/// is connected to PC.
void SystemManagerAdaptor::onUSBCableChanged(bool connected)
{
    // Need to double check, sometimes release event is emitted
    // but USB cable is connected.
    bool high = power_manager_.isUSBConnected();
    if (high != connected)
    {
        qWarning("High voltage and cable state mismatched, use voltage %d %d.", high, connected);
        connected = high;
    }
    qDebug("USB cable changed %d", connected);
    power_manager_.usbCableChanged(connected);

    // Always turn on charge led.
    // chargeLed(true);

    // Load necessary module at first.
    if (connected)
    {
        // usb_module detect
        changeUSBState(USB_STATE_DETECT);
    }
    else
    {
        if (context_.usb_state() == USB_STATE_CONNECT)
        {
            // unload all modules, usb_module normal
            changeUSBState(USB_STATE_DISCONNECT);
        }

        // Need to reset timer as Qt does not use key release event
        // to reset screen saver.
        screenSaverActivate(false);
    }
}

/// This function is called by udev daemon through udev rule.
/// When it's called, we're sure that the device is connected to
/// PC, so we can broadcast a signal to all applications.
/// TODO: When disconnected, it can not receive any signal so far.
void SystemManagerAdaptor::udevUSBConnectToPC(bool connected)
{
    onPreConnectToPC(connected);
}

/// Already connected to PC, but we need to ask user if he wants
/// the device to become a USB disk or not.
/// This function will also be called when user safely disconnect
/// the USB connection.
void SystemManagerAdaptor::onPreConnectToPC(bool connected)
{
    qDebug("connection to pc changed now %d.", connected);

    // We only broadcast connected signal here, the disconnected signal
    // will be broadcasted by shell script by using reportUSBConnectionChanged.
    if (connected)
    {
        emit connectToPC(connected);
    }

    if (!connected)
    {
        changeUSBState(USB_STATE_DISCONNECT);
    }
}

/// A test interface enable us to change usb cable state through dbus.
void SystemManagerAdaptor::dbgChangeUSBCable(bool connected)
{
    qDebug("usb cable changed.");
}

void SystemManagerAdaptor::dbgUpdateBattery(int current, int status)
{
    emit batterySignal(current, status);
}


/// This function is used when user wants the device to work in usb slave mode.
/// At this stage, the device becomes a usb disk to pc. All applications
/// should release the resource on internal flash so that system manager can 
/// umount the internal flash.
bool SystemManagerAdaptor::workInUSBSlaveMode()
{
    emit inUSBSlaveMode();

    // Change usb state.
    changeUSBState(USB_STATE_CONNECT);
    return true;
}

/// Change USB state by calling usb_module state.
void SystemManagerAdaptor::changeUSBState(const QString & state)
{
    if (state == USB_STATE_CONNECT)
    {
        enableInput(false);
        screen_manager_->enableUpdate(false);
        screen_manager_->showUSBScreen();

        // it's ok to broadcast signal here, because the sd card and internal flash
        // are not really un-mounted yet.
        emit mountTreeSignal(false, SDMMC_ROOT);
        emit mountTreeSignal(false, LIBRARY_ROOT);
    }
    else if (state == USB_STATE_DISCONNECT)
    {
        enableInput(true);
        screen_manager_->enableUpdate(true);
        refreshScreen();

        // should not broadcast signal here.
    }
    context_.mutable_usb_state() = state;

    // Have to use startDetached otherwise it may hang as some udev script
    // also invoke method of system manager.
    QProcess script;
    script.setEnvironment(QProcess::systemEnvironment());
    script.startDetached("usb_module.sh", QStringList(state));
}

void SystemManagerAdaptor::onStandbyPressed()
{
    // Need to check the wakeup to see if it's wakeup just now.
    // If it's waken up by power key, just ignore the key.
    if (!context_.sleeping() && context_.usb_state() != USB_STATE_CONNECT)
    {
        qDebug("system manager broadcast aboutToSuspend signal.");
        broadcastSuspendSignal();
    }
    context_.toggleSleep();
}

/// The shutdown key pressed. We need to make sure that system can
/// be shutdown no matter GUI shell is running or not.
void SystemManagerAdaptor::onShutdownPressed()
{
    emit aboutToShutdown();

    // increase suspend count, so it can shutdown correctly.
    context_.inc_suspend_count();

    // Ensure shutdown is called in 8 seconds.
    QTimer::singleShot(8 * 1000, this, SLOT(onShutdownTimeout()));
}

void SystemManagerAdaptor::onWifiKeyChanged(bool on)
{
    // We don't star the wifi manager when system manager
    // is launched. It will be started only when the switcher
    // is on.
    wifiManager().enableDevice(on);
    if (on)
    {
        wifiManager().startWpaSupplicant("");
    }
    else
    {
        wifiManager().stopWpaSupplicant();
    }
    emit wifiDeviceChanged(on);
}

/// The touch screen is automatically shutdown by kernel.
/// So not necessary to close again. Just tell all listeners
/// the message.
void SystemManagerAdaptor::onStylusChanged(bool inserted)
{
    qDebug("Stylus changed inserted %d", inserted);
    TouchScreenManager::instance().enableTouchScreen(!inserted);
    emit stylusChanged(inserted);
}

void SystemManagerAdaptor::onStylusDistanceChanged(bool press)
{
    // qDebug("Stylus distance changed %d", press);
    if (press)
    {
        // TouchScreenManager::instance().enableTouchScreen(true);
    }
}

void SystemManagerAdaptor::onVolumeUpPressed()
{
    soundManager().increaseVolume();
    emit volumeUpPressed();
}

void SystemManagerAdaptor::onVolumeDownPressed()
{
    soundManager().decreaseVolume();
    emit volumeDownPressed();
}

void SystemManagerAdaptor::onVolumeChanged(int new_volume, bool is_mute)
{
    emit volumeChanged(new_volume, is_mute);
}

void SystemManagerAdaptor::onlineServicePressed()
{
    triggerOnlineService();
}

void SystemManagerAdaptor::onForceQuitPressed()
{
    emit forceQuit();
}

void SystemManagerAdaptor::onPrintScreen()
{
    if (!QFile::exists(SDMMC_DEVICE))
    {
        qDebug("SD card not available.");
        return;
    }

    // Construct file path.
    QDir dir(SDMMC_ROOT);
    const QString SNAP_SHOT = "snapshot";
    if (!dir.cd(SNAP_SHOT))
    {
        if (!dir.mkdir(SNAP_SHOT))
        {
            return;
        }
        dir.cd(SNAP_SHOT);
    }

    QDir::Filters filters = QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot;
    QFileInfoList all = dir.entryInfoList(QStringList(), filters);

    QString path("%1_%2.png");
    path = path.arg(SNAP_SHOT).arg(all.size() + 1);
 
    qDebug("Generate screen snapshot now.\n");
    snapshot(dir.absoluteFilePath(path));
}

void SystemManagerAdaptor::startGuiShell(QApplication & app)
{
    QString cmd(shellPath(app.arguments()));
    if (cmd.isEmpty())
    {
        std::cerr << "GUI shell not found!" << std::endl;
    }
    else
    {
        baby_sitter_.reset(new BabySitter(cmd));
        baby_sitter_->start();
    }
}

/// Need to broadcast some initial signals.
void SystemManagerAdaptor::broadcastInitialSignals()
{
    if (power_manager_.isUSBConnected())
    {
        onUSBCableChanged(true);
    }

    // Reset screen saver timer.
    screenSaverActivate(false);
}

void SystemManagerAdaptor::led(bool on)
{
    if (on)
    {
        gpio_.setValue(0, 4, 1);
    }
    else
    {
        gpio_.setValue(0, 4, 0);
    }
}

QTimer & SystemManagerAdaptor::ledTimer()
{
    if (!led_timer_)
    {
        led_timer_.reset( new QTimer());
        led_timer_->setSingleShot(false);
        connect(led_timer_.get(), SIGNAL(timeout()), this, SLOT(onLedTimeout()));
    }
    return *led_timer_;
}

void SystemManagerAdaptor::blinkLed(int ms)
{
    ledTimer().start(ms);
}

void SystemManagerAdaptor::stopLedBlinking()
{
    led(false);
    ledTimer().stop();
}

void SystemManagerAdaptor::onLedTimeout()
{
    toggleLed();
}

void SystemManagerAdaptor::toggleLed()
{
    static bool on = true;
    led(on);
    if (on)
    {
        QTimer::singleShot(300, this, SLOT(toggleLed()));
    }
    on = !on;

}

void SystemManagerAdaptor::chargeLed(bool on)
{
    if (on)
    {
        gpio_.setValue(0, 7, 1);
    }
    else
    {
        gpio_.setValue(0, 7, 0);
    }
}

void SystemManagerAdaptor::enableInput(bool enable)
{
    enableKeyboard(enable);
    enableMouseHandler(enable);
}

void SystemManagerAdaptor::enableKeyboard(bool enable)
{
    keyboard_filter_.enable(enable);
}

void SystemManagerAdaptor::enableStandbySignal(bool enable)
{
    keyboard_filter_.enableStandby(enable);
}

void SystemManagerAdaptor::enableMouseHandler(bool enable)
{
#ifdef BUILD_FOR_ARM
    if (enable)
    {
        QWSServer::mouseHandler()->resume();
    }
    else
    {
        QWSServer::mouseHandler()->suspend();
    }
#endif
}

SoundManager & SystemManagerAdaptor::soundManager()
{
    if (!sound_manager_)
    {
        sound_manager_.reset(new SoundManager);

        // Volume manager.
        connect(sound_manager_.get(), SIGNAL(volumeChanged(int, bool)), this, SLOT(onVolumeChanged(int, bool)));
    }

    return *sound_manager_;
}

WifiManager & SystemManagerAdaptor::wifiManager()
{
    if (!wifi_manager_)
    {
        wifi_manager_.reset(new WifiManager(gpio_));
        connect(wifi_manager_.get(), SIGNAL(addressAcquired(bool)), this, SLOT(onAddressAcquired(bool)));
    }
    return *wifi_manager_;
}

ThreeGManager & SystemManagerAdaptor::threeGManager()
{
    if (!threeg_manager_)
    {
        threeg_manager_.reset(new ThreeGManager(gpio_));
        connect(threeg_manager_.get(), SIGNAL(pppdExit(int)), this, SLOT(onPppdExit(int)));
        connect(threeg_manager_.get(), SIGNAL(pppConnectionChanged(const QString &, int)),
                this, SLOT(onPppConnectionChanged(const QString &, int)));
        connect(threeg_manager_.get(), SIGNAL(imeiAvailable(const QString&)),
                this, SLOT(onImeiAvailable(const QString&)));
        connect(threeg_manager_.get(), SIGNAL(signalStrengthChanged(int, int, int)), this,
            SLOT(report3GNetworkSignal(int, int, int)));
    }
    return *threeg_manager_;
}

DRMManager & SystemManagerAdaptor::drmManager()
{
    if (!drm_manager_)
    {
        drm_manager_.reset(new DRMManager);
        connect(drm_manager_.get(), SIGNAL(requestDRMUserInfo(const QString &, const QString &)),
                this, SLOT(onRequestDRMUserInfo(const QString &, const QString &)));
        connect(drm_manager_.get(), SIGNAL(fulfillmentFinished(const QString &)),
                this, SLOT(onFulfillmentFinished(const QString &)));
        connect(drm_manager_.get(), SIGNAL(loanReturnFinished(const QString &)),
                this, SLOT(onLoanReturnFinished(const QString &)));
        connect(drm_manager_.get(), SIGNAL(reportWorkflowError(const QString &, const QString &)),
                this, SLOT(onReportWorkflowError(const QString &, const QString &)));
    }
    return *drm_manager_;
}

MessengerManager & SystemManagerAdaptor::messengerManager()
{
    if (!messenger_manager_)
    {
        messenger_manager_.reset(new MessengerManager);
    }
    return *messenger_manager_;
}

/// When pppd exit, this function is invoked to clean up resource.
void SystemManagerAdaptor::onPppdExit(int code)
{
    emit pppConnectionChanged(threeGManager().errStr(code), TG_DISCONNECTED);
    stopLedBlinking();
    enableIdle(true);
}

void SystemManagerAdaptor::onPppConnectionChanged(const QString &message, int value)
{
    emit pppConnectionChanged(message, value);
    if (value == TG_CONNECTED)
    {
        blinkLed();
    }
    else if (value == TG_DISCONNECTED ||
             value == TG_STOP ||
             value == TG_INVALID)
    {
        stopLedBlinking();
    }
}

void SystemManagerAdaptor::onImeiAvailable(const QString &imei)
{
    qDebug("imei value %s", qPrintable(imei));
}

bool SystemManagerAdaptor::startDRMService(const QStringList &strings)
{
    qDebug("Start DRM Service.");
    return drmManager().start(strings);
}

bool SystemManagerAdaptor::stopDRMService()
{
    qDebug("Stop DRM Service.");
    return drmManager().stop();
}

bool SystemManagerAdaptor::startMessenger()
{
    return messengerManager().start();
}

bool SystemManagerAdaptor::stopMessenger()
{
    return messengerManager().stop();
}

void SystemManagerAdaptor::onRequestDRMUserInfo(const QString &string, const QString & param)
{
    emit requestDRMUserInfo(string, param);
}

void SystemManagerAdaptor::onFulfillmentFinished(const QString & string)
{
    emit fulfillmentFinished(string);
}

void SystemManagerAdaptor::onLoanReturnFinished(const QString & string)
{
    emit loanReturnFinished(string);
}

void SystemManagerAdaptor::onReportWorkflowError(const QString & workflow, const QString & error_code)
{
    emit reportWorkflowError(workflow, error_code);
}

void SystemManagerAdaptor::onAddressAcquired(bool ok)
{
    emit addressAcquired(ok);
}

/// Shutdown the whole system. TODO, ensure the screen is cleanup.
/// We can use the raw API provided by screen manager.
void SystemManagerAdaptor::shutdown(int r)
{
    qDebug("System manager: going to shutdown now reason %d.", r);
    if (baby_sitter_)
    {
        baby_sitter_->ensureFinished();
    }

    // Turn on led.
    led(true);

    // Turn off power supply of all devices.
    powerOff();

    // Stop system manager.
    qApp->exit();
}

void SystemManagerAdaptor::preSuspend()
{
    // Update screen at first to make user feel it's fast.
    screen_manager_->showDeepSleepScreen();
    screen_manager_->enableUpdate(false);

    // Change state to sleep mode and disable message broadcasting.
    context_.sleep(true);
    context_.enableBroadcast(false);

    // Execute the script.
    sys::runScript("pre_suspend.sh");

    // Cut down power supply of touch screen.
    TouchScreenManager::instance().turnOff(gpio_);

    // wifi.
    context_.storeWifiState(wifiManager().isEnabled());
    wifiManager().enableDevice(false);

    // 3G
    context_.store3GState(threeGManager().state() == TG_CONNECTED);
    context_.store3GEnabled(threeGManager().isEnabled());
    disconnect3g();
    stopLedBlinking();

    // keyboard.
    enableKeyboard(false);
    enableStandbySignal(false);

    // Turn off display controller.
    screen_manager_->sleep();
}

void SystemManagerAdaptor::postSuspend()
{
    // Execute the script.
    sys::runScript("post_suspend.sh");

    // Refresh screen.
    screen_manager_->enableUpdate(true);
    refreshScreen();

    // wifi.
    wifiManager().enableDevice(context_.isWifiEnabled());

    // 3G
    // threeGManager().enableDevice(threeGManager().isEnabled());

    // We can use removePostedEvents(0, QEvent::KeyRelease)
    // to remove all pending key release events.
    QCoreApplication::removePostedEvents(0, QEvent::KeyRelease);
    keyboard_filter_.ignoreKeyboardEvent(1);

    // Now, enable key.
    QTime t;
    t.start();
    while (QCoreApplication::hasPendingEvents() || t.elapsed() <= TIMEOUT)
    {
        QCoreApplication::processEvents();
    }

    // Ensure keyboard is always enabled.
    enableKeyboard(true);
    enableStandbySignal(true);
    keyboard_filter_.ignoreKeyboardEvent(0);

    // touch screen.
    TouchScreenManager::instance().resetTouchScreen(gpio_);
    TouchScreenManager::instance().turnOn(gpio_);

    // Change sleep flag and enable broadcast message now.
    context_.sleep(false);
    context_.enableBroadcast(true);
    if (context_.is3GConnected())
    {
        qDebug("Previous state is connected.");
        QTimer::singleShot(4000, this, SLOT(connect3g()));
    }

    // Reset suspend count reference.
    context_.rst_suspend_count();

    // Reset screen saver timer.
    screenSaverActivate(false);
}

/// Enable or disable power management.
void SystemManagerAdaptor::enablePowerManagement(bool enable)
{

}

/// Suspend to ram.
bool SystemManagerAdaptor::suspend()
{
    // TODO: Just disable it so far, we need a stable version.
    // The main problem is that SD card driver has some problem
    // with suspend and resume. When it's ready, we can enable
    // the deepSleep again.
    // Remember state.
    bool inserted = mount_entry_watcher_.isSDMounted();

    // Do some work before real suspend.
    preSuspend();

    // deep sleep now.
    bool ok = power_manager_.deepSleep();

    // Check battery again.
    if (!power_manager_.canUpdateScreen())
    {
        // Wait for shutdown.
        return false;
    }

    // Always process post suspend.
    postSuspend();

    // Now, re-check sd state.
    if (context_.is_sd_inserted() != inserted)
    {
        qDebug("emit mountTreeSignal after wakeup.");
        emit mountTreeSignal(context_.is_sd_inserted(), SDMMC_ROOT);
    }

    // When returns from deep sleep, broadcast wakeup signal.
    if (ok)
    {
        emit wakeup();
    }

    return true;
}

int SystemManagerAdaptor::volume()
{
    return soundManager().volume();
}

bool SystemManagerAdaptor::setVolume(int volume)
{
    return soundManager().setVolume(volume);
}

bool SystemManagerAdaptor::mute(bool m)
{
    return soundManager().mute(m);
}

bool SystemManagerAdaptor::isMute()
{
    return soundManager().isMute();
}

bool SystemManagerAdaptor::isWpaSupplicantRunning()
{
    return wifiManager().isWpaSupplicantRunning();
}

bool SystemManagerAdaptor::isProcessRunning(const QString & proc_name)
{
    QProcess builder;
    builder.setProcessChannelMode(QProcess::MergedChannels);
    builder.start("ps xf");

    if (!builder.waitForFinished())
    {
        return false;
    }
    else
    {
        QByteArray process_list(builder.readAll());
        return process_list.contains(proc_name.toAscii());
    }
}

bool SystemManagerAdaptor::startWpaSupplicant(const QString & path)
{
    wifiManager().enableDevice(true);
    return wifiManager().startWpaSupplicant(path);
}

void SystemManagerAdaptor::stopWpaSupplicant()
{
    // Don't stop wifi device.
    wifiManager().stopWpaSupplicant();
    wifiManager().acquireAddress(false);
    wifiManager().resetIfup();
    wifiManager().enableDevice(false);
}

bool SystemManagerAdaptor::acquireAddress(bool acquire)
{
    wifiManager().resetIfup();
    return wifiManager().acquireAddress(acquire);
}

bool SystemManagerAdaptor::assignStaticAddress(const QString & ip,
                                               const QString & mask,
                                               const QString & gateway,
                                               const QString & dns1,
                                               const QString & dns2)
{
    return wifiManager().assignStaticAddress(ip, mask, gateway, dns1, dns2);
}

void SystemManagerAdaptor::modemNotify(const QString &message,
                                       const QString &vendor,
                                       const QString &product)
{
    // Forward message to 3g manager.
    threeGManager().modemNotify(message, vendor, product);
}

bool SystemManagerAdaptor::isPowerSwitchOn()
{
    return threeGManager().isPowerSwitchOn();
}

bool SystemManagerAdaptor::connect3g(const QString & file, const QString & username, const QString & password)
{
    // Check
    if (!isPowerSwitchOn())
    {
        qDebug("Don't connect as power switch is off.");
        emit pppConnectionChanged("", TG_DISCONNECTED);
        return false;
    }

    threeGManager().enableDevice(true);
    if (threeGManager().state() == TG_INVALID ||
        threeGManager().state() == TG_DISCONNECTED ||
        threeGManager().state() == TG_STOP)
    {
        qDebug("Connecting...");
        power_manager_.enableIdle(false);
        threeGManager().changeState(TG_CHECKING_NETWORK);
        emit pppConnectionChanged("", TG_CHECKING_NETWORK);
        threeGManager().updateSignal(true);
        changeUSBState(USB_3G_HOST);
        return threeGManager().setOptions(file, username, password);
    }
    if (threeGManager().state() == TG_CONNECTED)
    {
        qDebug("Connected.");
        emit pppConnectionChanged("", TG_CONNECTED);
        threeGManager().updateSignal(true);
        return true;
    }
    if (threeGManager().state() == TG_CONNECTING)
    {
        qDebug("In connecting.");
        emit pppConnectionChanged("", TG_CONNECTING);
        threeGManager().updateSignal(true);
        return true;
    }
    return true;
}

void SystemManagerAdaptor::disconnect3g()
{
    emit pppConnectionChanged("", TG_DISCONNECTED);
    threeGManager().stop();
    stopLedBlinking();
    power_manager_.enableIdle(true);
}

bool SystemManagerAdaptor::report3GNetworkSignal(int strength, int max, int type)
{
    emit signalStrengthChanged(strength, 5, type);
    return true;
}

void SystemManagerAdaptor::report3GPowerSwitch(bool on)
{
    threeGManager().report3GPowerSwitch(on);
    if (on)
    {
        connect3g();
        report3GNetworkSignal(0, 0, 0);
    }
    else
    {
        disconnect3g();
        report3GNetworkSignal(-1, 0, 0);
    }
}

bool SystemManagerAdaptor::setGrayScale(int colors)
{
    return screen_manager_->setGrayScale(colors);
}

int SystemManagerAdaptor::grayScale()
{
    return screen_manager_->grayScale();
}

void SystemManagerAdaptor::resetTouchScreen()
{
    TouchScreenManager::instance().resetTouchScreen(gpio_);
    // TouchScreenManager::instance().checkDigitizerType();
    TouchScreenManager::instance().turnOn(gpio_);
}

bool SystemManagerAdaptor::hasTouchScreen()
{
    return TouchScreenManager::instance().hasTouchScreen();
}

void SystemManagerAdaptor::reportUSBConnectionChanged(bool connected)
{
    emit USBConnectionChanged(connected);
    emit connectToPC(connected);
    emit mountTreeSignal(true, LIBRARY_ROOT);
}

/// TODO, add led later.
void SystemManagerAdaptor::setSystemBusy(bool busy,
                                         bool show_indicator)
{
    enableInput(!busy);
    screen_manager_->setBusy(busy, show_indicator);
}

void SystemManagerAdaptor::reportDownloadState(const QString &path,
                                               int percentage,
                                               bool open)
{
    emit downloadStateChanged(path, percentage, open);
}

void SystemManagerAdaptor::snapshot(const QString &path)
{
    screen_manager_->snapshot(path);
}

void SystemManagerAdaptor::downloadFinished(const QString & url,
                                            int state)
{
    qDebug("report download finished signal now.");
    emit downloadStateChanged(url, state, false);
}

void SystemManagerAdaptor::triggerOnlineService()
{
    qDebug("Trigger online service.");
    emit onlineServiceSignal();
}

void SystemManagerAdaptor::refreshScreen()
{
#ifdef BUILD_FOR_ARM
    QWSServer::instance()->refresh();
#endif
    screen_manager_->refreshScreen(onyx::screen::ScreenProxy::GC);
}

void SystemManagerAdaptor::screenSaverActivate(bool activate)
{
#ifdef BUILD_FOR_ARM
    QWSServer::instance()->screenSaverActivate(activate);
#endif
}

void SystemManagerAdaptor::powerOff()
{
    qDebug("system manager power off");
    // Wi-Fi
    stopWpaSupplicant();

    // Turn off touch screen.
    // TouchScreenManager::instance().enableTouchScreen(false);

    // Turn off epson controller to avoid abnormal screen lines.
    screen_manager_->shutdown();

    // 3G manager
    threeGManager().shutdown();

    // Request system power off.
    power_manager_.shutdown();
}

