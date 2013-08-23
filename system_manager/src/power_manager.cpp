#ifdef BUILD_FOR_ARM
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/apm_bios.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <linux/rtc.h>
#include <sys/time.h>
#include <time.h>
#endif
#include "power_manager.h"
#include "onyx/sys/sys_conf.h"
#include "i2c/i2c.h"

static const int CPU_FREQUENCY[] = {532, 200, 0};
static int INTERVALS[] = {2000, 1000, 180 *1000 , 0};
static const int LEVELS = sizeof(INTERVALS)/sizeof(INTERVALS[0]) - 1;
static const int IDLE_INDEX = LEVELS - 2;
static const int LAST_INDEX = LEVELS - 1;   /// For deep sleep or shutdown.

static const int DEEP_SLEEP = 1;
static const int SHUTDOWN   = 2;
static const int EPIT_TIMEOUT = 180 * 1000;

const char * RTC_DEVICE = "/dev/rtc0";
static const int BUFFER_SIZE = 1024;

using namespace sys;

/// See system_manager.vsd for details.
PowerManager::PowerManager(Gpio & gpio)
: QObject(0)
, battery_fd_(0)
, gpio_(gpio)
, voltage_(0)
, left_(100)
, status_(BATTERY_STATUS_NORMAL)
, minutes_(0)
, battery_timer_(this)
, type_(0)
, idle_count_(0)
, mxc_pm_interrupts_(0)
, epit_flag_(false)
{
    updateBattery();

    // TODO: The activated signal is always emitted.
    // watcher_.reset(new QSocketNotifier(battery_fd_, QSocketNotifier::Read));
    // connect(watcher_.get(), SIGNAL(activated(int)), this, SLOT(onStateChanged(int)));

    // So far, have to use timer. TODO, not sure the best interval.
    connect(&battery_timer_, SIGNAL(timeout(void)), this, SLOT(onBatteryTimeout(void)));
    battery_timer_.start(60 * 1000);

    // By default, we start charging the button cell.
    chargeButtonCell(true);

    // Load the intervals from system configuration.
    SystemConfig conf;

    int suspend = conf.suspendInterval();
    int shutdown = conf.shutdownInterval();
    INTERVALS[LAST_INDEX] = 0;
    if (suspend > 0)
    {
        INTERVALS[LAST_INDEX] = suspend;
        type_ = DEEP_SLEEP;
    }
    if (shutdown > 0)
    {
        INTERVALS[LAST_INDEX] = shutdown;
        type_ = SHUTDOWN;
    }

#ifdef BUILD_FOR_ARM
    QWSServer::setScreenSaverIntervals(&INTERVALS[0]);
    QWSServer::setScreenSaver(this);

    // Not necessary to block now.
    // QWSServer::setScreenSaverBlockLevel(LEVELS - 1);
#endif
}

PowerManager::~PowerManager()
{
}

/// Change the suspend interval.
bool PowerManager::setSuspendInterval(int ms)
{
    // Change seconds to ms.
    SystemConfig conf;
    conf.setSuspendInterval(ms);

    // A little bit different.
    // seconds -= INTERVALS[IDLE_INDEX];
    INTERVALS[LAST_INDEX] = ms;
#ifdef BUILD_FOR_ARM
    QWSServer::setScreenSaverIntervals(&INTERVALS[0]);
#endif
    if (ms > 0)
    {
        type_ = DEEP_SLEEP;
    }
    else if (type() == DEEP_SLEEP)
    {
        type_ = 0;
    }
    return true;
}

/// Retrieve the suspend interval.
int PowerManager::suspendInterval()
{
    if (type() == DEEP_SLEEP)
    {
        return INTERVALS[LAST_INDEX];
    }
    return 0;
}

