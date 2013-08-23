

#include "3G_reporter.h"

static const QString PREFIX  = "/dev/ttyUSB";
static const int MAX = 5;
static const int IMEI_LENGTH = 15;

static QString findDevice(const int index)
{
    // Try the name at first.
    QString name = PREFIX;
    for(int i = index; i < index + 10; ++i)
    {
        QString tmp("%1%2");
        tmp = tmp.arg(PREFIX).arg(i);
        if (QFile::exists(tmp))
        {
            return tmp;
        }
    }

    name = name.append(QString::number(index));
    return name;
}

struct MapItem
{
    QString vendor_id;
    QString product_id;
    QString provider;
    int signal_channel;
    int chat_channel;
    bool request_imei;
    QString tag;
};

static const QString HUAWEI = "12d1";
static const QString ZTE = "19d2";
static const MapItem s_maps[] = 
{
    {ZTE, "0117", "ZTE", 1, 2, true, "MF200"},
    {ZTE, "0016", "ZTE", 1, 2, true, "MF210"},
    {ZTE, "0094", "ZTE", 2, 0, true, "AC200"},
    {ZTE, "ffed", "ZTE", 1, 0, false, "MC2716"},
    {HUAWEI, "1001", "HUAWEI", 2, 0, true, "EM730"},
};
static const int MAP_SIZE = sizeof(s_maps)/sizeof(s_maps[0]);



ModemMap::ModemMap(const QString & vendor_id, const QString & product_id)
: vid_(vendor_id)
, pid_(product_id)
, require_imei_(true)
{
    if (vid_.isEmpty() || pid_.isEmpty())
    {
        detectModem(vid_, pid_);
    }

    for(int i = 0; i < MAP_SIZE; ++i)
    {
        if (s_maps[i].product_id == pid_ && s_maps[i].vendor_id == vid_)
        {
            signal_channel_ = findDevice(s_maps[i].signal_channel);
            chat_channel_ = findDevice(s_maps[i].chat_channel);
            require_imei_ = s_maps[i].request_imei;

            QFile::remove("/dev/chatChannel");
            QFile::link(chat_channel_, "/dev/chatChannel");

            QFile::remove("/dev/atChannel");
            QFile::link(signal_channel_, "/dev/atChannel");

            break;
        }
    }
}

ModemMap::~ModemMap()
{
}

bool ModemMap::detectModem(QString &vid, QString & pid)
{
    QProcess script;
    script.setEnvironment(QProcess::systemEnvironment());
    script.start("lsusb", QStringList());
    if (!script.waitForStarted())
    {
        qDebug("Could not start lsusb");
        return false;
    }

    if (!script.waitForFinished(2000))
    {
        qDebug("lsusb failed");
        return false;
    }
    QByteArray output = script.readAll();
    int huawei = output.indexOf(HUAWEI);
    int zte = output.indexOf(ZTE);
    if (huawei > 0)
    {
        vid = HUAWEI;
        pid = output.mid(huawei + 5, 4);
        qDebug("huawei vid %s pid %s", qPrintable(vid), qPrintable(pid));
        return true;
    }
    else if (zte > 0)
    {
        vid = ZTE;
        pid = output.mid(zte + 5, 4);
        qDebug("zte vid %s pid %s", qPrintable(vid), qPrintable(pid));
        return true;
    }
    return false;
}

QString ModemMap::provider()
{
    if (vid_ == HUAWEI)
    {
        return "HUAWEI";
    }
    else if (vid_ == ZTE)
    {
        return "ZTE";
    }
    return "";
}

QString ModemMap::signalChannel()
{
    return signal_channel_;
}

QString ModemMap::chatChannel()
{
    return chat_channel_;
}

bool ModemMap::requireImei()
{
    return require_imei_;
}


// Report interface.
Reporter::Reporter( ModemMap & mm)
: mm_(mm)
{
}

Reporter::~Reporter()
{
}

