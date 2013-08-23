
#include "onyx/ui/factory.h"
#include "onyx/ui/content_view.h"

namespace ui
{

Factory::Factory()
{
}

Factory::~Factory()
{
}

ContentView * Factory::createView(QWidget *parent, const QString &type)
{
    ContentView *target = 0;
    if (CheckBoxView::type() == type)
    {
        target = new CheckBoxView(parent);
    }
    else if (LineEditView::type() == type)
    {
        target = new LineEditView(parent);
    }
    else if (ClockView::type() == type)
    {
        target = new ClockView(parent);
    }
    else if (ButtonView::type() == type)
    {
        target = new ButtonView(parent);
    }
    else
    {
        target = new CoverView(parent);
    }
    return target;
}

}
