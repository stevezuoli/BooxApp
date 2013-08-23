#include "3G_manager.h"
#include "onyx/base/device.h"
#include "onyx/data/network_types.h"
#include "onyx/sys/sys_conf.h"
#include "onyx/sys/sys_utils.h"


static const char* pppd_exit_code[] =
{
    "EXIT_OK",
    "EXIT_FATAL_ERROR",
    "EXIT_OPTION_ERROR",
    "EXIT_NOT_ROOT",
    "EXIT_NO_KERNEL_SUPPORT",
    "EXIT_USER_REQUEST",
    "EXIT_LOCK_FAILED",
    "EXIT_OPEN_FAILED",
    "EXIT_CONNECT_FAILED",
    "EXIT_PTYCMD_FAILED",
    "EXIT_NEGOTIATION_FAILED",
    "EXIT_PEER_AUTH_FAILED",
    "EXIT_IDLE_TIMEOUT",
    "EXIT_CONNECT_TIME",
    "EXIT_CALLBACK",
    "EXIT_PEER_DEAD",
    "EXIT_HANGUP",
    "EXIT_LOOPBACK",
    "EXIT_INIT_FAILED",
    "EXIT_AUTH_TOPEER_FAILED",
    "EXIT_TRAFFIC_LIMIT",
    "EXIT_CNID_AUTH_FAILED",
};
static const int SIZE = sizeof(pppd_exit_code)/sizeof(pppd_exit_code[0]);
static const int EXIT_USER_REQUEST = 5;

/// 3G manager.
ThreeGManager::ThreeGManager(Gpio & gpio)
: gpio_(gpio)
, enabled_(false)
, pppd_retry_count_(0)
, state_(TG_INVALID)
, need_pin_code_(true)
{
}

ThreeGManager::~ThreeGManager()
{
}

void ThreeGManager::enableDevice(bool enable)
{
    // TODO: Not sure, we need to reset the 3g chip or not.
    enabled_ = enable;
    if (enable)
    {
        // gpio_.setValue(THREEG_GROUP, THREEG_USB_PIN, 0);
        qDebug("Enable 3G power now.");
        gpio_.setValue(THREEG_GROUP, THREEG_LDO, 1);
        gpio_.mySleep(25 * 1000);
        gpio_.setValue(THREEG_GROUP, THREEG_PIN, 1);
    }
    else
    {
        // gpio_.setValue(THREEG_GROUP, THREEG_USB_PIN, 1);
        qDebug("Disable 3G power now.");
        gpio_.setValue(THREEG_GROUP, THREEG_PIN, 0);
        gpio_.setValue(THREEG_GROUP, THREEG_LDO, 0);
    }
}

bool ThreeGManager::isEnabled()
{
    return enabled_;
}

void ThreeGManager::updateSignal(bool broadcast)
{
    int signal, total, type;
    readSignal(signal, total, type, broadcast);
}

int ThreeGManager::readSignal(int &signal, int &total, int &type, bool broadcast)
{
    reporter().network(signal, total, type);
    if (isEnabled() && broadcast)
    {
        emit signalStrengthChanged(signal, total, type);
    }
    return 0;
}

void ThreeGManager::onModemNotFound(const QString &message)
{
    clear();
    changeState(TG_DISCONNECTED);

    updateSignal();
    emit pppConnectionChanged(message, TG_STOP);
}

void ThreeGManager::onModemDetected(const QString &message)
{
    emit pppConnectionChanged(message, TG_CHECKING_NETWORK);
    changeState(TG_CHECKING_NETWORK);
    clear();
    checkNetwork();
}