bool Reporter::sendCommand(SerialPort &sp, QByteArray &data, int timeout)
{
    sp.write(data);

    data.clear();
    QTime t;
    t.start();
    while (t.elapsed() < timeout)
    {
        if (sp.waitForReadyRead(1000))
        {
            QByteArray d = sp.read();
            data.append(d);
            if (data.contains("OK") || data.contains("ERROR"))
            {
                break;
            }
        }
        else
        {
#ifndef _WINDOWS
            ::usleep(10 * 1000);
#endif
        }
    }
    return data.contains("OK");
}


// HuaWeiReport Implementation.
HuaWeiReport::HuaWeiReport(ModemMap & mm)
: Reporter(mm)
{
}

HuaWeiReport::~HuaWeiReport()
{
}

/// Need to scan from AT_device and AT_device + 1...
bool HuaWeiReport::network(int& signal,
                           int &total,
                           int &network)
{
    // Try the name at first.
    SerialPort sp(mm_.signalChannel().toAscii());

    int rssi = 0, ber = 0, type;
    querySignal(sp, rssi, ber);
    queryNetworkType(sp, type);
    signal = toSignal(rssi);
    total = MAX;
    network = type;
    return true;
}

bool HuaWeiReport::changeMode(OperationMode mode)
{
    SerialPort sp(mm_.signalChannel().toAscii());
    return changeMode(sp, mode);
}

bool HuaWeiReport::needPincode()
{
    SerialPort sp(mm_.signalChannel().toAscii());
    return needPincode(sp);
}

bool HuaWeiReport::unlockSimcard(const QString &pin)
{
    return false;
}

QString HuaWeiReport::imei()
{
    if (mm_.requireImei())
    {
        SerialPort sp(mm_.signalChannel().toAscii());
        return queryImei(sp);
    }
    return "empty_imei";
}

QString HuaWeiReport::simCardId()
{
    SerialPort sp(mm_.signalChannel().toAscii());
    return querySimCardId(sp);
}

bool HuaWeiReport::querySignal(SerialPort &sp, int & rssi, int & ber)
{
    QByteArray data("AT+CSQ\r");
    sendCommand(sp, data);
    rssi = 0;
    ber = 0;
    int index = data.lastIndexOf("+CSQ: ");
    if (index < 0)
    {
        return false;
    }
    data = data.mid(index);
    sscanf(data.constData(), "+CSQ: %d,%d", &rssi, &ber);
    return true;
}

bool HuaWeiReport::queryNetworkType(SerialPort &sp, int & type)
{
    QByteArray data("AT^SYSINFO\r\n");
    sendCommand(sp, data);
    int index = data.lastIndexOf("^SYSINFO");
    if (index < 0)
    {
        return false;
    }
    data = data.mid(index);
    int srv_status, srv_domain, roam_status, sys_mode, sim_state, sys_submode;
    sscanf(data.constData(),"^SYSINFO:%d,%d,%d,%d,%d,,%d", &srv_status, &srv_domain,
           &roam_status, &sys_mode, &sim_state, &sys_submode);
    type = sys_submode;
    return true;
}

bool HuaWeiReport::changeMode(SerialPort &sp, OperationMode mode)
{
    QByteArray data("AT+CFUN=");
    QByteArray d;
    d.setNum(mode);
    data.append(d);
    data.append("\r\n");
    return sendCommand(sp, data);
}

bool HuaWeiReport::needPincode(SerialPort &sp)
{
    QByteArray data("AT+CPIN?\r\n");
    if (sendCommand(sp, data))
    {
        // qDebug("data return %s", data.constData());
        if (data.contains("SIM"))
        {
            qDebug("PIN code is required.");
            return true;
        }
        else
        {
            qDebug("PIN code is ignored.");
            return false;
        }
    }
    return true;
}

