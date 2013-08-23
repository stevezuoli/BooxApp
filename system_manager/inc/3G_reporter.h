
#ifndef ONYX_3G_REPORTER_H_
#define ONYX_3G_REPORTER_H_

#include <QApplication>
#include "onyx/base/dbus.h"
#include "serial_port/sp.h"
#include "onyx/data/network_types.h"


class ModemMap
{
public:
    ModemMap(const QString & vendor_id = QString(), const QString & product_id = QString());
    ~ModemMap();

public:
    bool detectModem(QString &vid, QString & pid);
    QString provider();
    QString signalChannel();
    QString chatChannel();
    bool requireImei();

    QString vid() { return vid_; }
    QString pid() { return pid_; }

private:
    QString vid_;
    QString pid_;
    QString signal_channel_;
    QString chat_channel_;
    bool require_imei_;
};

class Reporter : public QObject
{
    Q_OBJECT

public:
    Reporter(ModemMap & mm);
    ~Reporter();

public:
    virtual bool network(int& signal, int &total, int &network) = 0;
    virtual bool changeMode(OperationMode mode) = 0;
    virtual bool needPincode() = 0;
    virtual bool unlockSimcard(const QString &pin) = 0;
    virtual QString imei() = 0;
    virtual QString simCardId() = 0;


    bool sendCommand(SerialPort &sp, QByteArray &data, int timeout_ms = 300);

protected:
    ModemMap & mm_;
};


class HuaWeiReport : public Reporter
{
    Q_OBJECT
public:
    HuaWeiReport(ModemMap &mm);
    ~HuaWeiReport();

public:
    virtual bool network(int& signal, int &total, int &network);
    virtual bool changeMode(OperationMode mode);
    virtual bool needPincode();
    virtual bool unlockSimcard(const QString &pin);
    virtual QString imei();
    virtual QString simCardId();

private:
    bool querySignal(SerialPort &sp, int & rssi, int & ber);
    bool queryNetworkType(SerialPort &sp, int & type);
    bool changeMode(SerialPort &sp, OperationMode mode);
    bool needPincode(SerialPort &sp);
    QString queryImei(SerialPort &sp);
    QString querySimCardId(SerialPort &sp);

    int toSignal(int);
};


class ZTEReport : public Reporter
{
    Q_OBJECT

public:
    ZTEReport(ModemMap &mm);
    ~ZTEReport();

public:
    virtual bool network(int& signal, int &total, int &network);
    virtual bool changeMode(OperationMode mode);
    virtual bool needPincode();
    virtual bool unlockSimcard(const QString &pin);
    virtual QString imei();
    virtual QString simCardId();

private:
    bool querySignal(SerialPort &sp, int & rssi, int & ber);
    bool queryNetworkType(SerialPort &sp, int & type);
    bool changeMode(SerialPort &sp, OperationMode mode);
    bool needPincode(SerialPort &sp);
    bool unlockSimcard(SerialPort &sp, const QString &pin);
    QString queryImei(SerialPort &sp);
    QString querySimCardId(SerialPort &sp);

    int toSignal(int);
};

#endif

