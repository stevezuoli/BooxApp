#ifndef UDEV_WATCHER_H_
#define UDEV_WATCHER_H_

#include "onyx/base/base.h"
#include <QtCore/QtCore>

class UdevWatcher : public QObject
{
    Q_OBJECT
public:
    UdevWatcher();
    ~UdevWatcher();

public Q_SLOTS:

    bool isUSBMounted();
    bool isSDMounted();

Q_SIGNALS:
    void USBChanged(bool mounted, const QString &mount_point);
    void SDChanged(bool mounted, const QString &mount_point);

private Q_SLOTS:
    void onKernelEvent(int);
    void onUdevEvent(int);

private:
    bool createWatcher();
    int init_udev_monitor_socket(void);
    int init_uevent_netlink_sock(void);

    bool checkSD();
    bool checkUSB();

    bool mountSD(bool mount);
    bool mountUSB(bool mount);

private:
    int uevent_netlink_sock_;   ///< Kernel event.
    int udev_monitor_sock_;     ///< Udev sends out after rule processing.
    scoped_ptr<QSocketNotifier> uevent_notifier_;
    scoped_ptr<QSocketNotifier> udev_notifier_;

    bool is_sd_mounted_;
    bool is_usb_mounted_;
};

#endif // UDEV_WATCHER_H_