bool PowerManager::setShutdownInterval(int ms)
{
    SystemConfig conf;
    conf.setShutdownInterval(ms);

    // More than specified seconds.
    // seconds -= INTERVALS[IDLE_INDEX];
    INTERVALS[LAST_INDEX] = ms;
#ifdef BUILD_FOR_ARM
    QWSServer::setScreenSaverIntervals(&INTERVALS[0]);
#endif
    if (ms > 0)
    {
        type_ = SHUTDOWN;
    }
    else if (type() == SHUTDOWN)
    {
        type_ = 0;
    }
    return true;
}

int  PowerManager::shutdownInterval()
{
    if (type() == SHUTDOWN)
    {
        return INTERVALS[LAST_INDEX];
    }
    return 0;
}

/// This function will shutdown the device. Usually, caller receive the
/// aboutToShutdown signal and caller stores everything into persistent
/// media. After that, caller uses this function to turn the device off.
bool PowerManager::shutdown()
{
    QProcess::startDetached("shutdown_device.sh");
    return true;
}

void PowerManager::writeString(const char *str)
{
#ifdef BUILD_FOR_ARM
    int fd = open("/sys/power/state", O_RDWR);
    write(fd, str, strlen(str));
    close(fd);
#endif
}

/// Deep sleep mode.
bool PowerManager::deepSleep()
{
    enableLdo123(false);
    while (1)
    {
        recordInterrupts();

        writeString("standby");

        if (!wakeupByEPITTimer())
        {
            break;
        }

        if (!checkBattery())
        {
            break;
        }
    }

    enableLdo123(true);
    return true;
}

void PowerManager::onIdle()
{
    monitor_.sample();
}

void PowerManager::resetIdle()
{
    idle_count_ = 0;
}

void PowerManager::enableIdle(bool enable)
{
    if (enable)
    {
        if (--idle_count_ < 0)
        {
            idle_count_ = 0;
        }
    }
    else
    {
        ++idle_count_;
    }
    qDebug("Enable idle %d ", idle_count_);
}

bool PowerManager::idle(bool force)
{
    if (!monitor_.isIdle())
    {
        qDebug("Not idle.");
        return false;
    }

    if (!isIdleEnabled() && !force)
    {
        qDebug("Ilde is disabled.");
        return true;
    }


    if (isUSBConnected())
    {
        return true;
    }

    saveTime();

    // QTime and QDateTime do not work.
    // Using EPIT timer, we can ensure the timeout is 3 minutes: 180 * 1000
    int ms_elapsed = 0;
    while (1)
    {
        recordInterrupts();
        writeString("idle");

        if (!wakeupByEPITTimer())
        {
            break;
        }

        // Check remaining quantity.
        if (!checkBattery())
        {
            break;
        }

        // Check time now.
        ms_elapsed += EPIT_TIMEOUT;
        if (ms_elapsed >= INTERVALS[LAST_INDEX])
        {
            save(LAST_INDEX);
            break;
        }
    }

    return true;
}

void PowerManager::restore()
{
}

// Need to suspend according to the level.
bool PowerManager::save(int level)
{
    bool ret = true;
    if (level == IDLE_INDEX)
    {
        qDebug("System idle signal");
        emit systemIdleSignal(level, ret);
    }
    else if (level == LAST_INDEX)
    {
        if (type() == DEEP_SLEEP)
        {
            qDebug("Broadcast deep sleep signal.");
            emit aboutToSuspend();
        }
        else if (type() == SHUTDOWN)
        {
            qDebug("Broadcast shutdown signal.");
            emit aboutToShutdown(true);
        }
    }
    else
    {
        // qDebug("System short idle level %d after %d", level, INTERVALS[level]);
        emit systemIdleSignal(level, ret);
    }
    return ret;
}

void PowerManager::recordInterrupts()
{
    mxc_pm_interrupts_ = readInterrupts();
}

bool PowerManager::wakeupByEPITTimer()
{
    int now = readInterrupts();
    if (now > mxc_pm_interrupts_)
    {
        syncSystemTimeFromRtc();
        saveTime();
        emit wakeupByEPIT();
        return true;
    }
    else
    {
        saveTime();
        return false;
    }
}

