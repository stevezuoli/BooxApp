#include "applications_actions.h"
#include "system_controller.h"

namespace explorer
{

using namespace model;

ApplicationsActions::ApplicationsActions()
: BaseActions()
{
    category()->setIcon(QIcon(QPixmap(":/images/settings.png")));
}

ApplicationsActions::~ApplicationsActions(void)
{
}

void ApplicationsActions::generateActions()
{
    category()->setText(QApplication::tr("Settings"));
    actions_.clear();

    shared_ptr<QAction> calendar(new QAction(exclusiveGroup()));
    calendar->setCheckable(true);
    calendar->setText(QApplication::tr("Calendar"));
    calendar->setIcon(QIcon(QPixmap(":/images/calendar.png")));
    calendar->setData(NODE_TYPE_CALENDAR);
    actions_.push_back(calendar);

    shared_ptr<QAction> clock(new QAction(exclusiveGroup()));
    clock->setCheckable(true);
    clock->setText(QApplication::tr("Clock"));
    clock->setIcon(QIcon(QPixmap(":/images/full_screen_clock.png")));
    clock->setData(NODE_TYPE_FULL_SCREEN_CLOCK);
    actions_.push_back(clock);
}

model::NodeType ApplicationsActions::selected()
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
