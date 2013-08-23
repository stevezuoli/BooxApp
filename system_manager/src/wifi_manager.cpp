#include "wifi_manager.h"

/// Wifi manager.
WifiManager::WifiManager(Gpio & gpio)
: gpio_(gpio)
, enabled_(false)
, ifup_count_(0)
, acquire_address_(false)
{
    // Setup connections.
    connect(&ifup_, SIGNAL(finished(int, QProcess::ExitStatus)),
        this, SLOT(onIfupTerminated(int, QProcess::ExitStatus)));
}

WifiManager::~WifiManager()
{
}

void WifiManager::enableDevice(bool enable)
{
    // After we write the value, we should change the direction,
    // otherwise CPU may not receive the interrupt.
    // When disable the device, we don't change the direction so
    // that cpu should never receive the interrupt.
    qDebug("Enable device %d", enable);
    if (enable)
    {
        if (enabled_ == false)
        {
            gpio_.setValue(WIFI_GROUP, WIFI_PIN, 0);
        }
        gpio_.setValue(WIFI_GROUP, WIFI_PIN, 1);
        gpio_.value(WIFI_GROUP, WIFI_PIN);
    }
    else
    {
        gpio_.setValue(WIFI_GROUP, WIFI_PIN, 0);
    }
    enabled_ = enable;
}

bool WifiManager::isEnabled()
{
    return enabled_;
}

bool WifiManager::isWpaSupplicantRunning()
{
    ensureInterfaceUp();

    if (wpa_supplicant_.state() == QProcess::Running)
    {
        qDebug("wpa is running...");
        return true;
    }
    qDebug("wpa not running.");
    return false;
}

/// Start or restart wpa_supplicant.
/// TODO, when we support more network connections, these functions will
/// be placed in network manager.
bool WifiManager::startWpaSupplicant(const QString &path)
{
    qDebug("Start wpa_supplicant.");
    stopWpaSupplicant();

    // Ensure we load the necessary kernel modules.
    loadKernelModules(true);

    // Make sure the system dbus daemon is running.
    // use dbus-daemon --system --print-address.
    // Also make sure the /etc/dbus-1/system.conf defines the correct security policy.
    QString cmd("wpa_supplicant -i eth0 -D wext -C /var/run/wpa_supplicant -c %1");
    if (!path.isEmpty() && QFile::exists(path))
    {
        cmd = cmd.arg(path);
    }
    else
    {
        cmd = cmd.arg(" /etc/wpa.conf ");
    }
    wpa_supplicant_.setEnvironment(QProcess::systemEnvironment());
    qDebug("start wpa %s", qPrintable(cmd));
    wpa_supplicant_.start(cmd);

    if (wpa_supplicant_.waitForStarted(2000))
    {
        return true;
    }
    return false;
}

/// Stop wpa_supplicant daemon.
void WifiManager::stopWpaSupplicant()
{
    // If running, just kill the process.
    if (wpa_supplicant_.state() == QProcess::Running)
    {
        wpa_supplicant_.kill();
        wpa_supplicant_.waitForFinished(1000);
    }
}

bool WifiManager::acquireAddress(bool acquire)
{
    acquire_address_ = acquire;
    QString cmd("acquire_address.sh %1");
    if (acquire)
    {
        cmd = cmd.arg("acquire");
    }
    else
    {
        cmd = cmd.arg("release");
    }
    qDebug("cmd %s", qPrintable(cmd));
    ifup_.start(cmd);
    return ifup_.waitForStarted(2000);
}

bool WifiManager::assignStaticAddress(const QString & ip,
                                      const QString & mask,
                                      const QString & gateway,
                                      const QString & dns1,
                                      const QString & dns2)
{
    return true;
}

void WifiManager::onIfupTerminated(int exit_code,
                                   QProcess::ExitStatus exit_status)
{
    // Check the return value
    qDebug("acquire_address.sh exit_code %d ", exit_code);
    if (exit_code != 0)
    {
        if (shouldIfupRetry())
        {
            qDebug("Try to re-acquire address.");
            increaseIfup();
            acquireAddress(true);
        }
        else
        {
            qDebug("Don't retry now.");
            emit addressAcquired(false);
        }
    }
    else if (exit_code == 0 && acquire_address_)
    {
        qDebug("Acquired address.");
        emit addressAcquired(true);
    }
}

bool WifiManager::shouldIfupRetry()
{
    return false;
    /*
    if (ifup_count_ <= 2 && acquire_address_)
    {
        return true;
    }
    return false;
    */
}

// Ensure network interface is up.
bool WifiManager::ensureInterfaceUp()
{
    // Sometimes, the interface could be down
    // So ensure it's up
    const int TIMEOUT = 1500;
    QProcess loader;
    QStringList parameters;
    loader.start("bringup_interface.sh", parameters);
    return loader.waitForFinished(TIMEOUT);
}

/// Load or unload kernel modules that are needded.
bool WifiManager::loadKernelModules(bool load)
{
    const int TIMEOUT = 4000;
    QProcess loader;
    QStringList parameters;
    loader.start("load_wifi_modules.sh", parameters);
    return loader.waitForFinished(TIMEOUT);
}

