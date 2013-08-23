#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include "onyx/base/device.h"
#include "mount_entry_watcher.h"

#ifndef _WINDOWS
#include <mntent.h>
#endif

static const char* MOUNT_FS_DESC_FILE       = "/proc/mounts";

// The solution we implemented depeneds on udev.
// When a usb card is inserted to device, the udev will check it rules
// to mount the usb card automatically. After that the /proc/mounts
// is changed. The /proc/mounts is pollable (after Linux kernel 2.6.15),
// so we can add a watcher
// on the file handler to receive mount tree change event.
// Another way is to receive udev event directly, which is a little bit
// complex as we have to parse the messages. More details can be found
// from source code of hal.

// A result from /proc/mount could be
// tmpfs   /dev/shm
// devpts  /dev/pts
// tmpfs   /var/run
// tmpfs   /var/lock
// securityfs      /sys/kernel/security
// /dev/sdb1       /media/KINGSTON

static const std::string sdmmc_device = SDMMC_DEVICE;
static const std::string usb_device = USB_DEVICE;
static const std::string flash_device = "/dev/mtdblock5";

MountEntryWatcher::MountEntryWatcher()
: mount_fs_fd_(-1)
, notifier_(0)
{
    createWatcher();
}

MountEntryWatcher::~MountEntryWatcher()
{
#ifndef _WINDOWS
    if (mount_fs_fd_ != -1)
    {
        close(mount_fs_fd_);
        mount_fs_fd_ = -1;
    }
#endif

    if (notifier_)
    {
        delete notifier_;
        notifier_ = 0;
    }
}

bool MountEntryWatcher::createWatcher()
{
#ifndef _WINDOWS
    mount_fs_fd_ = open(MOUNT_FS_DESC_FILE, O_RDONLY);
    if (mount_fs_fd_ == -1)
    {
        qWarning("Can't open %s!", MOUNT_FS_DESC_FILE);
        return false;
    }
#endif

    updateMountTable();

    notifier_ = new QSocketNotifier(mount_fs_fd_, QSocketNotifier::Read, this);
    connect(notifier_, SIGNAL(activated(int)), this, SLOT(onMountEntryChanged(int)));
    return true;
}

void MountEntryWatcher::updateMountTable()
{
#ifndef _WINDOWS
    mount_entries.clear();

    // Initialize the local mount entries.
    FILE* mount_fp = setmntent(MOUNT_FS_DESC_FILE, "r");

    if (mount_fp == NULL)
    {
        qFatal("Can't get mount point list %s", MOUNT_FS_DESC_FILE);
        return;
    }

    // Loop over /proc/mounts
    struct mntent* mnt_entry = NULL;
    while ((mnt_entry = getmntent(mount_fp)))
    {
        // We get a mount entry in /proc/mounts
        // qDebug("%s\t%s", mnt_entry->mnt_fsname,  mnt_entry->mnt_dir);
        if (usb_device == mnt_entry->mnt_fsname)
        {
            mount_entries.insert(std::make_pair(usb_device, MountEntry(mnt_entry->mnt_dir, true)));
        }
        else if (sdmmc_device == mnt_entry->mnt_fsname)
        {
            mount_entries.insert(std::make_pair(sdmmc_device, MountEntry(mnt_entry->mnt_dir, true)));
        }
        else if (flash_device == mnt_entry->mnt_fsname)
        {
            mount_entries.insert(std::make_pair(flash_device, MountEntry(mnt_entry->mnt_dir, true)));
        }
    }

    // Finish with mount_fp.
    endmntent(mount_fp);
#endif
}

bool MountEntryWatcher::isUSBMounted() const
{
    return (mount_entries.find(usb_device) != mount_entries.end());
}

bool MountEntryWatcher::isSDMounted() const
{
    return (mount_entries.find(sdmmc_device) != mount_entries.end());
}

bool MountEntryWatcher::isFlashMounted() const
{
    return (mount_entries.find(flash_device) != mount_entries.end());
}

bool MountEntryWatcher::umountSD()
{
    QStringList envs = QProcess::systemEnvironment();
    QProcess process;
    process.setEnvironment(envs);

    // args
    QStringList args;
    args << SDMMC_ROOT;

    // the application.
    process.start("umount", args);
    process.waitForFinished();
    return true;
}

bool MountEntryWatcher::umountFlash()
{
    QStringList envs = QProcess::systemEnvironment();
    QProcess process;
    process.setEnvironment(envs);

    // args
    QStringList args;
    args << LIBRARY_ROOT;

    // the application.
    process.start("umount", args);
    process.waitForFinished();
    return true;
}

bool MountEntryWatcher::umountUSB()
{
    QStringList envs = QProcess::systemEnvironment();
    QProcess process;
    process.setEnvironment(envs);

    // args
    QStringList args;
    args << USB_ROOT;

    // the application.
    process.start("umount", args);
    process.waitForFinished();
    return true;
}

void MountEntryWatcher::onMountEntryChanged(int)
{
    bool usb_mounted = isUSBMounted();
    bool sd_mounted  = isSDMounted();
    bool flash_mounted  = isFlashMounted();
    updateMountTable();

    if (usb_mounted != isUSBMounted())
    {
        emit USBChanged(!usb_mounted, USB_ROOT);
    }

    // Still always emit sd card signals.
    // When some application occupies the files on sd card, the umount does not
    // work correctly. So the mount tree is not changed. The sd card removed event
    // can not be emitted. To solve this problem, we use the dbus-send in naboo_sd_handler.
    // In order to avoid notifiy twice, the sd card removed signal is emitted in
    // naboo_sd_handler.
    if (sd_mounted != isSDMounted())
    {
        emit SDChanged(!sd_mounted, SDMMC_ROOT);
    }

    if (flash_mounted != isFlashMounted())
    {
        emit flashChanged(!flash_mounted, LIBRARY_ROOT);
    }
}

void MountEntryWatcher::sdCardRemoved()
{
    emit SDChanged(false, SDMMC_ROOT);
}
