
#ifdef __linux__
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <linux/types.h>
#include <linux/netlink.h>
#endif

#include "onyx/base/device.h"
#include "udev_watcher.h"

static const int UEVENT_BUFFER_SIZE         = 2048;
static const QString DEV_ADD                = "add";
static const QString DEV_REMOVE             = "remove";
static const QString MMC                    = "mmc";
static const QString USB                    = "scsi_device";

#ifdef BUILD_FOR_ARM

static const char *search_key(const char *searchkey, const char *buf, size_t buflen)
{
    size_t bufpos = 0;
    size_t searchkeylen = strlen(searchkey);

    while (bufpos < buflen) {
        const char *key;
        int keylen;

        key = &buf[bufpos];
        keylen = strlen(key);
        if (keylen == 0)
            break;
        if ((strncmp(searchkey, key, searchkeylen) == 0) && key[searchkeylen] == '=')
            return &key[searchkeylen + 1];
        bufpos += keylen + 1;
    }
    return NULL;
}

#endif  // BUILD_FOR_ARM

UdevWatcher::UdevWatcher()
: uevent_netlink_sock_(-1)
, udev_monitor_sock_(-1)
, is_sd_mounted_(false)
, is_usb_mounted_(false)
{
    createWatcher();
}

UdevWatcher::~UdevWatcher()
{
#ifdef BUILD_FOR_ARM
    // Close socket.
    if (uevent_netlink_sock_ != -1)
    {
        close(uevent_netlink_sock_);
        uevent_netlink_sock_ = -1;
    }

    if (udev_monitor_sock_ != -1)
    {
        close(udev_monitor_sock_);
        udev_monitor_sock_ = -1;
    }
#endif
}

bool UdevWatcher::isUSBMounted()
{
    return checkUSB();
}

bool UdevWatcher::isSDMounted()
{
    return checkSD();
}

/// Borrowed from udev monitor. Initialize the udev monitor socket.
int UdevWatcher::init_udev_monitor_socket(void)
{
#ifdef BUILD_FOR_ARM
    struct sockaddr_un saddr;
    socklen_t addrlen;
    const int feature_on = 1;
    int retval;

    memset(&saddr, 0x00, sizeof(saddr));
    saddr.sun_family = AF_LOCAL;
    /* use abstract namespace for socket path */
    strcpy(&saddr.sun_path[1], "/org/kernel/udev/monitor");
    addrlen = offsetof(struct sockaddr_un, sun_path) + strlen(saddr.sun_path+1) + 1;

    udev_monitor_sock_ = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (udev_monitor_sock_ == -1) {
        fprintf(stderr, "error getting socket: %s\n", strerror(errno));
        return -1;
    }

    /* the bind takes care of ensuring only one copy running */
    retval = bind(udev_monitor_sock_, reinterpret_cast<struct sockaddr *>(&saddr), addrlen);
    if (retval < 0) {
        fprintf(stderr, "bind failed: %s\n", strerror(errno));
        close(udev_monitor_sock_);
        udev_monitor_sock_ = -1;
        return -1;
    }

    /* enable receiving of the sender credentials */
    setsockopt(udev_monitor_sock_, SOL_SOCKET, SO_PASSCRED, &feature_on, sizeof(feature_on));
#endif
    return 0;
}

int UdevWatcher::init_uevent_netlink_sock(void)
{
#ifdef BUILD_FOR_ARM
    struct sockaddr_nl snl;
    int retval;

    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;

    uevent_netlink_sock_ = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (uevent_netlink_sock_ == -1) {
        fprintf(stderr, "error getting socket: %s\n", strerror(errno));
        return -1;
    }

    retval = bind(uevent_netlink_sock_, reinterpret_cast<struct sockaddr *>(&snl),
        sizeof(struct sockaddr_nl));
    if (retval < 0) {
        fprintf(stderr, "bind failed: %s\n", strerror(errno));
        close(uevent_netlink_sock_);
        uevent_netlink_sock_ = -1;
        return -1;
    }
#endif
    return 0;
}

bool UdevWatcher::createWatcher()
{
    // Seems we don't need the uevent socket so far.
    // init_uevent_netlink_sock();
    // uevent_notifier_.reset(new QSocketNotifier(uevent_netlink_sock_, QSocketNotifier::Read, this));
    // connect(uevent_notifier_.get(), SIGNAL(activated(int)), this, SLOT(onKernelEvent(int)));

    init_udev_monitor_socket();
    udev_notifier_.reset(new QSocketNotifier(udev_monitor_sock_, QSocketNotifier::Read, this));
    connect(udev_notifier_.get(), SIGNAL(activated(int)), this, SLOT(onUdevEvent(int)));

    checkSD();
    checkUSB();

    return true;
}