QString HuaWeiReport::queryImei(SerialPort &sp)
{
    QByteArray data("AT+CGSN\r\n");
    if (sendCommand(sp, data))
    {
        int start = data.indexOf("AT+CGSN"), end = 0;
        while (start < data.size() && start >= 0)
        {
            for(; start < data.size(); ++start)
            {
                if (isdigit(data.at(start)))
                {
                    break;
                }
            }
            if (start >= data.size())
            {
                return false;
            }
            for(end = start + 1; end < start + IMEI_LENGTH && end < data.size(); ++end)
            {
                if (!isdigit(data.at(end)))
                {
                    break;
                }
            }
            if (end - start == IMEI_LENGTH)
            {
                return QString::fromLocal8Bit(data.constData() + start, IMEI_LENGTH);
            }
            start = end;
        }
    }
    return QString();
}

QString HuaWeiReport::querySimCardId(SerialPort &sp)
{
    QByteArray string("AT+CIMI");
    QByteArray data(string);
    data.append("\r\n");
    if (sendCommand(sp, data))
    {
        int pos = data.indexOf(string);
        if (pos < 0)
        {
            return false;
        }
        data = data.mid(pos + string.length() + 1);
        pos = data.indexOf("\n");
        return data.left(pos);
    }
    return "";
}

int HuaWeiReport::toSignal(int rssi)
{
    if (rssi > 31 || rssi < 3)
    {
        return 0;
    }
    else if (rssi >= 3 && rssi < 7)
    {
        return 1;
    }
    else if (rssi >= 7 && rssi < 10)
    {
        return 2;
    }
    else if (rssi >= 10 && rssi < 13)
    {
        return 3;
    }
    else if (rssi >= 13 && rssi < 16)
    {
        return 4;
    }
    else if (rssi >= 16 && rssi <= 31)
    {
        return MAX;
    }
    return 0;
}

// ZTE implementation.
ZTEReport::ZTEReport(ModemMap &mm)
: Reporter(mm)
{
}

ZTEReport::~ZTEReport()
{
}

/// Need to scan from AT_device and AT_device + 1...
bool ZTEReport::network(int& signal,
                        int &total,
                        int &network)
{
    // Try the name at first.
    SerialPort sp(mm_.signalChannel().toAscii());

    int rssi = 0, ber = 0, type = 0;
    querySignal(sp, rssi, ber);
    queryNetworkType(sp, type);
    signal = toSignal(rssi);
    total = MAX;
    network = type;
    return true;
}

bool ZTEReport::changeMode(OperationMode mode)
{
    SerialPort sp(mm_.signalChannel().toAscii());
    return changeMode(sp, mode);
}

bool ZTEReport::needPincode()
{
    SerialPort sp(mm_.signalChannel().toAscii());
    return needPincode(sp);
}

bool ZTEReport::unlockSimcard(const QString &pin)
{
    SerialPort sp(mm_.signalChannel().toAscii());
    return unlockSimcard(sp, pin);
}

QString ZTEReport::imei()
{
    if (mm_.requireImei())
    {
        SerialPort sp(mm_.signalChannel().toAscii());
        return queryImei(sp);
    }
    return "empty_imei";
}

QString ZTEReport::simCardId()
{
    SerialPort sp(mm_.signalChannel().toAscii());
    return querySimCardId(sp);
}

bool ZTEReport::querySignal(SerialPort &sp, int & rssi, int & ber)
{
    QByteArray data("AT+CSQ\r");
    sendCommand(sp, data);
    rssi = 0;
    ber = 0;
    int index = data.lastIndexOf("+CSQ: ");
    if (index < 0)
    {
        return false;
    }
    data = data.mid(index);
    sscanf(data.constData(), "+CSQ: %d,%d", &rssi, &ber);
    return true;
}

