#ifndef THREEG_MANAGER_H_
#define THREEG_MANAGER_H_

#include <QtCore/QtCore>

#include "onyx/base/base.h"
#include "onyx/data/network_types.h"

#include "gpio/gpio.h"
#include "3G_reporter.h"

/// 3G manager.
class ThreeGManager : public QObject
{
    Q_OBJECT

public:
    ThreeGManager(Gpio & gpio);
    ~ThreeGManager();

public Q_SLOTS:
    void enableDevice(bool enable);
    bool isEnabled();

    bool setOptions(const QString& chat_file,
                    const QString& username,
                    const QString& password);

    bool connect();
    void stop();
    const char* errStr(int code);

    void modemNotify(const QString &message, const QString &vendor, const QString & product);

    void report3GPowerSwitch(bool on);
    void updateSignal(bool broadcast = true);
    int  readSignal(int &signal, int &total, int &type, bool broadcast = true);
    bool isPowerSwitchOn();

    void changeState(TGConnectionState s);
    TGConnectionState state() { return state_; }
    bool isConnecting() { return state_ == TG_CONNECTING; }

    void shutdown();

public:
    static const int THREEG_GROUP = 0;
    static const int THREEG_PIN = 3;
    static const int THREEG_USB_PIN = 8;
    static const int THREEG_LDO = 21;

Q_SIGNALS:
    void pppdExit(int exitCode);
    void signalStrengthChanged(int strength, int total, int network);
    void pppConnectionChanged(const QString &, int);
    void imeiAvailable(const QString &);

private Q_SLOTS:
    bool needPincode() { return need_pin_code_;}
    QString defaultPincode();
    QString chatScript();

    bool connectImpl();

    // State slot
    void onModemNotFound(const QString &message);
    void onModemDetected(const QString &message);
    void checkNetwork();
    void onIpUp(const QString &message);
    void onPppdExit(const QString &message, const QString &v);
    void onNetworkFound();

    void updateSignalCallback();

private:
    QString checkImei();

    bool canPppdRetry();
    void increasePppdRetry();
    void resetPppdRetry();
    void cleanupPppd();

    bool shouldCheckPincode();

    bool canCheckNetworkRetry();
    void increaseCheckNetworkRetry();
    void resetCheckNetworkRetry();

    bool checkNetworkAddress();

    void clear();
    QString peer();

    ModemMap & map();
    Reporter & reporter();

private:
    Gpio & gpio_;
    bool enabled_;
    QString peer_;
    QString username_;
    QString password_;
    int pppd_retry_count_;
    int network_retry_count_;
    TGConnectionState state_;
    bool need_pin_code_;
    scoped_ptr<ModemMap> mm_;
    scoped_ptr<Reporter> reporter_;

};

#endif // THREEG_MANAGER_H_