void PowerManager::setEPITTimerInterval(const int seconds)
{
    FILE * fd = fopen("/sys/onyx/epit_timeout", "w");
    if (fd == 0)
    {
        qWarning("Can't open /sys/onyx/epit_timeout!");
        return;
    }
    char temp[12] = {0};
    sprintf(temp, "%d", seconds);
    fputs(temp,fd);
    fclose(fd);
}

int PowerManager::readInterrupts()
{
    int count = -1;
    FILE * fd = fopen("/proc/interrupts", "r");
    if (fd == 0)
    {
        qWarning("Can't open /proc/interrupts!");
        return count;
    }

    while (!feof(fd))
    {
        char buf[BUFFER_SIZE + 1] = {0};
        char ignore[20] = {0};

        fgets(buf, BUFFER_SIZE, fd);
        if (strstr(&buf[0], "mxc_pm") != 0)
        {
            sscanf(buf, "%s %d %s %s", ignore, &count, ignore, ignore);
            break;
        }
    }
    fclose(fd);
    return count;
}

void PowerManager::enableLdo123(bool enable)
{
    printf("Ignore i2c request.\n");
/*
    const int reg = 0x12;
    I2C i2c(0, 0x34);
    unsigned char value = 0;
    i2c.read(reg, value);
    if (enable)
    {
        value |= 0x3e;
    }
    else
    {
        value &= 0x30;
    }
    i2c.writeByte(reg, value);
*/
}

void PowerManager::enableUSBPhy(bool enable)
{
    FILE * count_fd = fopen("/sys/class/regulator/regulator_2_LDO3/enabled_count", "r");
    if (count_fd == 0)
    {
        qWarning("Can't open /sys/class/regulator/regulator_2_LDO3/enabled_count!");
        return;
    }

    int enabled_count = 0;
    fseek(count_fd, 0, SEEK_SET);
    char buf[BUFFER_SIZE + 1] = {0};
    fgets(buf, BUFFER_SIZE, count_fd);
    sscanf(buf, "%d", &enabled_count);

    if (enable)
    {
        writeLP3971Ctrl(1);
    }
    else
    {
        for(int i = 0; i <= enabled_count; ++i)
        {
            writeLP3971Ctrl(0);
        }
    }

    fclose(count_fd);
}

void PowerManager::writeLP3971Ctrl(int value)
{
    FILE* ctl_fd = fopen("/sys/class/regulator/regulator_2_LDO3/ctl", "w");
    if (ctl_fd == 0)
    {
        qWarning("Can't open /sys/class/regulator/regulator_2_LDO3/ctl!");
        return;
    }

    fputc(value, ctl_fd);
    fclose(ctl_fd);
}


static const int VOLTAGE_WARNING   = 3390;  ///< Warning, charge please.
static const int VOLTAGE_ALARM     = 3350;  ///< Should charge immediately.
static const int VOLTAGE_DANGEROUS = 3300;  ///< According to feedback from battery vendor.
static const int VOLTAGE_THRESHOLD = 3250;  ///< Don't update screen when voltage is lower than threshold.

static const int VOLTAGE_MAX = 4209;
static const int VOLTAGE_MIN = VOLTAGE_DANGEROUS;
static const int VOLTAGE_RANGE = VOLTAGE_MAX - VOLTAGE_MIN;

bool PowerManager::createBatteryWatcher()
{
    battery_fd_ = fopen("/proc/driver/bq27510", "r");
    if (battery_fd_ == 0)
    {
        qWarning("Can't open battery device bq27510!");
    }
    return true;
}