void ThreeGManager::checkNetwork()
{
    const int TIME = 2000;
    if (canCheckNetworkRetry())
    {
        if (checkImei().isEmpty())
        {
            qDebug("No imei found, retry %d.", network_retry_count_);
            network_retry_count_ += 5;
            QTimer::singleShot(TIME, this, SLOT(checkNetwork()));
            return;
        }

        if (shouldCheckPincode())
        {
            if (reporter().needPincode())
            {
                if (!reporter().unlockSimcard(defaultPincode()))
                {
                    // TODO: broadcast signal later.
                    qDebug("SIM card error, retry %d.", network_retry_count_);
                    network_retry_count_ += 60;
                }
                QTimer::singleShot(TIME, this, SLOT(checkNetwork()));
                return;
            }
        }
        else
        {
            qDebug("Pin code is ignored by profile.");
        }

        int signal, total, type = 0;
        readSignal(signal, total, type, false);
        if (type != 0)
        {
            qDebug("Network found %d", type);
            QTimer::singleShot(500, this, SLOT(onNetworkFound()));
            return;
        }

        qDebug("Network is not found, retry %d.", network_retry_count_);
        increaseCheckNetworkRetry();
        QTimer::singleShot(TIME, this, SLOT(checkNetwork()));

        // Broadcast state
        emit pppConnectionChanged("", TG_CHECKING_NETWORK);
        changeState(TG_CHECKING_NETWORK);

        return;
    }
    emit pppConnectionChanged("", TG_DISCONNECTED);
}

void ThreeGManager::onIpUp(const QString &message)
{
    resetPppdRetry();
    if (!checkNetworkAddress())
    {
        qDebug("Invalid DNS found, restart pppd.");
        cleanupPppd();
        return;
    }

    changeState(TG_CONNECTED);

    updateSignal(true);
    emit pppConnectionChanged(message, TG_CONNECTED);
}

void ThreeGManager::onPppdExit(const QString &message, const QString & v)
{
    int exitCode = v.toInt();
    qDebug("pppd exit with %s", errStr(exitCode));

    // Check if we have already got retry limit
    if (!canPppdRetry())
    {
        changeState(TG_STOP);
        emit pppdExit(exitCode);
        return;
    }

    increasePppdRetry();
    connect();
}

void ThreeGManager::onNetworkFound()
{
    qDebug("Network found now, start pppd to connect.");
    connect();
}

/// To receive modem notification.
void ThreeGManager::modemNotify(const QString & message,
                                const QString & vendor,
                                const QString & product)
{
    qDebug("ThreeGManager::modem_notify %s", qPrintable(message));
    if (message.compare("device-not-found", Qt::CaseInsensitive) == 0)
    {
        onModemNotFound(message);
    }
    else if (message.compare("device-detected", Qt::CaseInsensitive) == 0)
    {
        onModemDetected(message);
    }
    else if (message.compare("ip-up", Qt::CaseInsensitive) == 0)
    {
        onIpUp(message);
    }
    else if (message.compare("pppd-exit", Qt::CaseInsensitive) == 0)
    {
        onPppdExit(message, vendor);
    }
}

/// This function is called when 3G power switch has been changed.
void ThreeGManager::report3GPowerSwitch(bool on)
{
    return;
}

bool ThreeGManager::isPowerSwitchOn()
{
    int value = gpio_.value(0, 22);
    qDebug("3G power switch state %d", value);
    return (value == 1);
}

QString ThreeGManager::checkImei()
{
    static QString imei;
    if (imei.isEmpty())
    {
        imei = reporter().imei();
        qDebug("imei %s", qPrintable(imei));
        emit imeiAvailable(imei);
    }
    return imei;
}

void ThreeGManager::updateSignalCallback()
{
    if (!map().signalChannel().isEmpty())
    {
        updateSignal(true);
        QTimer::singleShot(30 * 1000, this, SLOT(updateSignalCallback()));
    }
}


bool ThreeGManager::canPppdRetry()
{
    return (pppd_retry_count_ <= 4 && isEnabled());
}

void ThreeGManager::increasePppdRetry()
{
    ++pppd_retry_count_;
}

void ThreeGManager::resetPppdRetry()
{
    pppd_retry_count_ = 0;
}

void ThreeGManager::cleanupPppd()
{
    qDebug("Terminate pppd block.");
    sys::runScriptBlock("pppd_cleanup.sh", QStringList());
}

bool ThreeGManager::shouldCheckPincode()
{
    return !(qgetenv("IGNORE_PINCODE").toInt() == 1);
}

