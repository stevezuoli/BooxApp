#include "website_actions.h"

namespace ui
{

WebSiteActions::WebSiteActions()
: BaseActions()
{
    category()->setIcon(QIcon(QPixmap(":/images/websites.png")));
}

WebSiteActions::~WebSiteActions(void)
{
}

void WebSiteActions::generateActions(WebSiteActionTypes types)
{
    category()->setText(QApplication::tr("WebSites"));
    actions_.clear();

    // Adjust the order if necessary.
    const WebSiteActionType all[] =
    {
        WEBSITE_ADD,WEBSITE_DELETE 
    };
    int size = sizeof(all)/sizeof(all[0]);
    for(int i = 0; i < size; ++i)
    {
        if (types.testFlag(all[i]))
        {
            shared_ptr<QAction> action(new QAction(exclusiveGroup()));
            switch (all[i])
            {
                case WEBSITE_ADD:
                    action->setCheckable(true);
                    action->setText(QApplication::tr("Add"));
                    action->setIcon(QIcon(QPixmap(":/images/add_link.png")));
                    action->setData(WEBSITE_ADD);
                    actions_.push_back(action);
                    break;

                case WEBSITE_DELETE:
                    action->setCheckable(true);
                    action->setText(QApplication::tr("Delete"));
                    action->setIcon(QIcon(QPixmap(":/images/remove_link.png")));
                    action->setData(WEBSITE_DELETE);
                    actions_.push_back(action);
                    break;

                default:
                    break;
            }
        }
    } 
}

QAction * WebSiteActions::action(const WebSiteActionType type)
{
    for(int i = 0; i < static_cast<int>(actions_.size()); ++i)
    {
        if (actions_.at(i)->data().toInt() == type)
        {
            return actions_.at(i).get();
        }
    }
    return 0;
}

WebSiteActionType WebSiteActions::selected()
{
    // Search for the changed actions.
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<WebSiteActionType>(act->data().toInt());
    }
    return INVALID_WEBSITE_ACTION;
}


}