bool ZTEReport::queryNetworkType(SerialPort &sp, int & type)
{
    type = NO_SERVICE;
    QByteArray data("AT+ZPAS?\r\n");
    QByteArray target = "ZPAS:";
    if (mm_.pid().compare("ffed", Qt::CaseInsensitive) == 0)
    {
        data = "AT+ZPASR?\r\n";
        target = "ZPASR:";
    }
    sendCommand(sp, data);
    int index = data.lastIndexOf(target);
    if (index < 0)
    {
        // Ignore network checking.
        type = HSDPA_MODE;
        return true;
        /*
        if (target.contains("ZPASR"))
        {
            type = HSDPA_MODE;
            return true;
        }
        return false;
        */
    }
    data = data.mid(index);
    if (data.contains("UMTS"))
    {
        type = HSDPA_MODE;
    }
    else if (data.contains("EDGE"))
    {
        type = EDGE_MODE;
    }
    else if (data.contains("GPRS"))
    {
        type = GPRS_MODE;
    }
    else if (data.contains("GSM"))
    {
        type = GSM_MODE;
    }
    else if (data.contains("HSDPA"))
    {
        type = HSDPA_MODE;
    }
    else if (data.contains("HSUPA"))
    {
        type = HSUPA_MODE;
    }
    else if (data.contains("EVDO"))
    {
        type = HSDPA_MODE;
    }
    else if (data.contains("CDMA"))
    {
        type = HSDPA_MODE;
    }
    else
    {
        type = NO_SERVICE;
    }
    return true;
}

bool ZTEReport::changeMode(SerialPort &sp, OperationMode mode)
{
    QByteArray data("AT+CFUN=");
    QByteArray d;
    d.setNum(mode);
    data.append(d);
    data.append("\r\n");
    return sendCommand(sp, data);
}

bool ZTEReport::needPincode(SerialPort &sp)
{
    QByteArray data("AT+CPIN?\r\n");
    if (sendCommand(sp, data))
    {
        // qDebug("data return %s", data.constData());
        if (data.contains("READY"))
        {
            qDebug("PIN code is ignored.");
            return false;
        }
        else if (data.contains("BUSY"))
        {
            qDebug("SIM card is busy.");
            return false;
        }
        else
        {
            qDebug("PIN code is required.");
            return true;
        }
    }
    return true;
}

bool ZTEReport::unlockSimcard(SerialPort &sp,
                              const QString &pin)
{
    QByteArray data("AT+CPIN=");
    data.append(pin.toAscii());
    data.append("\r\n");
    if (sendCommand(sp, data))
    {
        // qDebug("data return %s", data.constData());
        if (data.contains("OK"))
        {
            qDebug("PIN code unlock successfully.");
            return true;
        }
    }
    return false;
}

QString ZTEReport::queryImei(SerialPort &sp)
{
    QByteArray data("AT+CGSN\r\n");
    if (sendCommand(sp, data))
    {
        int start = data.indexOf("AT+CGSN"), end = 0;
        while (start < data.size() && start >= 0)
        {
            for(; start < data.size(); ++start)
            {
                if (isdigit(data.at(start)))
                {
                    break;
                }
            }
            if (start >= data.size())
            {
                return false;
            }
            for(end = start + 1; end < start + IMEI_LENGTH && end < data.size(); ++end)
            {
                if (!isdigit(data.at(end)))
                {
                    break;
                }
            }
            if (end - start == IMEI_LENGTH)
            {
                return QString::fromLocal8Bit(data.constData() + start, IMEI_LENGTH);
            }
            start = end;
        }
    }
    return QString();
}

QString ZTEReport::querySimCardId(SerialPort &sp)
{
    return "";
}


int ZTEReport::toSignal(int rssi)
{
    if (rssi > 31 || rssi < 3)
    {
        return 0;
    }
    else if (rssi >= 3 && rssi < 7)
    {
        return 1;
    }
    else if (rssi >= 7 && rssi < 10)
    {
        return 2;
    }
    else if (rssi >= 10 && rssi < 13)
    {
        return 3;
    }
    else if (rssi >= 13 && rssi < 16)
    {
        return 4;
    }
    else if (rssi >= 16 && rssi <= 31)
    {
        return MAX;
    }
    return 0;
}

