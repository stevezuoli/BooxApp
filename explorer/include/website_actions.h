#ifndef WEBSITE_ACTIONS_H_
#define WEBSITE_ACTIONS_H_

#include "onyx/base/base.h"
#include "onyx/ui/context_dialog_base.h"

namespace ui
{

enum WebSiteActionType
{
    INVALID_WEBSITE_ACTION     = 0x0,
    WEBSITE_ADD                = 0x1,
    WEBSITE_DELETE             = 0x2
};

Q_DECLARE_FLAGS(WebSiteActionTypes, WebSiteActionType)
Q_DECLARE_OPERATORS_FOR_FLAGS(WebSiteActionTypes)

class WebSiteActions : public BaseActions
{
public:
    WebSiteActions(void);
    ~WebSiteActions(void);

public:
    /// Generate or re-generate the edit actions.
    void generateActions(WebSiteActionTypes types = WEBSITE_ADD);
    QAction * action(const WebSiteActionType type);
    WebSiteActionType selected();

};

}

#endif //  WEBSITE_ACTIONS_H_
