#include "view_type_actions.h"

static const int VIEW_BASE = 0;
static const int FIELD_BASE = 100;
static const int ORDER_BASE = 200;

namespace ui
{

ViewTypeActions::ViewTypeActions()
: BaseActions()
, view_group_(this)
, field_group_(this)
, order_group_(this)
{
    category()->setIcon(QIcon(QPixmap(":/images/view.png")));
}

ViewTypeActions::~ViewTypeActions(void)
{
    actions_.clear();
}

void ViewTypeActions::generateActions(const ViewType current,
                                      const Field by,
                                      const SortOrder order)
{
    category()->setText(QApplication::tr("View"));
    actions_.clear();

    for(int i = LIST_VIEW; i <= THUMBNAIL_VIEW; ++i)
    {
        shared_ptr<QAction> act(new QAction(&view_group_));
        act->setCheckable(true);

        if (i == current)
        {
            act->setChecked(true);
        }

        switch (i)
        {
        case LIST_VIEW:
            act->setText(QApplication::tr("List view"));
            act->setIcon(QIcon(QPixmap(":/images/list_view.png")));
            break;
        case DETAILS_VIEW:
            act->setText(QApplication::tr("Details view"));
            act->setIcon(QIcon(QPixmap(":/images/details_view.png")));
            break;
        case THUMBNAIL_VIEW:
            act->setText(QApplication::tr("Thumbnail view"));
            act->setIcon(QIcon(QPixmap(":/images/thumbnail_view.png")));
            break;
        }
        act->setData(i + VIEW_BASE);
        actions_.push_back(act);
    }

    // separator.
    shared_ptr<QAction> separator1(new QAction(exclusiveGroup()));
    separator1->setSeparator(true);
    actions_.push_back(separator1);

    // by field, the same order as header bar.
    shared_ptr<QAction> by_name(new QAction(&field_group_));
    by_name->setCheckable(true);
    by_name->setText(QApplication::tr("By Name"));
    by_name->setIcon(QIcon(QPixmap(":/images/sort_by_name.png")));
    by_name->setData(NAME + FIELD_BASE);
    if (by == NAME)
    {
        by_name->setChecked(true);
    }
    actions_.push_back(by_name);

    // by type
    shared_ptr<QAction> by_type(new QAction(&field_group_));
    by_type->setCheckable(true);
    by_type->setText(QApplication::tr("By Type"));
    by_type->setIcon(QIcon(QPixmap(":/images/sort_by_type.png")));
    by_type->setData(NODE_TYPE + FIELD_BASE);
    if (by == NODE_TYPE)
    {
        by_type->setChecked(true);
    }
    actions_.push_back(by_type);

    // by size
    shared_ptr<QAction> by_size(new QAction(&field_group_));
    by_size->setCheckable(true);
    by_size->setText(QApplication::tr("By Size"));
    by_size->setIcon(QIcon(QPixmap(":/images/sort_by_size.png")));
    by_size->setData(SIZE + FIELD_BASE);
    if (by == SIZE)
    {
        by_size->setChecked(true);
    }
    actions_.push_back(by_size);

    // By last access time
    shared_ptr<QAction> by_access(new QAction(&field_group_));
    by_access->setCheckable(true);
    by_access->setText(QApplication::tr("By Access Time"));
    by_access->setIcon(QIcon(QPixmap(":/images/sort_by_time.png")));
    by_access->setData(LAST_ACCESS_TIME + FIELD_BASE);
    if (by == LAST_ACCESS_TIME)
    {
        by_access->setChecked(true);
    }
    actions_.push_back(by_access);

    // By rating
    shared_ptr<QAction> by_rating(new QAction(&field_group_));
    by_rating->setCheckable(true);
    by_rating->setText(QApplication::tr("By Rating"));
    by_rating->setIcon(QIcon(QPixmap(":/images/sort_by_rating.png")));
    by_rating->setData(RATING + FIELD_BASE);
    if (by == RATING)
    {
        by_rating->setChecked(true);
    }
    actions_.push_back(by_rating);

    // separator.
    shared_ptr<QAction> separator2(new QAction(exclusiveGroup()));
    separator2->setSeparator(true);
    actions_.push_back(separator2);

    // order
    shared_ptr<QAction> ascending(new QAction(&order_group_));
    ascending->setCheckable(true);
    ascending->setText(QApplication::tr("Ascending"));
    ascending->setIcon(QIcon(QPixmap(":/images/ascending.png")));
    ascending->setData(ASCENDING + ORDER_BASE);
    if (order == ASCENDING)
    {
        ascending->setChecked(true);
    }
    actions_.push_back(ascending);

    shared_ptr<QAction> descending(new QAction(&order_group_));
    descending->setCheckable(true);
    descending->setText(QApplication::tr("Descending"));
    descending->setIcon(QIcon(QPixmap(":/images/descending.png")));
    descending->setData(DESCENDING + ORDER_BASE);
    if (order == DESCENDING)
    {
        descending->setChecked(true);
    }
    actions_.push_back(descending);
}

QAction * ViewTypeActions::action(const ViewType view)
{
    for(int i = 0; i < static_cast<int>(actions_.size()); ++i)
    {
        if (actions_[i]->data().toInt() == view + VIEW_BASE)
        {
            return actions_[i].get();
        }
    }
    return 0;
}

QAction * ViewTypeActions::action(const Field by)
{
    for(int i = 0; i < static_cast<int>(actions_.size()); ++i)
    {
        if (actions_[i]->data().toInt() == by + FIELD_BASE)
        {
            return actions_[i].get();
        }
    }
    return 0;
}

QAction * ViewTypeActions::action(const  SortOrder order)
{
    for(int i = 0; i < static_cast<int>(actions_.size()); ++i)
    {
        if (actions_[i]->data().toInt() == order + ORDER_BASE)
        {
            return actions_[i].get();
        }
    }
    return 0;
}

    /// Retrieve the selected view type.
ViewType ViewTypeActions::selectedViewType()
{
    // Search for the changed actions.
    QAction * act = view_group_.checkedAction();
    if (act)
    {
        return static_cast<ViewType>(act->data().toInt() - VIEW_BASE);
    }
    return INVALID_VIEW;
}

Field ViewTypeActions::selectedField()
{
    // Search for the changed actions.
    QAction * act = field_group_.checkedAction();
    if (act)
    {
        return static_cast<Field>(act->data().toInt() - FIELD_BASE);
    }
    return NONE;
}

SortOrder ViewTypeActions::selectedOrder()
{
    // Search for the changed actions.
    QAction * act = order_group_.checkedAction();
    if (act)
    {
        return static_cast<SortOrder>(act->data().toInt() - ORDER_BASE);
    }
    return NO_ORDER;
}

}
