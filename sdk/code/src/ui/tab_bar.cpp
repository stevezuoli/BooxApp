#include "onyx/ui/tab_bar.h"

namespace ui
{

TabBar::TabBar(QWidget *parent)
: CatalogView(0, parent)
, orientation_(Qt::Horizontal)
{
}

TabBar::~TabBar(void)
{
    clearDatas(data());
}

bool TabBar::addButton(const int id,
                       const QString & title,
                       const QPixmap & pixmap)
{
    setFixedGrid(1, data().size() + 1);
    ODataPtr d(new OData);
    d->insert("id", id);
    d->insert("title", title);
    d->insert("cover", pixmap);
    data().push_back(d);
    setData(data(), true);
    return true;
}

bool TabBar::removeButton(const int id)
{
    return false;
}

bool TabBar::clickButton(const int id)
{
    selectButton(id);
    activateButton(id);
    return true;
}

int TabBar::selectedButton()
{
    return -1;
}

void TabBar::selectButton(const int id)
{
    foreach(ContentView * d, visibleSubItems())
    {
        if (d->data() && d->data()->value("id").toInt() == id)
        {
            d->setChecked(true);
        }
        else
        {
            d->setChecked(false);
        }
    }
}

void TabBar::activateButton(const int id)
{
    foreach(ContentView * d, visibleSubItems())
    {
        if (d->data() && d->data()->value("id").toInt() == id)
        {
            d->activate();
            return;
        }
    }
}

bool TabBar::setButtonText(const int id, const QString & title)
{
    return false;
}

bool TabBar::setOrientation(const Qt::Orientation orientation)
{
    if (orientation == orientation_)
    {
        return false;
    }
    return true;
}

void TabBar::onItemActivated(ContentView *item, int)
{

    if (item && item->data())
    {
        int id = item->data()->value("id").toInt();
        selectButton(id);
        emit buttonClicked(id);
    }
}

}
