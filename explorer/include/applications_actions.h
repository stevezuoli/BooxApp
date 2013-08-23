#ifndef SETTING_ACTIONS_H_
#define SETTING_ACTIONS_H_

#include "onyx/base/base.h"
#include "onyx/ui/context_dialog_base.h"
#include "node_types.h"

using namespace ui;

namespace explorer
{

class ApplicationsActions : public BaseActions
{
public:
    ApplicationsActions(void);
    ~ApplicationsActions(void);

public:
    /// Generate or re-generate the setting actions.
    void generateActions();
    model::NodeType selected();

};

}

#endif //  SETTING_ACTIONS_H_
