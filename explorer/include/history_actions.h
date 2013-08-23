#ifndef HISTORY_ACTIONS_H_
#define HISTORY_ACTIONS_H_

#include "onyx/base/base.h"
#include "onyx/ui/context_dialog_base.h"

namespace ui
{

enum HistoryActionType
{
    INVALID_HISTORY_ACTION = -1,
    HISTORY_CLEAR_ALL  = 0,
};

class HistoryActions : public BaseActions
{
public:
    HistoryActions(void);
    ~HistoryActions(void);

public:
    /// Generate or re-generate the edit actions.
    void generateActions();
    QAction * action(const HistoryActionType type);
    HistoryActionType selected();

};

}

#endif //  FILE_ACTIONS_H_
