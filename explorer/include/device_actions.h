#ifndef DEVICE_ACTIONS_H_
#define DEVICE_ACTIONS_H_

#include "onyx/base/base.h"
#include "onyx/ui/context_dialog_base.h"

using namespace ui;

namespace explorer
{

enum ExplorerDeviceAction
{
    INVALID_DEVICE_ACTION = -1,
    DEVICE_UMOUNT_SD_CARD,
    DEVICE_UMOUNT_USB,
    DEVICE_ROTATE_SCREEN,
    DEVICE_MUSIC,
    DEVICE_STANDBY,
    DEVICE_SHUTDOWN,
};

class DeviceActions : public BaseActions
{
    Q_OBJECT
public:
    DeviceActions(void);
    ~DeviceActions(void);

public:
    /// Generate or re-generate the setting actions.
    void generateActions();

    QAction * action(ExplorerDeviceAction type);

    ExplorerDeviceAction selected();

};

}

#endif //  SETTING_ACTIONS_H_
