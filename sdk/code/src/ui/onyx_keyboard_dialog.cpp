#include "onyx/ui/onyx_keyboard_dialog.h"
#include "onyx/ui/onyx_keyboard_utils.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/ui_utils.h"

namespace ui
{

// The text of "OK" item can be changed by specifying ok_button_text.
OnyxKeyboardDialog::OnyxKeyboardDialog(QWidget *parent,
                                       const QString &title)
    : OnyxDialog(parent)
    , big_layout_(&content_widget_)
    , line_edit_layout_(0)
    , line_edit_(0, this)
    , sub_menu_(0, this)
    , keyboard_(this)
    , title_(title)
    , ok_button_text_(tr("OK"))
    , input_text_("")
{
    createLayout();
    connectWithChildren();
    onyx::screen::watcher().addWatcher(this);
}

OnyxKeyboardDialog::~OnyxKeyboardDialog()
{
    clearDatas(line_edit_datas_);
    clearDatas(sub_menu_datas_);
}

void OnyxKeyboardDialog::setOKButtonText(const QString& button_text)
{
    ok_button_text_ = button_text;
}

QString OnyxKeyboardDialog::popup(const QString &text)
{
    if (isHidden())
    {
        show();
    }

    QRect rc = qApp->desktop()->screenGeometry();
    resize(rc.width(), height());
    move(rc.x(), rc.height() - height());

    LineEditView *input = static_cast<LineEditView *>(
            line_edit_.visibleSubItems().front());
    input->innerEdit()->setText(text);

    int is_accepted = exec();
    if (QDialog::Accepted == is_accepted)
    {
        return getText();
    }
    else
    {
        return QString();
    }
}

QString OnyxKeyboardDialog::getText()
{
    LineEditView *input = static_cast<LineEditView *>(
                        line_edit_.visibleSubItems().front());
    return input->innerEdit()->text();
}

QString OnyxKeyboardDialog::inputText()
{
    return input_text_;
}

void OnyxKeyboardDialog::createLineEdit()
{
    line_edit_.setSubItemType(LineEditView::type());
    line_edit_.setPreferItemSize(QSize(rect().width(), defaultItemHeight()));

    ODataPtr dd(new OData);
    dd->insert(TAG_TITLE, "");
    line_edit_datas_.push_back(dd);

    line_edit_.setFixedGrid(1, 1);
    line_edit_.setFixedHeight(defaultItemHeight()+2*SPACING);
    line_edit_.setMargin(OnyxKeyboard::CATALOG_MARGIN);
    line_edit_.setData(line_edit_datas_);
    line_edit_.setNeighbor(keyboard_.top(), CatalogView::DOWN);
    line_edit_.setNeighbor(keyboard_.menu(), CatalogView::RECYCLE_DOWN);
    line_edit_.setNeighbor(&sub_menu_, CatalogView::RIGHT);
    line_edit_.setNeighbor(&sub_menu_, CatalogView::RECYCLE_RIGHT);
}

void OnyxKeyboardDialog::createSubMenu()
{
    const int height = defaultItemHeight();
    sub_menu_.setPreferItemSize(QSize(height, height));

    ODataPtr dd(new OData);
    dd->insert(TAG_TITLE, ok_button_text_);
    dd->insert(TAG_MENU_TYPE, OnyxKeyboard::KEYBOARD_MENU_OK);
    sub_menu_datas_.push_back(dd);

    ODataPtr b(new OData);
    b->insert(TAG_TITLE, tr("Clear"));
    b->insert(TAG_MENU_TYPE, OnyxKeyboard::KEYBOARD_MENU_CLEAR);
    sub_menu_datas_.push_back(b);

    sub_menu_.setSpacing(2);
    sub_menu_.setFixedGrid(1, 2);
    sub_menu_.setMargin(OnyxKeyboard::CATALOG_MARGIN, OnyxKeyboard::CATALOG_MARGIN, OnyxKeyboard::CATALOG_MARGIN, OnyxKeyboard::CATALOG_MARGIN);
    sub_menu_.setFixedHeight(defaultItemHeight()+2*SPACING);
    sub_menu_.setFixedWidth(defaultItemHeight()*6);
    sub_menu_.setData(sub_menu_datas_);
    sub_menu_.setNeighbor(&line_edit_, CatalogView::RECYCLE_LEFT);
    sub_menu_.setNeighbor(keyboard_.top(), CatalogView::DOWN);
    sub_menu_.setNeighbor(keyboard_.menu(), CatalogView::RECYCLE_DOWN);
}

void OnyxKeyboardDialog::createLayout()
{
    vbox_.setSpacing(0);
    content_widget_.setBackgroundRole(QPalette::Button);
    content_widget_.setContentsMargins(0, 0, 0, 0);
    updateTitle(title_);

    createLineEdit();
    createSubMenu();

    line_edit_layout_.setContentsMargins(0, 2, 0, 0);
    line_edit_layout_.addWidget(&line_edit_);
    line_edit_layout_.setSpacing(2);
    line_edit_layout_.addWidget(&sub_menu_);

    big_layout_.setContentsMargins(2, 2, 2, 2);
    big_layout_.setSpacing(0);
    big_layout_.addLayout(&line_edit_layout_);
    big_layout_.addWidget(&keyboard_);
}

void OnyxKeyboardDialog::onItemActivated(CatalogView *catalog,
                                   ContentView *item,
                                   int user_data)
{
    OData * item_data = item->data();
    if (item_data->contains(TAG_MENU_TYPE))
    {
        int menu_type = item->data()->value(TAG_MENU_TYPE).toInt();
        if(OnyxKeyboard::KEYBOARD_MENU_OK == menu_type)
        {
            input_text_ = getText();
            accept();
        }
        else if(OnyxKeyboard::KEYBOARD_MENU_CLEAR == menu_type)
        {
            input_text_ = "";
            clearClicked();
            update();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW);
        }
    }
}

void OnyxKeyboardDialog::connectWithChildren()
{
    connect(&line_edit_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&sub_menu_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}

void OnyxKeyboardDialog::clearClicked()
{
    LineEditView *input = static_cast<LineEditView *>(
            line_edit_.visibleSubItems().front());
    input->innerEdit()->clear();
}

void OnyxKeyboardDialog::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (Qt::Key_Up != key
            && Qt::Key_Down != key
            && Qt::Key_Left != key
            && Qt::Key_Right != key)
    {
        QApplication::sendEvent(line_edit_.visibleSubItems().front(), event);
    }
}

}   // namespace ui
