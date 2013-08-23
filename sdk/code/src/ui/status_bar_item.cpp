#include "onyx/ui/status_bar_item.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/sys/platform.h"

namespace ui
{

StatusBarItem::StatusBarItem(const StatusBarItemType type, QWidget *parent)
    : QWidget(parent)
    , type_(type)
    , state_(STATE_NORMAL)
{
}

StatusBarItem::~StatusBarItem(void)
{
}


void StatusBarItem::setState(const int state)
{
    if (state_ == state)
    {
        return;
    }
    state_ = state;
}

QString StatusBarItem::getIconPath(QString str)
{
    QString path = "/usr/share/ui/status_bar";
    if(ui::isHD() && sys::isIRTouch())
    {
        if(str.startsWith(":"))
        {
            str = str.remove(0, 1);
        }
        path = path.append(str);
        return path;
    }
    return str;
}


}
