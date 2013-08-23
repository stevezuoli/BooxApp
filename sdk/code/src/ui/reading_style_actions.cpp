
#include "onyx/ui/reading_style_actions.h"


namespace ui
{

ReadingStyleActions::ReadingStyleActions(void)
: BaseActions()
{
    category()->setIcon(QIcon(QPixmap(":/images/reading_style.png")));
}

ReadingStyleActions::~ReadingStyleActions(void)
{
}

void ReadingStyleActions::generateActions(ReadingStyleType selected)
{
    category()->setFont(actionFont());
    category()->setText(QCoreApplication::tr("Style"));
    actions_.clear();

    for(int i = STYLE_LINE_SPACING_8; i <= STYLE_LINE_SPACING_20; ++i)
    {
        // Add to category automatically.
        shared_ptr<QAction> act(new QAction(exclusiveGroup()));

        // Change font and make it as checkable.
        act->setCheckable(true);
        act->setData(i);

        QString t(QCoreApplication::tr("%1% line spacing"));
        int percentage = 80 + (i - STYLE_LINE_SPACING_8) * 10;
        t = t.arg(percentage);
        act->setFont(actionFont());
        act->setText(t);
        act->setIcon(QIcon(QPixmap(":/images/line_space.png")));

        if (selected == i)
        {
            act->setChecked(true);
        }
        actions_.push_back(act);
    }

#if defined(BUILD_WITH_TFT) || defined(_WINDOWS)

    shared_ptr<QAction> separator(new QAction(exclusiveGroup()));
    separator->setSeparator(true);
    actions_.push_back(separator);

    shared_ptr<QAction> day(new QAction(exclusiveGroup()));
    day->setCheckable(true);
    day->setData(STYLE_WHITE_BACKGROUND);
    QString d(tr("White Backgroud"));
    day->setText(d);
    day->setIcon(QIcon(QPixmap(":/images/line_space.png")));
    actions_.push_back(day);

    shared_ptr<QAction> night(new QAction(exclusiveGroup()));
    night->setCheckable(true);
    night->setData(STYLE_BLACK_BACKGROUND);
    QString n(tr("Black Backgroud"));
    night->setText(n);
    night->setIcon(QIcon(QPixmap(":/images/line_space.png")));
    actions_.push_back(night);

#endif
    /*
    shared_ptr<QAction> separator(new QAction(exclusiveGroup()));
    separator->setSeparator(true);
    actions_.push_back(separator);

    for(int i = STYLE_PAGE_MARGINS_SMALL; i <= STYLE_PAGE_MARGINS_LARGE; ++i)
    {
        // Add to category automatically.
        shared_ptr<QAction> act(new QAction(exclusiveGroup()));
        act->setCheckable(true);
        act->setData(i);

        QString t;
        switch (i)
        {
        case STYLE_PAGE_MARGINS_SMALL:
            t = QCoreApplication::tr("Small");
            break;
        case STYLE_PAGE_MARGINS_MIDDLE:
            t = QCoreApplication::tr("Middle");
            break;
        case STYLE_PAGE_MARGINS_LARGE:
            t = QCoreApplication::tr("Middle");
            break;
        }
        act->setText(t);
        act->setIcon(QIcon(QPixmap(":/images/free_text_search.png")));
        actions_.push_back(act);
    }
    */
}

void ReadingStyleActions::setActionStatus(const ReadingStyleType type, bool selected)
{
}

QAction * ReadingStyleActions::action(const ReadingStyleType type)
{
    for (int i = 0; i < static_cast<int>(actions_.size()); ++i)
    {
        if (type == actions_[i]->data().toInt())
        {
            return actions_[i].get();
        }
    }
    return 0;
}

ReadingStyleType ReadingStyleActions::selected()
{
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<ReadingStyleType>(act->data().toInt());
    }
    return STYLE_INVALID;
}

}  // namespace ui

