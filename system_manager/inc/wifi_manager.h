#ifndef WIFI_MANAGER_H_
#define WIFI_MANAGER_H_

#include <QtCore/QtCore>
#include "gpio/gpio.h"

/// Wifi manager.
class WifiManager : public QObject
{
    Q_OBJECT
public:
    WifiManager(Gpio & gpio);
    ~WifiManager();

public Q_SLOTS:
    void enableDevice(bool enable);
    bool isEnabled();

    bool isWpaSupplicantRunning();
    bool startWpaSupplicant(const QString & path);
    void stopWpaSupplicant();
    bool acquireAddress(bool acquire);
    bool assignStaticAddress(const QString & ip,
                             const QString & mask,
                             const QString & gateway,
                             const QString & dns1,
                             const QString & dns2);
    void onIfupTerminated(int, QProcess::ExitStatus);
    void resetIfup() { ifup_count_ = 0; }

public:
    static const int WIFI_GROUP = 1;
    static const int WIFI_PIN = 9;

Q_SIGNALS:
    void addressAcquired(bool);

private:
    bool ensureInterfaceUp();
    bool loadKernelModules(bool load);

    bool shouldIfupRetry();
    void increaseIfup() { ++ifup_count_; }

private:
    Gpio & gpio_;
    bool enabled_;
    QProcess wpa_supplicant_;
    int ifup_count_;
    bool acquire_address_;
    QProcess ifup_;
};

#endif // WIFI_MANAGER_H_
