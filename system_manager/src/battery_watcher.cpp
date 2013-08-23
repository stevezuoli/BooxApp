// -*- mode: c++; c-basic-offset: 4; -*-
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "onyx/base/device.h"
#include "battery_watcher.h"

static const int VOLTAGE_WARNING   = 3390;  ///< Warning, charge please.
static const int VOLTAGE_ALARM     = 3350;  ///< Should charge immediately.
static const int VOLTAGE_DANGEROUS = 3300;  ///< According to feedback from battery vendor.

static const int VOLTAGE_MAX = 4209;
static const int VOLTAGE_MIN = VOLTAGE_DANGEROUS;
static const int VOLTAGE_RANGE = VOLTAGE_MAX - VOLTAGE_MIN;

BatteryWatcher::BatteryWatcher(Gpio & gpio)
: QObject(0)
, battery_fd_(0)
, gpio_(gpio)
, voltage_(0)
, left_(100)
, status_(BATTERY_STATUS_NORMAL)
, usb_cable_connected_(false)
, minutes_(0)
, timer_(this)
{
    if (createBatteryWatcher())
    {
        update();

        // TODO: The activated signal is always emitted.
        // watcher_.reset(new QSocketNotifier(battery_fd_, QSocketNotifier::Read));
        // connect(watcher_.get(), SIGNAL(activated(int)), this, SLOT(onStateChanged(int)));

        // So far, have to use timer. TODO, not sure the best interval.
        connect(&timer_, SIGNAL(timeout(void)), this, SLOT(onTimeout(void)));
        timer_.start(60 * 1000);

        // By default, we start charging the button cell.
        chargeButtonCell(true);
    }
}

BatteryWatcher::~BatteryWatcher()
{
    if (battery_fd_)
    {
        fclose(battery_fd_);
    }
}

bool BatteryWatcher::createBatteryWatcher()
{
    battery_fd_ = fopen("/proc/driver/bq27510", "r");
    if (battery_fd_ == 0)
    {
        qWarning("Can't open battery device bq27510!");
    }
    return true;
}

void BatteryWatcher::onTimeout()
{
    const int MAX = 2 * 60;
    update();

    // Check if we can disable button cell charging now.
    ++minutes_;
    if ((minutes_ % 10 == 0) && isButtonCellCharging())
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

void BatteryWatcher::update()
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
bool BatteryWatcher::batteryStatus(int& current, int & status)
{
    static const int BUFFER_SIZE = 1024;

    // If we can not get battery status, we always return 100% to make
    // the device continue working.
    current = 100;
    status = BATTERY_STATUS_NORMAL;
    if (battery_fd_ <= 0)
    {
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

    bool charging = usb_cable_connected_;
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
    return true;
}

/// It's used to update the battery status. I'm not sure the battery driver
/// can notify caller or not, by using the slot, we can always update battery
/// status even the battery driver does not work.
void BatteryWatcher::usbCableChanged(bool connected)
{
    if (usb_cable_connected_ != connected)
    {
        usb_cable_connected_ = connected;
        update();
    }

    // Charge button cell if usb connected.
    if (usb_cable_connected_ && !isButtonCellCharging())
    {
        chargeButtonCell(true);
    }
}

bool BatteryWatcher::chargeButtonCell(bool charge)
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