bool ThreeGManager::canCheckNetworkRetry()
{
    // 2 minutes.
    return (network_retry_count_ <= 120 && isEnabled());
}

void ThreeGManager::resetCheckNetworkRetry()
{
    network_retry_count_ = 0;
}

void ThreeGManager::increaseCheckNetworkRetry()
{
    ++network_retry_count_;
}

bool ThreeGManager::checkNetworkAddress()
{
    QFile file("/etc/resolv.conf");
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    // Do not check now.
    //QByteArray d = file.readAll();
    //if (d.indexOf("10.11.12.13") < 15 ||
    //    d.indexOf("10.11.12.14") < 15)
    //{
    //    return true;
    //}

    return true;
}

// Reset everything.
void ThreeGManager::clear()
{
    // Should not clear peer, as we may use it later.
    // peer_.clear();
    cleanupPppd();
    resetCheckNetworkRetry();
    resetPppdRetry();
    mm_.reset(0);
    reporter_.reset(0);
}

QString ThreeGManager::peer()
{
    if (!peer_.isEmpty())
    {
        return peer_;
    }
    return "default";
}

ModemMap & ThreeGManager::map()
{
    if (!mm_)
    {
        mm_.reset(new ModemMap());
    }
    return *mm_;
}

Reporter & ThreeGManager::reporter()
{
    ModemMap & mm = map();
    if (!reporter_)
    {
        if (mm.provider() == "ZTE")
        {
            reporter_.reset(new ZTEReport(mm));
        }
        else
        {
            reporter_.reset(new HuaWeiReport(mm));
        }
    }
    return *reporter_;
}

void ThreeGManager::changeState(TGConnectionState s)
{
    state_ = s;
}

/// Start to connect to 3G network. It will load all necessary modules
/// and check 3G device. After that, it talks with the 3G device through pppd
/// process. The pppd can report all necessary informtion to system manager.
/// system manager will forward these messages to 3G manager.
bool ThreeGManager::setOptions(const QString& peer,
                               const QString& username,
                               const QString& password)
{
    if (!peer.isEmpty())
    {
        peer_ = peer;
    }
    username_ = username;
    password_ = password;
    resetPppdRetry();
    resetCheckNetworkRetry();
    return true;
}

QString ThreeGManager::defaultPincode()
{
    return sys::SystemConfig::defaultPincode();
}

QString ThreeGManager::chatScript()
{
    if (reporter().needPincode())
    {
        return "/etc/ppp/chat_pincode";
    }
    else
    {
        return "/etc/ppp/chat";
    }
}

// Use singleShot to ensure time sequence is correct.
bool ThreeGManager::connect()
{
    QTimer::singleShot(0, this, SLOT(connectImpl()));
    return true;
}

bool ThreeGManager::connectImpl()
{
    // Construct pppd command line
    // /usr/sbin/pppd connect '/usr/sbin/chat -v -f /tmp/chat-script' /dev/ttyUSB2
    // nodetach crtscts debug usepeerdns defaultroute
    QString cmd = QString("/usr/sbin/pppd call %1\"").arg(peer());
    qDebug("cmd: %s", qPrintable(cmd));
    QProcess::startDetached(qPrintable(cmd));
    emit pppConnectionChanged("", TG_CONNECTING);

    QProcess::startDetached("/usr/bin/query_3g_signal.sh");
    return true;
}

const char* ThreeGManager::errStr(int code)
{
    if (code >= 0 && code < SIZE)
    {
        return pppd_exit_code[code];
    }
    return 0;
}

void ThreeGManager::stop()
{
    clear();
    enableDevice(false);
    changeState(TG_STOP);
    QProcess::startDetached("pppd_cleanup.sh");
}

// Shutdown device and make it as offline.
void ThreeGManager::shutdown()
{
    reporter().changeMode(OFFLINE);

    stop();
    gpio_.setValue(THREEG_GROUP, THREEG_LDO, 0);
    gpio_.setValue(THREEG_GROUP, THREEG_USB_PIN, 1);
    gpio_.setValue(THREEG_GROUP, THREEG_PIN, 0);
}


