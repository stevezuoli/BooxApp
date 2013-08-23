#ifndef MOUNT_ENTRY_WATCHER_H_
#define MOUNT_ENTRY_WATCHER_H_

#include <string>
#include <vector>
#include <QtCore/QtCore>
#include <QString>

/// Mount tree watcher.
class MountEntryWatcher : public QObject
{
    Q_OBJECT
public:
    MountEntryWatcher();
    ~MountEntryWatcher();

public Q_SLOTS:
    bool isUSBMounted() const;
    bool isSDMounted() const;
    bool isFlashMounted() const;

    bool umountUSB();
    bool umountSD();
    bool umountFlash();
    void sdCardRemoved();

Q_SIGNALS:
    void USBChanged(bool mounted, const QString &mount_point);
    void SDChanged(bool mounted, const QString &mount_point);
    void flashChanged(bool mounted, const QString &mount_point);

private Q_SLOTS:
    void onMountEntryChanged(int);

private:
    bool createWatcher();
    void updateMountTable();

private:
    struct MountEntry
    {
    public:
        std::string mount_point_;
        bool        exist_;

    public:
        MountEntry(const char* mount_point, bool exist = false)
        : mount_point_(mount_point)
        , exist_(exist)
        {
        }

        MountEntry & operator = (const MountEntry & right)
        {
            if (this != &right)
            {
                mount_point_ = right.mount_point_;
                exist_ = right.exist_;
            }
            return *this;
        }
    };

    typedef std::map<std::string, MountEntry> MountTable;
    typedef MountTable::iterator  MountTableIter;

private:
    MountTable mount_entries;

private:
    int mount_fs_fd_;
    QSocketNotifier *notifier_;
};

#endif // MOUNT_ENTRY_WATCHER_H_