bool UdevWatcher::checkSD()
{
#ifdef BUILD_FOR_ARM
    // Check sd device is available or not.
    int fd = open(SDMMC_DEVICE, O_RDWR);
    if (fd < 0)
    {
        is_sd_mounted_ = false;
    }
    else
    {
        is_sd_mounted_ = true;
        close(fd);
    }

    // Not correct
    qDebug("SD is inserted %d", is_sd_mounted_);
#endif
    return is_sd_mounted_;
}

bool UdevWatcher::checkUSB()
{
#ifdef BUILD_FOR_ARM
    // Check usb device is available or not.
    int fd = open(USB_DEVICE, O_RDWR);
    if (fd < 0)
    {
        is_usb_mounted_ = false;
    }
    else
    {
        is_usb_mounted_ = true;
        close(fd);
    }
    qDebug("USB is inserted %d", is_usb_mounted_);
#endif
    return is_usb_mounted_;
}

bool UdevWatcher::mountSD(bool mount)
{
    QStringList envs = QProcess::systemEnvironment();
    // Always umount the usb at first.
    {
        QProcess process;
        process.setEnvironment(envs);

        // args
        QStringList args;
        args << SDMMC_ROOT;

        // the application.
        process.start("umount", args);
        process.waitForFinished();
    }

    if (mount)
    {
        QProcess process;
        process.setEnvironment(envs);

        QStringList args;
        args << "-t";
        args << "vfat";
        args << SDMMC_DEVICE;
        args << SDMMC_ROOT;

        process.start("mount", args);
        if (process.waitForFinished())
        {
            if (process.exitCode() == 0)
            {
                qDebug("mount sd card succeed!");
                return true;
            }
            else
            {
                qDebug("can not mount sd card!");
                return true;
            }
        }
        return false;
    }
    return true;
}

bool UdevWatcher::mountUSB(bool mount)
{
    QStringList envs = QProcess::systemEnvironment();
    // Always umount the usb at first.
    {
        QProcess process;
        process.setEnvironment(envs);

        // args
        QStringList args;
        args << USB_ROOT;

        // the application.
        process.start("umount", args);
        process.waitForFinished();
    }

    if (mount)
    {
        QProcess process;
        process.setEnvironment(envs);

        QStringList args;
        args << "-t";
        args << "vfat";
        args << USB_DEVICE;
        args << USB_ROOT;

        process.start("mount", args);
        if (process.waitForFinished())
        {
            if (process.exitCode() == 0)
            {
                qDebug("mount usb succeed!");
                return true;
            }
            else
            {
                qDebug("can not mount usb.");
            }
        }
        return false;
    }
    return true;
}

void UdevWatcher::onKernelEvent(int)
{
#ifdef BUILD_FOR_ARM
    char buf[UEVENT_BUFFER_SIZE*2] = {0};
    ssize_t buflen;
    ssize_t keys;

    buflen = recv(uevent_netlink_sock_, &buf, sizeof(buf), 0);
    if (buflen <= 0)
    {
        qDebug("error receiving uevent message: %s\n", strerror(errno));
        return;
    }

    const char *devpath, *action, *subsys;
    keys = strlen(buf) + 1; /* start of payload */
    devpath = search_key("DEVPATH", &buf[keys], buflen);
    action = search_key("ACTION", &buf[keys], buflen);
    subsys = search_key("SUBSYSTEM", &buf[keys], buflen);
    qDebug("kernel event: action %s devpath %s subsys %s", action, devpath, subsys);
#endif
}

void UdevWatcher::onUdevEvent(int)
{
#ifdef BUILD_FOR_ARM
    char buf[UEVENT_BUFFER_SIZE*2] = {0};
    ssize_t buflen;
    ssize_t keys;

    buflen = recv(udev_monitor_sock_, &buf, sizeof(buf), 0);
    if (buflen <= 0)
    {
        qDebug("error receiving udev message: %s\n", strerror(errno));
        return;
    }

    keys = strlen(buf) + 1; /* start of payload */
    QString action = search_key("ACTION", &buf[keys], buflen);
    QString subsys = search_key("SUBSYSTEM", &buf[keys], buflen);

    if (subsys == MMC)
    {
        if (action == DEV_ADD)
        {
            qDebug("SD inserted!");
            mountSD(true);
            emit SDChanged(true, SDMMC_ROOT);
        }
        else if (action == DEV_REMOVE)
        {
            qDebug("SD unplugged!");
            mountSD(false);
            emit SDChanged(false, SDMMC_ROOT);
        }
    }
    else if (subsys == USB)
    {
        if (action == DEV_ADD)
        {
            qDebug("USB inserted and try to mount it!");
            mountUSB(true);
            emit USBChanged(true, USB_ROOT);
        }
        else if (action == DEV_REMOVE)
        {
            qDebug("USB unplugged!");
            mountUSB(false);
            emit USBChanged(false, USB_ROOT);
        }
    }

    qDebug("udev event: action %s subsys %s", qPrintable(action), qPrintable(subsys));
#endif
}