/// Return false if battery is in a very dangerous state.
bool PowerManager::checkBattery()
{
    int c = left_;
    int status = status_;
    batteryStatus(c, status);

    if (!isUSBConnected())
    {
        if (voltage() <= VOLTAGE_THRESHOLD)
        {
            emit aboutToShutdown(true);
            return false;
        }
        else if (voltage() <= VOLTAGE_DANGEROUS)
        {
            emit aboutToShutdown(true);
            return false;
        }
    }
    return true;
}

void PowerManager::onBatteryTimeout()
{
    const int MAX = 2 * 60;
    updateBattery();

    // Check if we can disable button cell charging now.
    ++minutes_;
    if ((minutes_ % 100 == 0) && isButtonCellCharging() && !isUSBConnected())
    {
        chargeButtonCell(false);
    }

    // Every 2 hours, we ask it to change the charge state.
    if (isUSBConnected() && (minutes_ % MAX) == 0)
    {
        // Not sure we need sleep.
        qDebug("Update power charge.");
        gpio_.setValue(1, 22, 1);
        gpio_.mySleep(1000 * 500);
        gpio_.setValue(1, 22, 0);
    }

    if (minutes_ >= MAX)
    {
        minutes_ = 0;
    }
}

void PowerManager::updateBattery()
{
    int c = left_;
    int status = status_;
    batteryStatus(c, status);

    // qDebug("voltage %d current %d status %d", voltage_, c, status);

    if (c != left_ || status != status_ ||
        status & BATTERY_STATUS_DANGEROUS)
    {
        left_ = c;
        status_ = status;
        emit batteryChangedSignal(left_, status_);
    }
}

/// Qurey battery status without broadcasting any signal.
bool PowerManager::batteryStatus(int& current, int & status)
{
    // If we can not get battery status, we always return 100% to make
    // the device continue working.
    status = BATTERY_STATUS_NORMAL;
    battery_fd_ = fopen("/proc/driver/bq27510", "r");
    if (battery_fd_ == 0)
    {
        qWarning("Can't open battery device bq27510!");
        return false;
    }

    // Query all data:
    // cat /proc/driver/bq27510
    // Flags: 0x000064b4
    // Voltage: 4218mV
    // Current: -8mA
    // Relative State of Charge: 100%
    // Remaining capacity: 11520mAh
    // Time to Empty: 65535min
    // Time to Full: 0min
    fseek(battery_fd_, 0, SEEK_SET);

    bool charging = isUSBConnected();
    while (!feof(battery_fd_))
    {
        char buf[BUFFER_SIZE + 1] = {0};
        char ignore[20] = {0};

        fgets(buf, BUFFER_SIZE, battery_fd_);
        if (strstr(&buf[0], "Voltage") != 0)
        {
            sscanf(buf, "%s %d", ignore, &voltage_);
            if (voltage_ <= 0 || voltage_ >= VOLTAGE_MAX)
            {
                voltage_ = VOLTAGE_MAX;
            }
            break;
        }
    }

    if (charging)
    {
        // If charging, forget critical.
        status = BATTERY_STATUS_NORMAL | BATTERY_STATUS_CHARGING;
    }

    if (voltage_ <= VOLTAGE_WARNING && voltage_ >= VOLTAGE_ALARM && !charging)
    {
        status = BATTERY_STATUS_WARNING;
    }
    else if (voltage_ <= VOLTAGE_ALARM && voltage_ >= VOLTAGE_DANGEROUS && !charging)
    {
        status = BATTERY_STATUS_ALARM;
    }
    else if (voltage_ <= VOLTAGE_DANGEROUS && !charging)
    {
        status = BATTERY_STATUS_DANGEROUS;
    }

    // Update percentage
    current = (voltage_ - VOLTAGE_MIN) * 100 / (VOLTAGE_RANGE);
    if (current > 100)
    {
        current = 100;
    }
    else if (current < 0)
    {
        current = 0;
    }

    fclose(battery_fd_);
    return true;
}

