#include "onyx/ui/browser_navigation_actions.h"

namespace ui
{

BrowserNavigationActions::BrowserNavigationActions()
: BaseActions()
{
    category()->setIcon(QIcon(QPixmap(":/images/navigation.png")));
    enable_history_ = true;
}

BrowserNavigationActions::~BrowserNavigationActions(void)
{
    actions_.clear();
}

void BrowserNavigationActions::enableHistory(bool enable)
{
     this->enable_history_ = enable;
}

void BrowserNavigationActions::generateActions(QWebHistory *history,
        bool enable_hyperlink_navigation,
        bool hyperlink_navigation_mode,
        bool enable_mobile_mode,
        bool mobile_mode)
{
    category()->setFont(actionFont());
    category()->setText(QCoreApplication::tr("Navigation"));

    actions_.clear();

    bool can_go_back = history->canGoBack();
    if (can_go_back)
    {
        shared_ptr<QAction> backward(new QAction(exclusiveGroup()));
        backward->setFont(actionFont());
        backward->setText(QCoreApplication::tr("Backward"));
        backward->setIcon(QIcon(QPixmap(":/images/backward.png")));
        backward->setData(NAVIGATE_BACKWARD);
        backward->setEnabled(can_go_back);
        backward->setCheckable(true);
        backward->setChecked(false);
        actions_.push_back(backward);
    }

    bool can_go_forward = history->canGoForward();
    if (can_go_forward)
    {
        shared_ptr<QAction> forward(new QAction(exclusiveGroup()));
        forward->setFont(actionFont());
        forward->setText(QCoreApplication::tr("Forward"));
        forward->setIcon(QIcon(QPixmap(":/images/forward.png")));
        forward->setData(NAVIGATE_FORWARD);
        forward->setEnabled(can_go_forward);
        forward->setCheckable(true);
        forward->setChecked(false);
        actions_.push_back(forward);
    }

    shared_ptr<QAction> clear(new QAction(exclusiveGroup()));
    clear->setFont(actionFont());
    clear->setText(QCoreApplication::tr("Clear History"));
    clear->setIcon(QIcon(QPixmap(":/images/clear_history.png")));
    clear->setData(NAVIGATE_CLEAR_HISTORY);
    clear->setEnabled(true);
    clear->setCheckable(true);
    clear->setChecked(false);
    actions_.push_back(clear);

    if(enable_history_)
    {
        shared_ptr<QAction> Show(new QAction(exclusiveGroup()));
        Show->setFont(actionFont());
        Show->setText(QCoreApplication::tr("History"));
        Show->setIcon(QIcon(QPixmap(":/images/history.png")));
        Show->setData(NAVIGATE_SHOW_HISTORY);
        Show->setEnabled(true);
        Show->setCheckable(true);
        Show->setChecked(false);
        actions_.push_back(Show);
    }

    if (enable_hyperlink_navigation)
    {
        shared_ptr<QAction> navigate_hyperlink(new QAction(exclusiveGroup()));
        QString text;
        if (!hyperlink_navigation_mode)
        {
            text = QCoreApplication::tr("Hyperlink Navigation");
        }
        else {
            text = QCoreApplication::tr("Exit Navigation");
        }
        navigate_hyperlink->setText(text);
        navigate_hyperlink->setFont(actionFont());
        navigate_hyperlink->setIcon(QIcon(QPixmap(":/images/hyperlink_navigation_mode.png")));
        navigate_hyperlink->setData(NAVIGATE_HYPER_LINK_VIA_KEYBOARD);
        navigate_hyperlink->setEnabled(true);
        navigate_hyperlink->setCheckable(true);
        navigate_hyperlink->setChecked(false);
        actions_.push_back(navigate_hyperlink);
    }

    if (enable_mobile_mode)
    {
        shared_ptr<QAction> mode(new QAction(exclusiveGroup()));
        if (!mobile_mode)
        {
            mode->setText(QCoreApplication::tr("Mobile Mode"));
        }
        else
        {
            mode->setText(QCoreApplication::tr("Exit Mobile Mode"));
        }
        mode->setFont(actionFont());
        mode->setIcon(QIcon(QPixmap(":/images/mobile_mode.png")));
        mode->setData(NAVIGATE_BROWSER_MODE);
        mode->setEnabled(true);
        mode->setCheckable(true);
        mode->setChecked(false);
        actions_.push_back(mode);
    }
}

/// Retrieve the selected action.
NavigationType BrowserNavigationActions::selected()
{
    // Search for the changed actions.
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<NavigationType>(act->data().toInt());
    }
    return NAVIGATE_NONE;
}

}

