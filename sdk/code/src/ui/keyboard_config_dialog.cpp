#include "onyx/ui/keyboard_config_dialog.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/data/data_tags.h"

namespace ui
{

enum ButtonType
{
    BUTTON_TYPE_OK = 11,
    BUTTON_TYPE_CANCEL = 12,
};

const QString KeyboardConfigDialog::KEY_FOR_MISC_CONF = "keyboard_config";

KeyboardConfigDialog::KeyboardConfigDialog(bool home_and_back_locked,
        bool page_turning_locked,QWidget *parent)
    : OnyxDialog(parent)
    , big_layout_(&content_widget_)
    , button_layout_(0)
    , config_group_(0, this)
    , home_and_back_locked_(home_and_back_locked)
    , page_turning_locked_(page_turning_locked)
{
    QSize fix_size(450, 250);
    setFixedSize(fix_size);

    createLayout();
    connectWithChildren();
    updateTitle(tr("Lock Keyboard"));
}

KeyboardConfigDialog::~KeyboardConfigDialog()
{
    clearDatas(config_group_datas_);
}

void KeyboardConfigDialog::createConfigGroup()
{
    config_group_.setSubItemType(CheckBoxView::type());
    config_group_.setPreferItemSize(QSize(-1, 60));

    ODataPtr first(new OData);
//    QPixmap home_and_back(":/images/lock_home_and_back.png");
//    first->insert(TAG_COVER, home_and_back);
    // Use text temporary
    first->insert(TAG_TITLE, tr("Lock Menu and Back"));
    first->insert(TAG_CHECKED, home_and_back_locked_);
    config_group_datas_.push_back(first);

    ODataPtr second(new OData);
//    QPixmap page_turning(":/images/lock_page_turning.png");
//    second->insert(TAG_COVER, page_turning);
    // Use text temporary
    second->insert(TAG_TITLE, tr("Lock PageUp and PageDown"));
    second->insert(TAG_CHECKED, page_turning_locked_);
    config_group_datas_.push_back(second);

    config_group_.setData(config_group_datas_);

    config_group_.setFixedGrid(2, 1);
    config_group_.setFixedHeight(120);
    config_group_.setNeighbor(&button_view_, CatalogView::DOWN);
    config_group_.setNeighbor(&button_view_, CatalogView::RECYCLE_DOWN);
}

void KeyboardConfigDialog::createButtonView()
{
    const int height = defaultItemHeight();
    button_view_.setPreferItemSize(QSize(height, height));

    ODataPtr dd(new OData);
    dd->insert(TAG_TITLE, tr("OK"));
    dd->insert(TAG_MENU_TYPE, BUTTON_TYPE_OK);
    button_view_datas_.push_back(dd);

    ODataPtr b(new OData);
    b->insert(TAG_TITLE, tr("Cancel"));
    b->insert(TAG_MENU_TYPE, BUTTON_TYPE_CANCEL);
    button_view_datas_.push_back(b);

    button_view_.setSpacing(2);
    button_view_.setFixedGrid(1, 2);
    button_view_.setFixedHeight(defaultItemHeight()+2*SPACING);
    button_view_.setFixedWidth(defaultItemHeight()*6);
    button_view_.setData(button_view_datas_);
}

void KeyboardConfigDialog::connectWithChildren()
{
    connect(&config_group_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&button_view_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}

void KeyboardConfigDialog::setKeyboardConfig()
{
    ContentView *front = config_group_.visibleSubItems().front();
    home_and_back_locked_ = false;
    if (front->data() && front->data()->contains(TAG_CHECKED))
    {
        home_and_back_locked_ = front->data()->value(TAG_CHECKED).toBool();
    }

    ContentView *back = config_group_.visibleSubItems().back();
    page_turning_locked_ = false;
    if (back->data() && back->data()->contains(TAG_CHECKED))
    {
        page_turning_locked_ = back->data()->value(TAG_CHECKED).toBool();
    }
}

void KeyboardConfigDialog::createLayout()
{
    vbox_.setSpacing(0);
    content_widget_.setBackgroundRole(QPalette::Button);
    content_widget_.setContentsMargins(0, 0, 0, 0);
    updateTitleIcon(QPixmap());

//    description_.setText(tr("Choose keyboard to lock:"));
//    description_.setAlignment(Qt::AlignLeft);
//    description_.setFixedHeight(defaultItemHeight());

    createConfigGroup();
    createButtonView();

//    big_layout_.addWidget(&description_, 0, Qt::AlignTop);
    big_layout_.addWidget(&config_group_, 1, Qt::AlignTop);

    button_layout_.addWidget(&button_view_, 0, Qt::AlignRight);

    big_layout_.addLayout(&button_layout_);
}

int KeyboardConfigDialog::popup()
{
    if (isHidden())
    {
        show();
    }

    QWidget * widget = safeParentWidget(parentWidget());
    int x = (widget->width()-width()) / 2;
    int y = (widget->height()-height()) / 2;
    move(x, y);

    onyx::screen::watcher().addWatcher(this);
    // set initial focus
    config_group_.setFocusTo(0, 0);
    int ret = this->exec();
    onyx::screen::watcher().removeWatcher(this);
    return ret;
}

bool KeyboardConfigDialog::homeAndBackLocked()
{
    return home_and_back_locked_;
}

bool KeyboardConfigDialog::pageTurningLocked()
{
    return page_turning_locked_;
}

void KeyboardConfigDialog::onItemActivated(CatalogView *catalog,
                                   ContentView *item,
                                   int user_data)
{
    OData * item_data = item->data();
    if (item_data->contains(TAG_MENU_TYPE))
    {
        int menu_type = item->data()->value(TAG_MENU_TYPE).toInt();
        if(BUTTON_TYPE_OK == menu_type)
        {
            setKeyboardConfig();
            accept();
        }
        else if(BUTTON_TYPE_CANCEL == menu_type)
        {
            onCloseClicked();
        }
    }
    else if (catalog == &config_group_)
    {
        bool checked = false;
        if (item->data()->contains(TAG_CHECKED))
        {
            checked = item->data()->value(TAG_CHECKED).toBool();
        }
        item->data()->insert(TAG_CHECKED, !checked);
        item->repaint();
        update();
        onyx::screen::watcher().enqueue(item, onyx::screen::ScreenProxy::GU);
    }
}

}
