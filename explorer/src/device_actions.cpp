#include "device_actions.h"
#include "onyx/sys/sys_conf.h"

namespace explorer
{


DeviceActions::DeviceActions()
: BaseActions()
{
    category()->setIcon(QIcon(QPixmap(":/images/settings.png")));
}


DeviceActions::~DeviceActions(void)
{
}

void DeviceActions::generateActions()
{
    category()->setText(QApplication::tr("Device"));
    actions_.clear();

    shared_ptr<QAction> rotate(new QAction(exclusiveGroup()));
    rotate->setCheckable(true);
    rotate->setText(QApplication::tr("Screen Rotation"));
    rotate->setIcon(QIcon(QPixmap(":/images/screen_rotation.png")));
    rotate->setData(DEVICE_ROTATE_SCREEN);
    actions_.push_back(rotate);

    shared_ptr<QAction> umount_sd(new QAction(exclusiveGroup()));
    umount_sd->setCheckable(true);
    umount_sd->setText(QApplication::tr("Safely Remove SD"));
    umount_sd->setIcon(QIcon(QPixmap(":/images/umount_sd.png")));
    umount_sd->setData(DEVICE_UMOUNT_SD_CARD);
    actions_.push_back(umount_sd);

    if (sys::SystemConfig::isMusicPlayerAvailable())
    {
        shared_ptr<QAction> music(new QAction(exclusiveGroup()));
        music->setCheckable(true);
        music->setText(QCoreApplication::tr("Music"));
        music->setIcon(QIcon(QPixmap(":/images/music.png")));
        music->setData(DEVICE_MUSIC);
        actions_.push_back(music);
    }

    //So far, don't support usb device.
    //shared_ptr<Action> umount_usb(new Action(exclusiveGroup()));
    //umount_usb->setCheckable(true);
    //umount_usb->setText(QCoreApplication::tr("Umount USB"));
    //umount_usb->setIcon(QIcon(QPixmap(":/images/umount_usb.png")));
    //umount_usb->setData(DEVICE_UMOUNT_USB);
    //actions_.push_back(umount_usb);

    /*
    shared_ptr<QAction> standby(new QAction(exclusiveGroup()));
    standby->setCheckable(true);
    standby->setText(QCoreApplication::tr("Standby"));
    standby->setIcon(QIcon(QPixmap(":/images/standby_menu.png")));
    standby->setData(DEVICE_STANDBY);
    actions_.push_back(standby);
    */

    shared_ptr<QAction> standby(new QAction(exclusiveGroup()));
    standby->setCheckable(true);
    standby->setText(QApplication::tr("Standby"));
    standby->setIcon(QIcon(QPixmap(":/images/standby_menu.png")));
    standby->setData(DEVICE_STANDBY);
    actions_.push_back(standby);

    shared_ptr<QAction> shutdown(new QAction(exclusiveGroup()));
    shutdown->setCheckable(true);
    shutdown->setText(QApplication::tr("Shutdown"));
    shutdown->setIcon(QIcon(QPixmap(":/images/shutdown_menu.png")));
    shutdown->setData(DEVICE_SHUTDOWN);
    actions_.push_back(shutdown);
}

QAction * DeviceActions::action(ExplorerDeviceAction type)
{
    for(int i = 0; i < static_cast<int>(actions_.size()); ++i)
    {
        if (actions_[i]->data().toInt() == type)
        {
            return actions_[i].get();
        }
    }
    return 0;
}

ExplorerDeviceAction DeviceActions::selected()
{
    // Search for the changed actions.
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<ExplorerDeviceAction>(act->data().toInt());
    }
    return INVALID_DEVICE_ACTION;
}

}
