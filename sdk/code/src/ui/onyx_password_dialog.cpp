#include "onyx/ui/onyx_password_dialog.h"
#include "onyx/data/data_tags.h"
#include "onyx/ui/content_view.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/ui_utils.h"

namespace ui
{

static const int LABEL_WIDTH = 150;

// A password line edit will be provided on default. Need to specify an OData
// for each line edit besides password.
// Sample:
//
//  User ID:
//  Password:
//
// needs an OData with TAG_TITLE/value ("User ID: ") inserted.
// Insert TAG_IS_PASSWD/value (true) to specify password field.
// Insert TAG_DEFAULT_VALUE/value ("123456") to specify default text for line edit.

OnyxPasswordDialog::OnyxPasswordDialog(QWidget *parent, const ODatas &ds,
        const QString &title, const QString &default_passwd_label)
    : OnyxDialog(parent)
    , big_layout_(&content_widget_)
    , line_edit_layout_(0)
    , sub_menu_(0, this)
    , show_plain_text_(0, this)
    , keyboard_(this)
    , title_(title)
    , default_passwd_label_(default_passwd_label)
    , edit_list_(ds)
{
    appendDefaultPasswordEdit();

    createLayout();
    connectWithChildren();

    updateTitle(tr("Password "));
    if (!title_.isEmpty())
    {
        updateTitle(title_);
    }
    onyx::screen::watcher().addWatcher(this);
}

OnyxPasswordDialog::~OnyxPasswordDialog()
{
    foreach (ODatas *edit_datas, all_line_edit_datas_)
    {
        clearDatas(*edit_datas);
        delete edit_datas;
    }
    clearDatas(sub_menu_datas_);
    clearDatas(show_plain_text_datas_);
}

void OnyxPasswordDialog::appendDefaultPasswordEdit()
{
    ODataPtr dd(new OData);
    dd->insert(TAG_TITLE, default_passwd_label_);
    dd->insert(TAG_IS_PASSWD, true);
    edit_list_.append(dd);
}

void OnyxPasswordDialog::addLineEditsToGroup()
{
    foreach (CatalogView *edit_item, edit_view_list_)
    {
        LineEditView * line_edit = static_cast<LineEditView *>(
                edit_item->visibleSubItems().front());
        edit_view_group_.addEdit(line_edit);
    }
}

bool OnyxPasswordDialog::popup(const QString &password)
{
    // set given password
    QVector<ContentView *> sub_items = edit_view_list_.back()->visibleSubItems();
    if (sub_items.size() > 0)
    {
        OData * data = sub_items.front()->data();
        if (data)
        {
            data->insert(TAG_TITLE, password);
        }
        sub_items.front()->updateData(data, true);
    }

    if (isHidden())
    {
        show();
    }
    QWidget * widget = safeParentWidget(parentWidget());
    resize(widget->width(), height());
    move(widget->x(), widget->height() - height());

    if (0 == edit_view_group_.editList().size())
    {
        addLineEditsToGroup();
    }

    return exec();
}

QString OnyxPasswordDialog::value(OData * d_index)
{
    CatalogView *target = edit_view_list_.back();
    if (0 != d_index)
    {
        int index = edit_list_.indexOf(ODataPtr(d_index));
        target = edit_view_list_.at(index);
    }

    LineEditView * item = static_cast<LineEditView *>(
            target->visibleSubItems().front());
    return item->innerEdit()->text();
}

void OnyxPasswordDialog::createLayout()
{
    vbox_.setSpacing(0);
    content_widget_.setBackgroundRole(QPalette::Button);
    content_widget_.setContentsMargins(0, 0, 0, 0);
    big_layout_.setContentsMargins(2, 2, 2, 2);
    big_layout_.setSpacing(0);

    QWidget *pwidget = safeParentWidget(parentWidget());
    int sub_menu_width = defaultItemHeight()*5;
    int line_edit_width = pwidget->width()-LABEL_WIDTH-sub_menu_width-5;

    createLineEdits(line_edit_width);
    createSubMenu(sub_menu_width);
    createShowPlainText();

    int size = edit_list_.size();
    for (int i=0; i<size; i++)
    {
        ODataPtr data = edit_list_.at(i);
        QString label_text = data->value(TAG_TITLE).toString();
        OnyxLabel *label = new OnyxLabel(label_text);
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        label->setFont(font);
        label->setFixedWidth(LABEL_WIDTH);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        line_edit_layout_ = new QHBoxLayout;
        line_edit_layout_->addWidget(label, 0, Qt::AlignLeft);
        line_edit_layout_->addWidget(edit_view_list_.at(i), 0, Qt::AlignLeft);
        line_edit_layout_->addSpacing(SPACING);
        if (0 == i)
        {
            line_edit_layout_->addWidget(&sub_menu_, 0, Qt::AlignLeft);
        }
        else
        {
            OnyxLabel *dummy = new OnyxLabel("");
            dummy->setFixedWidth(sub_menu_.width());
            line_edit_layout_->addWidget(dummy, 0, Qt::AlignLeft);
        }
        big_layout_.addLayout(line_edit_layout_);
    }
    big_layout_.addWidget(&show_plain_text_);
    big_layout_.addWidget(&keyboard_);
}

CatalogView * OnyxPasswordDialog::createEditItem(OData *data, int index,
        ODatas *edit_datas, const int &line_edit_width)
{
    const int height = defaultItemHeight();
    CatalogView * edit_item = new CatalogView(0, this);
    edit_item->setSubItemType(LineEditView::type());
    edit_item->setPreferItemSize(QSize(height, height));

    ODataPtr dd(new OData);

    // copy the TAG_IS_PASSWD property
    if (data->contains(TAG_IS_PASSWD))
    {
        dd->insert(TAG_IS_PASSWD, data->value(TAG_IS_PASSWD).toBool());
    }

    // copy the TAG_DEFAULT_VALUE property to TAG_TITLE
    if (data->contains(TAG_DEFAULT_VALUE))
    {
        dd->insert(TAG_TITLE, data->value(TAG_DEFAULT_VALUE).toString());
    }

    // copy the TAG_DISABLED property
    if (data->contains(TAG_DISABLED))
    {
        dd->insert(TAG_DISABLED, data->value(TAG_DISABLED).toBool());
    }

    edit_datas->push_back(dd);

    edit_item->setFixedGrid(1, 1);
    edit_item->setMargin(OnyxKeyboard::CATALOG_MARGIN);
    edit_item->setFixedHeight(defaultItemHeight()+2*SPACING);
    edit_item->setFixedWidth(line_edit_width);
    edit_item->setData(*edit_datas);
    return edit_item;
}

void OnyxPasswordDialog::createLineEdits(const int &line_edit_width)
{
    int size = edit_list_.size();
    int default_checked = 0;
    for (int i=0; i<size; i++)
    {
        ODataPtr data = edit_list_.at(i);

        ODatas * edit_datas = new ODatas;
        CatalogView * edit_item = createEditItem(data, i, edit_datas,
                line_edit_width);
        all_line_edit_datas_.push_back(edit_datas);

        // set default checked edit item
        if (default_checked == i)
        {
            ODataPtr td = edit_item->data().front();
            if (td)
            {
                if (!td->contains(TAG_DISABLED) || !td->value(TAG_DISABLED).toBool())
                {
                    // set the TAG_CHECKED property
                    td->insert(TAG_CHECKED, true);
                    default_checked = -1;
                }
                else
                {
                    default_checked++;
                }
            }
        }

        if (!edit_view_list_.isEmpty())
        {
            edit_item->setNeighbor(edit_view_list_.back(), CatalogView::UP);
        }
        edit_item->setNeighbor(&sub_menu_, CatalogView::RIGHT);
        edit_item->setNeighbor(&sub_menu_, CatalogView::RECYCLE_RIGHT);
        edit_view_list_.push_back(edit_item);
    }
    if (!edit_view_list_.isEmpty())
    {
        edit_view_list_.front()->setNeighbor(keyboard_.menu(),
                CatalogView::RECYCLE_DOWN);
        edit_view_list_.back()->setNeighbor(&show_plain_text_,
                CatalogView::DOWN);
    }
}

void OnyxPasswordDialog::createSubMenu(const int &sub_menu_width)
{
    const int height = defaultItemHeight();
    sub_menu_.setPreferItemSize(QSize(height, height));

    ODataPtr dd(new OData);
    dd->insert(TAG_TITLE, tr("OK"));
    dd->insert(TAG_MENU_TYPE, OnyxKeyboard::KEYBOARD_MENU_OK);
    sub_menu_datas_.push_back(dd);

    ODataPtr b(new OData);
    b->insert(TAG_TITLE, tr("Clear"));
    b->insert(TAG_MENU_TYPE, OnyxKeyboard::KEYBOARD_MENU_CLEAR);
    sub_menu_datas_.push_back(b);

    sub_menu_.setFixedGrid(1, 2);
    sub_menu_.setMargin(OnyxKeyboard::CATALOG_MARGIN);
    sub_menu_.setFixedHeight(defaultItemHeight()+2*SPACING);
    sub_menu_.setFixedWidth(sub_menu_width);
    sub_menu_.setData(sub_menu_datas_);
    sub_menu_.setNeighbor(edit_view_list_.front(), CatalogView::LEFT);
    sub_menu_.setNeighbor(edit_view_list_.front(), CatalogView::RECYCLE_LEFT);
    sub_menu_.setNeighbor(&show_plain_text_, CatalogView::DOWN);
    sub_menu_.setNeighbor(keyboard_.menu(), CatalogView::RECYCLE_DOWN);
}

void OnyxPasswordDialog::createShowPlainText()
{
    const int height = defaultItemHeight();
    show_plain_text_.setSubItemType(CheckBoxView::type());
    show_plain_text_.setPreferItemSize(QSize(height, height));

    ODataPtr dd(new OData);
    dd->insert(TAG_TITLE, tr("Show Plain Text"));
    dd->insert(TAG_CHECKED, false);
    show_plain_text_datas_.push_back(dd);

    show_plain_text_.setFixedGrid(1, 1);
    show_plain_text_.setMargin(OnyxKeyboard::CATALOG_MARGIN);
    show_plain_text_.setFixedHeight(defaultItemHeight()+2*SPACING);
    show_plain_text_.setData(show_plain_text_datas_);
    show_plain_text_.setNeighbor(edit_view_list_.back(), CatalogView::UP);
    show_plain_text_.setNeighbor(keyboard_.top(), CatalogView::DOWN);
}

void OnyxPasswordDialog::connectWithChildren()
{
    connect(&sub_menu_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&show_plain_text_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}

void OnyxPasswordDialog::clearClicked()
{
    foreach (CatalogView *edit_item, edit_view_list_)
    {
        LineEditView *edit = static_cast<LineEditView *>(
                edit_item->visibleSubItems().front());
        edit->innerEdit()->clear();
    }
}

void OnyxPasswordDialog::setEditEchoMode(QLineEdit::EchoMode mode)
{
    foreach (CatalogView *edit_item, edit_view_list_)
    {
        LineEditView *edit = static_cast<LineEditView *>(
                edit_item->visibleSubItems().front());
        if (edit->data()->contains(TAG_IS_PASSWD))
        {
            if (QLineEdit::Normal == mode)
            {
                edit->data()->insert(TAG_IS_PASSWD, false);
            }
            else
            {
                edit->data()->insert(TAG_IS_PASSWD, true);
            }
        }
        edit->repaint();
    }
}

void OnyxPasswordDialog::showPlainTextClicked(bool target_value)
{
    if (target_value)
    {
        setEditEchoMode(QLineEdit::Normal);
    }
    else
    {
        setEditEchoMode(QLineEdit::Password);
    }
}

void OnyxPasswordDialog::keyPressEvent(QKeyEvent *event)
{
    if (0 == edit_view_group_.editList().size())
    {
        addLineEditsToGroup();
    }

    int key = event->key();
    if (Qt::Key_Up != key
            && Qt::Key_Down != key
            && Qt::Key_Left != key
            && Qt::Key_Right != key
            && Qt::Key_Return != key
            && Qt::Key_Enter != key)
    {
        QApplication::sendEvent(edit_view_group_.checkedEdit(), event);
    }
}

void OnyxPasswordDialog::onItemActivated(CatalogView *catalog,
                                   ContentView *item,
                                   int user_data)
{
    OData * item_data = item->data();
    if (item_data->contains(TAG_MENU_TYPE))
    {
        int menu_type = item->data()->value(TAG_MENU_TYPE).toInt();
        if(OnyxKeyboard::KEYBOARD_MENU_OK == menu_type)
        {
            accept();
        }
        else if(OnyxKeyboard::KEYBOARD_MENU_CLEAR == menu_type)
        {
            clearClicked();
            update();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW);
        }
    }
    else if (catalog == &show_plain_text_)
    {
        bool checked = false;
        ContentView * checkbox = show_plain_text_.visibleSubItems().front();
        if (checkbox->data()->contains(TAG_CHECKED))
        {
            checked = qVariantValue<bool>(checkbox->data()->value(TAG_CHECKED));
        }
        checkbox->data()->insert(TAG_CHECKED, !checked);
        showPlainTextClicked(!checked);
        checkbox->repaint();
        update();
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
    }
}

}   // namespace ui
