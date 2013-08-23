#include "mount_entry_watcher.h"

class MountTreeWatcher : public QObject
{
    Q_OBJECT
public:
    MountTreeWatcher();
    ~MountTreeWatcher();

private Q_SLOTS:
    void onUSBChanged(bool mounted, const QString &mount_point);
    void onSDChanged(bool mounted, const QString &mount_point);

private:
    MountEntryWatcher impl_;
};



