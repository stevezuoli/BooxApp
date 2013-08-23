#ifndef BATTERY_WATCHER_H_
#define BATTERY_WATCHER_H_

#include "onyx/base/base.h"
#include "onyx/base/device.h"
#include <QtCore/QtCore>
#include <QString>
#include "gpio/gpio.h"

/// Battery event watcher.
class BatteryWatcher : public QObject
{
    Q_OBJECT
public:
    BatteryWatcher(Gpio & gpio);
    ~BatteryWatcher();

public Q_SLOTS:
    bool batteryStatus(int& left, int & status);
    int queryVoltage() { return voltage_; }
    void usbCableChanged(bool connected);
    bool chargeButtonCell(bool charge);
    bool isUSBConnected() { return usb_cable_connected_; }

Q_SIGNALS:
    void batteryChangedSignal(int left, int status);
    void batteryFull();

private:
    bool createBatteryWatcher();

private Q_SLOTS:
    void onTimeout();
    void update();
    bool isButtonCellCharging() { return charge_button_cell_; }

private:
    Gpio & gpio_;
    FILE * battery_fd_;
    scoped_ptr<QSocketNotifier> watcher_;
    int voltage_;
    int left_;      ///< Cached value.
    int status_;
    bool usb_cable_connected_;
    int minutes_;
    bool charge_button_cell_;
    QTimer timer_;
};

#endif // BATTERY_WATCHER_H_
