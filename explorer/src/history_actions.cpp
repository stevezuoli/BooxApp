#include "history_actions.h"

namespace ui
{

HistoryActions::HistoryActions()
: BaseActions()
{
    category()->setIcon(QIcon(QPixmap(":/images/recently_documents.png")));
}

HistoryActions::~HistoryActions(void)
{
}

void HistoryActions::generateActions()
{
    category()->setText(QApplication::tr("Recent Documents"));
    actions_.clear();

    shared_ptr<QAction> clear(new QAction(exclusiveGroup()));
    clear->setCheckable(true);
    clear->setText(QApplication::tr("Clear"));
    clear->setIcon(QIcon(QPixmap(":/images/delete.png")));
    clear->setData(HISTORY_CLEAR_ALL);
    actions_.push_back(clear);
}

QAction * HistoryActions::action(const HistoryActionType type)
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

HistoryActionType HistoryActions::selected()
{
    // Search for the changed actions.
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<HistoryActionType>(act->data().toInt());
    }
    return INVALID_HISTORY_ACTION;
}


}
