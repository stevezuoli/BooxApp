#ifndef EXPLORER_VIEW_TYPE_ACTIONS_H_
#define EXPLORER_VIEW_TYPE_ACTIONS_H_

#include "onyx/base/base.h"
#include "onyx/ui/context_dialog_base.h"
#include "node_types.h"

using namespace explorer::model;

namespace ui
{

class ViewTypeActions : public BaseActions
{
public:
    ViewTypeActions(void);
    ~ViewTypeActions(void);

public:
    /// Generate or re-generate the view type actions group.
    void generateActions(const ViewType current, const Field by, const SortOrder order);

    /// Get action according to the view type.
    QAction * action(const ViewType view);

    /// Get action according to the by field.
    QAction * action(const Field by);

    /// Get action according to the order type.
    QAction * action(const  SortOrder order);

    /// Retrieve the selected view type.
    ViewType selectedViewType();
    Field selectedField();
    SortOrder selectedOrder();


private:
    QActionGroup view_group_;
    QActionGroup field_group_;
    QActionGroup order_group_;
};

}

#endif //  EXPLORER_VIEW_TYPE_ACTIONS_H_
