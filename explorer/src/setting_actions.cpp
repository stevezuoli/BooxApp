#include "setting_actions.h"
#include "system_controller.h"

namespace explorer
{

using namespace model;

SettingActions::SettingActions()
: BaseActions()
{
    category()->setIcon(QIcon(QPixmap(":/images/settings.png")));
}

SettingActions::~SettingActions(void)
{
}

void SettingActions::generateActions()
{
    category()->setText(QApplication::tr("Settings"));
    actions_.clear();

    shared_ptr<QAction> date(new QAction(exclusiveGroup()));
    date->setCheckable(true);
    date->setText(QApplication::tr("Date Settings"));
    date->setIcon(QIcon(QPixmap(":/images/date.png")));
    date->setData(NODE_TYPE_DATE);
    actions_.push_back(date);

    shared_ptr<QAction> time_zone(new QAction(exclusiveGroup()));
    time_zone->setCheckable(true);
    time_zone->setText(QApplication::tr("Time Zone"));
    time_zone->setIcon(QIcon(QPixmap(":/images/time_zone_menu.png")));
    time_zone->setData(NODE_TYPE_TIMEZONE);
    actions_.push_back(time_zone);

    shared_ptr<QAction> locale(new QAction(exclusiveGroup()));
    locale->setCheckable(true);
    locale->setText(QApplication::tr("Locale Settings"));
    locale->setIcon(QIcon(QPixmap(":/images/locale.png")));
    locale->setData(NODE_TYPE_LOCALE);
    actions_.push_back(locale);

    shared_ptr<QAction> pm(new QAction(exclusiveGroup()));
    pm->setCheckable(true);
    pm->setText(QApplication::tr("Power Management"));
    pm->setIcon(QIcon(QPixmap(":/images/power_management.png")));
    pm->setData(NODE_TYPE_PM);
    actions_.push_back(pm);

    /*
    // Disable network management.
    shared_ptr<QAction> nm(new QAction(exclusiveGroup()));
    nm->setCheckable(true);
    nm->setText(QCoreApplication::tr("Network Management"));
    nm->setIcon(QIcon(QPixmap(":/images/network_management.png")));
    nm->setData(NETWORK_MANAGEMENT_SETTING);
    actions_.push_back(nm);
    */

    if (explorer::controller::SystemController::instance().hasTouch())
    {
        shared_ptr<QAction> sc(new QAction(exclusiveGroup()));
        sc->setCheckable(true);
        sc->setText(QApplication::tr("Screen Calibration"));
        sc->setIcon(QIcon(QPixmap(":/images/screen_calibration.png")));
        sc->setData(NODE_TYPE_SCREEN_CALIBRATION);
        actions_.push_back(sc);
    }

    /*
    shared_ptr<QAction> 3g(new QAction(exclusiveGroup()));
    3g->setCheckable(true);
    3g->setText(QApplication::tr("Screen Calibration"));
    3g->setIcon(QIcon(QPixmap(":/images/screen_calibration.png")));
    3g->setData(NODE_TYPE_3G_CONNECTION);
    actions_.push_back(3g);
    */

    shared_ptr<QAction> format(new QAction(exclusiveGroup()));
    format->setCheckable(true);
    format->setText(QApplication::tr("Format Flash"));
    format->setIcon(QIcon(QPixmap(":/images/format_flash.png")));
    format->setData(NODE_TYPE_FORMAT_FLASH);
    actions_.push_back(format);

    /*
    shared_ptr<QAction> manual(new QAction(exclusiveGroup()));
    manual->setCheckable(true);
    manual->setText(QApplication::tr("User Manual"));
    manual->setIcon(QIcon(QPixmap(":/images/user_manual.png")));
    manual->setData(NODE_TYPE_USER_MANUAL);
    actions_.push_back(manual);

    shared_ptr<QAction> color(new QAction(exclusiveGroup()));
    color->setCheckable(true);
    color->setText(QApplication::tr("Color Settings"));
    color->setIcon(QIcon(QPixmap(":/images/color_settings.png")));
    color->setData(NODE_TYPE_WAVEFORM_SETTINGS);
    actions_.push_back(color);
    */

    shared_ptr<QAction> remove_account_info(new QAction(exclusiveGroup()));
    remove_account_info->setCheckable(true);
    remove_account_info->setText(QApplication::tr("Remove Account Info"));
    remove_account_info->setIcon(QIcon(QPixmap(":/images/remove_account_info.png")));
    remove_account_info->setData(NODE_TYPE_REMOVE_ACCOUNT_INFO);
    actions_.push_back(remove_account_info);

    shared_ptr<QAction> startup(new QAction(exclusiveGroup()));
    startup->setCheckable(true);
    startup->setText(QApplication::tr("Startup Setting"));
    startup->setIcon(QIcon(QPixmap(":/images/startup.png")));
    startup->setData(NODE_TYPE_STARTUP);
    actions_.push_back(startup);

    shared_ptr<QAction> screenUpdate(new QAction(exclusiveGroup()));
    screenUpdate->setCheckable(true);
    screenUpdate->setText(QApplication::tr("Screen Update"));
    screenUpdate->setIcon(QIcon(QPixmap(":/images/screen_update_setting.png")));
    screenUpdate->setData(NODE_TYPE_SCREEN_UPATE_SETTING);
    actions_.push_back(screenUpdate);

    shared_ptr<QAction> about(new QAction(exclusiveGroup()));
    about->setCheckable(true);
    about->setText(QApplication::tr("About"));
    about->setIcon(QIcon(QPixmap(":/images/about.png")));
    about->setData(NODE_TYPE_ABOUT);
    actions_.push_back(about);
}

model::NodeType SettingActions::selected()
{
    // Search for the changed actions.
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<model::NodeType>(act->data().toInt());
    }
    return NODE_TYPE_NULL;
}

}