/// It's used to update the battery status. I'm not sure the battery driver
/// can notify caller or not, by using the slot, we can always update battery
/// status even the battery driver does not work. The gpio event is converted
/// to keyboard event here, we just check usb is connected or not.
void PowerManager::usbCableChanged(bool connected)
{
    // Charge button cell if usb connected.
    if (isUSBConnected())
    {
        chargeButtonCell(true);
    }

    // Need to use a timer here, as when USB cable is disconnected,
    // the bq27510 still needs time to update its state.
    QTimer::singleShot(1500, this, SLOT(updateBattery()));
}

bool PowerManager::isUSBConnected()
{
    return (gpio_.value(1, 18) == 1);
}

/// Check the remaining quantity of electricity is enough to update screen or not.
bool PowerManager::canUpdateScreen()
{
    return (voltage() > VOLTAGE_THRESHOLD || isUSBConnected());
}

bool PowerManager::chargeButtonCell(bool charge)
{
    charge_button_cell_ = charge;
#ifdef BUILD_FOR_ARM
    int fd = open("/sys/class/i2c-adapter/i2c-0/0-0068/nvram", O_RDWR);
    if (fd <= 0)
    {
        qDebug("Could not charge button cell.");
        return false;
    }

    unsigned char value = 0xAA;
    if (!charge)
    {
        value = 0;
    }

    lseek(fd, 8, SEEK_SET);
    write(fd, &value, sizeof(value));
    close(fd);

#endif
    qDebug("chargeButtonCell %d", charge);
    return true;
}

void PowerManager::saveTime()
{
#ifdef BUILD_FOR_ARM

    int fd = open(RTC_DEVICE, O_RDONLY);
    struct rtc_time rtc;
    memset(&rtc, 0, sizeof(struct rtc_time));

    if (ioctl(fd, RTC_RD_TIME, &rtc) < 0)
    {
        fprintf(stderr, "Can't read hw clock from %s!\n", RTC_DEVICE);
        close(fd);
        return;
    }

    gettimeofday(reinterpret_cast<struct timeval *>(&old_), 0);
    prev_ = myMakeTime(rtc.tm_year, rtc.tm_mon, rtc.tm_mday, rtc.tm_hour, rtc.tm_min, rtc.tm_sec);

    close(fd);
#endif
}

unsigned long PowerManager::myMakeTime(const unsigned int year0,
                                       const unsigned int mon0,
                                       const unsigned int day,
                                       const unsigned int hour,
                                       const unsigned int min,
                                       const unsigned int sec)
{
    unsigned int mon = mon0, year = year0;

    /* 1..12 -> 11,12,1..10 */
    if (0 >= (int) (mon -= 2))
    {
        mon += 12; /* Puts Feb last since it has leap day */
        year -= 1;
    }

    return ((((unsigned long)
        (year/4 - year/100 + year/400 + 367*mon/12 + day) +
        year*365 - 719499
        )*24 + hour /* now have hours */
        )*60 + min /* now have minutes */
        )*60 + sec; /* finally seconds */
}

bool PowerManager::syncSystemTimeFromRtc()
{
#ifdef BUILD_FOR_ARM

    const char * RTC_DEVICE = "/dev/rtc0";
    int fd = open(RTC_DEVICE, O_RDONLY);
    struct rtc_time rtc;
    memset(&rtc, 0, sizeof(struct rtc_time));

    if (ioctl(fd, RTC_RD_TIME, &rtc) < 0)
    {
        fprintf(stderr, "Can't read hw clock from %s!\n", RTC_DEVICE);
        close(fd);
        return false;
    }

    // Set system time using rtc
    unsigned long t = myMakeTime(rtc.tm_year, rtc.tm_mon, rtc.tm_mday, rtc.tm_hour, rtc.tm_min, rtc.tm_sec);
    old_.tv_sec += t - prev_;
    old_.tv_usec = 0;
    settimeofday(reinterpret_cast<struct timeval *>(&old_), 0);

    close(fd);

#endif
    return true;
}


