#include "onyx/ui/onyx_number_widget.h"
#include "onyx/screen/screen_proxy.h"

namespace ui
{

OnyxNumberWidget::OnyxNumberWidget(QWidget *parent)
    : QWidget(parent)
    , buttons_(0)
    , number_layout_(this)
{
    createLayout();
}

OnyxNumberWidget::~OnyxNumberWidget(void)
{
}

void OnyxNumberWidget::createLayout()
{
    ODatas button_data;
    buttons_.setSubItemType(ButtonView::type());
    buttons_.setPreferItemSize(QSize(60, 60));

    for (int i = 1; i < 10; ++i)
    {
        OData * dd = new OData;
        dd->insert(TAG_TITLE, QString::number(i));
        dd->insert(TAG_ID, i);
        dd->insert(TAG_FONT_SIZE, 32);
        button_data.push_back(dd);
    }

    // Backspace button.
    {
        OData * dd = new OData;
        dd->insert(TAG_TITLE, QString(QChar(0x2190)));
        dd->insert(TAG_ID, -1);
        dd->insert(TAG_FONT_SIZE, 32);
        button_data.push_back(dd);
    }

    // number zero button
    {
        OData * dd = new OData;
        dd->insert(TAG_TITLE, QString("0"));
        dd->insert(TAG_ID, 0);
        dd->insert(TAG_FONT_SIZE, 32);
        button_data.push_back(dd);
    }

    // OK
    {
        OData * dd = new OData;
        dd->insert(TAG_TITLE, QString("OK"));
        dd->insert(TAG_ID, -2);
        dd->insert(TAG_FONT_SIZE, 32);
        button_data.push_back(dd);
    }

    buttons_.setData(button_data);
    buttons_.setFixedGrid(4, 3);
    buttons_.setSpacing(8);
    buttons_.setMinimumHeight(250);

    buttons_.setSearchPolicy(CatalogView::AutoHorRecycle|CatalogView::NeighborFirst);
    connect(&buttons_, SIGNAL(itemActivated(CatalogView*,ContentView*,int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));

    buttons_.setCheckedTo(1, 1);
    number_layout_.addWidget(&buttons_);
}

void OnyxNumberWidget::keyReleaseEvent(QKeyEvent *e)
{
    e->accept();
}

void OnyxNumberWidget::setOkButtonFocus(void)
{
    buttons_.setFocusTo(3, 2);
    buttons_.setCheckedTo(3, 2);
}

void OnyxNumberWidget::onItemActivated(CatalogView *catalog, ContentView *item, int user_data)
{
    if (!item || !item->data())
    {
        return;
    }

    OData * item_data = item->data();
    int type = item_data->value(TAG_ID).toInt();
    QString text;
    QKeyEvent * key_event;

    switch(type)
    {
    case -1:
        key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier, "");
        emit keyPress(key_event);
        break;
    case -2:
        emit okClicked();
        break;
    default:
        text = QString::number(type);
        QKeyEvent * key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_0 + type, Qt::NoModifier, text);
        emit keyPress(key_event);
        break;
    }
}

}   // namespace ui

