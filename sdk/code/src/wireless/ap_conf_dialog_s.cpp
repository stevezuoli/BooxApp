#include "onyx/ui/keyboard_navigator.h"
#include "onyx/wireless/ap_conf_dialog_s.h"
#include "onyx/data/data_tags.h"
#include "onyx/ui/content_view.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/ui_utils.h"

namespace ui
{

static const int LABEL_WIDTH = 80;


ApConfigDialogS::ApConfigDialogS(QWidget *parent, WifiProfile & profile)
    : OnyxDialog(parent)
    , big_layout_(&content_widget_)
    , form_layout_(0)
    , auth_hbox_(0)
    , enc_hbox_(0)
    , line_edit_layout_(0)
    , auth_label_(tr("Authentication")+":", 0)
    , enc_label_(tr("Encryption")+":", 0)
    , auth_group_(0)
    , plain_button_(tr("None"), 0)
    , wep_button_(tr("WEP"), 0)
    , wpa_psk_button_(tr("WPA"), 0)
    , wpa2_psk_button_(tr("WPA2"), 0)
    , enc_tkip_button_(tr("TKIP"), 0)
    , enc_ccmp_button_(tr("AES"), 0)
    , sub_menu_(0, this)
    , show_plain_text_(0, this)
    , keyboard_(this)
    , profile_(profile)
{
    keyboard_.setKeyboardLanguae(QLocale::English);

    if (isSsidEmpty())
    {
        ODataPtr data_ssid(new OData);
        data_ssid->insert(TAG_TITLE, tr("SSID"));
        data_ssid->insert(TAG_DEFAULT_VALUE, profile_.ssid());
        edit_list_.push_back(data_ssid);
    }

    ODataPtr data_psk = new OData;
    data_psk->insert(TAG_TITLE, tr("PSK"));
    data_psk->insert(TAG_DEFAULT_VALUE, profile_.psk());
    data_psk->insert(TAG_IS_PASSWD, true);
    if (!isSsidEmpty())
    {
        data_psk->insert(TAG_CHECKED, true);
    }
    edit_list_.push_back(data_psk);

    createLayout();
    connectWithChildren();

    updateTitle(tr("Wifi Configuration"));
    onyx::screen::watcher().addWatcher(this);
}

ApConfigDialogS::~ApConfigDialogS()
{
    foreach (ODatas *edit_datas, all_line_edit_datas_)
    {
        clearDatas(*edit_datas);
        delete edit_datas;
    }
    clearDatas(sub_menu_datas_);
    clearDatas(show_plain_text_datas_);
}

void ApConfigDialogS::onPlainButtonClicked()
{
    enc_tkip_button_.setEnabled(false);
    enc_ccmp_button_.setEnabled(false);
    // TODO
//    psk_edit_.setText("");
//    psk_edit_.setEnabled(false);

    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
}

void ApConfigDialogS::onWepButtonClicked()
{
    enc_tkip_button_.setEnabled(false);
    enc_ccmp_button_.setEnabled(false);
    // TODO
//    psk_edit_.setEnabled(true);

    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
}

void ApConfigDialogS::onWpaButtonClicked()
{
    enc_tkip_button_.setEnabled(true);
    enc_ccmp_button_.setEnabled(true);
    // TODO
//    psk_edit_.setEnabled(true);
    if (profile_.pairwise().contains("TKIP", Qt::CaseInsensitive))
    {
        enc_tkip_button_.setChecked(true);
    }
    if (profile_.pairwise().contains("CCMP", Qt::CaseInsensitive))
    {
        enc_ccmp_button_.setChecked(true);
    }

    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
}

void ApConfigDialogS::onWpa2ButtonClicked()
{
    enc_tkip_button_.setEnabled(true);
    enc_ccmp_button_.setEnabled(true);
    // TODO
//    psk_edit_.setEnabled(true);
    if (profile_.pairwise().contains("TKIP", Qt::CaseInsensitive))
    {
        enc_tkip_button_.setChecked(true);
    }
    if (profile_.pairwise().contains("CCMP", Qt::CaseInsensitive))
    {
        enc_ccmp_button_.setChecked(true);
    }

    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
}

void ApConfigDialogS::addLineEditsToGroup()
{
    foreach (CatalogView *edit_item, edit_view_list_)
    {
        LineEditView * line_edit = static_cast<LineEditView *>(
                edit_item->visibleSubItems().front());
        edit_view_group_.addEdit(line_edit);
        line_edit->installEventFilter(this);
    }
}

bool ApConfigDialogS::popup()
{
    updateWidgets(profile_);

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

    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);

    return exec();
}

QString ApConfigDialogS::value(int d_index)
{
    CatalogView *target = edit_view_list_.back();
    if (-1 != d_index)
    {
        target = edit_view_list_.at(d_index);
    }

    LineEditView * item = static_cast<LineEditView *>(
            target->visibleSubItems().front());
    return item->innerEdit()->text();
}

void ApConfigDialogS::createInputs(int size)
{
    for (int i=0; i<size; i++)
    {
        ODataPtr data = edit_list_.at(i);
        QString label_text = data->value(TAG_TITLE).toString();
        OnyxLabel *label = new OnyxLabel(label_text+":");
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        label->setFont(font);
        label->setFixedWidth(LABEL_WIDTH);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        line_edit_layout_ = new QHBoxLayout;
        line_edit_layout_->addWidget(label);
        line_edit_layout_->addWidget(edit_view_list_.at(i));
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
}

void ApConfigDialogS::createLayout()
{
    vbox_.setSpacing(0);
    content_widget_.setBackgroundRole(QPalette::Button);
    content_widget_.setContentsMargins(0, 0, 0, 0);
    big_layout_.setContentsMargins(2, 2, 2, 2);
    big_layout_.setSpacing(0);

    QWidget *pwidget = safeParentWidget(parentWidget());
    int sub_menu_width = defaultItemHeight()*5;
    int line_edit_width = pwidget->width()-(LABEL_WIDTH+sub_menu_width)-5;

    Qt::Alignment align = static_cast<Qt::Alignment>(Qt::AlignLeft | Qt::AlignVCenter);
    form_layout_.setFormAlignment(align);
    form_layout_.setContentsMargins(4, 0, 4, 0);
    form_layout_.setVerticalSpacing(2);

    plain_button_.installEventFilter(this);
    wep_button_.installEventFilter(this);
    wpa_psk_button_.installEventFilter(this);
    wpa2_psk_button_.installEventFilter(this);
    enc_tkip_button_.installEventFilter(this);
    enc_ccmp_button_.installEventFilter(this);
    keyboard_.installEventFilter(this);

    auth_hbox_.setSpacing(4);
    auth_hbox_.setContentsMargins(4, 0, 0, 0);

    auth_hbox_.addWidget(&plain_button_, 0, 0);
    auth_hbox_.addWidget(&wep_button_, 0, 1);
    auth_hbox_.addWidget(&wpa_psk_button_, 0, 2);
    auth_hbox_.addWidget(&wpa2_psk_button_, 0, 3);
    auth_group_.addButton(&plain_button_);
    auth_group_.addButton(&wep_button_);
    auth_group_.addButton(&wpa_psk_button_);
    auth_group_.addButton(&wpa2_psk_button_);
    plain_button_.setCheckable(true);
    wep_button_.setCheckable(true);
    wpa_psk_button_.setCheckable(true);
    wpa2_psk_button_.setCheckable(true);

    enc_hbox_.setSpacing(4);
    enc_hbox_.setContentsMargins(4, 0, 0, 0);

    enc_hbox_.addWidget(&enc_tkip_button_, 0, 0);
    enc_hbox_.addWidget(&enc_ccmp_button_, 0, 1);
    enc_tkip_button_.setCheckable(true);
    enc_ccmp_button_.setCheckable(true);

    form_layout_.addRow(&auth_label_, &auth_hbox_);
    form_layout_.addRow(&enc_label_, &enc_hbox_);
    big_layout_.addLayout(&form_layout_);

    createLineEdits(line_edit_width);
    createSubMenu(sub_menu_width);
    createShowPlainText();

    int size = edit_list_.size();

    if (1 == size)
    {
        OnyxLabel *label = new OnyxLabel(tr("SSID")+":");
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        label->setFont(font);
        label->setFixedWidth(LABEL_WIDTH);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        OnyxLabel *ssid_label = new OnyxLabel(profile_.ssid());
        QFont it_font;
        it_font.setBold(true);
        it_font.setItalic(true);
        ssid_label->setFont(it_font);
        ssid_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        line_edit_layout_ = new QHBoxLayout;
        line_edit_layout_->addWidget(label);
        line_edit_layout_->setContentsMargins(4, 0, 0, 0);
        line_edit_layout_->addWidget(ssid_label, 400, Qt::AlignLeft);

        big_layout_.addLayout(line_edit_layout_);
    }

    createInputs(size);

    big_layout_.addWidget(&show_plain_text_);
    big_layout_.addWidget(&keyboard_);

    connect(&plain_button_, SIGNAL(clicked()), this, SLOT(onPlainButtonClicked()));
    connect(&wep_button_, SIGNAL(clicked()), this, SLOT(onWepButtonClicked()));
    connect(&wpa_psk_button_, SIGNAL(clicked()), this, SLOT(onWpaButtonClicked()));
    connect(&wpa2_psk_button_, SIGNAL(clicked()), this, SLOT(onWpa2ButtonClicked()));
}

CatalogView * ApConfigDialogS::createEditItem(OData *data, int index,
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

void ApConfigDialogS::createLineEdits(const int &line_edit_width)
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
        edit_view_list_.back()->setNeighbor(&show_plain_text_,
                CatalogView::DOWN);
    }
}

void ApConfigDialogS::createSubMenu(const int &sub_menu_width)
{
    const int height = defaultItemHeight();
    sub_menu_.setSubItemType(ButtonView::type());
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
    sub_menu_.setSpacing(8);
    sub_menu_.setMargin(OnyxKeyboard::CATALOG_MARGIN);
    sub_menu_.setFixedHeight(defaultItemHeight()+2*SPACING);
    sub_menu_.setFixedWidth(sub_menu_width);
    sub_menu_.setData(sub_menu_datas_);
    sub_menu_.setNeighbor(edit_view_list_.front(), CatalogView::LEFT);
    sub_menu_.setNeighbor(edit_view_list_.front(), CatalogView::RECYCLE_LEFT);
    sub_menu_.setNeighbor(&show_plain_text_, CatalogView::DOWN);
}

void ApConfigDialogS::createShowPlainText()
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

void ApConfigDialogS::updateWidgets(WifiProfile & profile)
{
    // Authentication
    if (profile.isWep())
    {
        wep_button_.setChecked(true);
        onWepButtonClicked();
    }
    else if (profile.isWpa())
    {
        wpa_psk_button_.setChecked(true);
        onWpaButtonClicked();
    }
    else if (profile.isWpa2())
    {
        wpa2_psk_button_.setChecked(true);
        onWpa2ButtonClicked();
    }
    else
    {
        plain_button_.setChecked(true);
        onPlainButtonClicked();
    }
}

void ApConfigDialogS::onOutOfDown(CatalogView* child, int, int)
{
    plain_button_.setFocus();
}

void ApConfigDialogS::connectWithChildren()
{
    connect(&sub_menu_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&show_plain_text_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));

    connect(&keyboard_, SIGNAL(outOfDown(CatalogView*, int, int)),
                this, SLOT(onOutOfDown(CatalogView*, int, int)));
}

void ApConfigDialogS::clearClicked()
{
    foreach (CatalogView *edit_item, edit_view_list_)
    {
        LineEditView *edit = static_cast<LineEditView *>(
                edit_item->visibleSubItems().front());
        edit->innerEdit()->clear();
    }
}

void ApConfigDialogS::setEditEchoMode(QLineEdit::EchoMode mode)
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

void ApConfigDialogS::showPlainTextClicked(bool target_value)
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

void ApConfigDialogS::keyPressEvent(QKeyEvent *event)
{
    if (0 == edit_view_group_.editList().size())
    {
        addLineEditsToGroup();
    }

    event->accept();
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

QString ApConfigDialogS::getSsidText()
{
    if (!isSsidEmpty())
    {
        return profile_.ssid();
    }
    return edit_view_group_.editList().first()->innerEdit()->text();
}

QString ApConfigDialogS::getPskText()
{
    return edit_view_group_.editList().last()->innerEdit()->text();
}

void ApConfigDialogS::setPlainProfile(WifiProfile & profile,
                                     const QString & id)
{
    QByteArray d = profile.bssid();
    profile.clear();
    profile.setBssid(d);
    profile.defineByUser(true);
    profile.setSsid(id);
    profile.setKeyMgmt("NONE");
}

void ApConfigDialogS::setWepProfile(WifiProfile & profile,
                                   const QString &id,
                                   const QString & key)
{
    QByteArray d = profile.bssid();
    profile.clear();
    profile.setBssid(d);
    profile.defineByUser(true);
    profile.setSsid(id);
    profile.setKeyMgmt("NONE");
    profile.setWep(true);
    profile.setWepKey1(key);
}

void ApConfigDialogS::setWpaProfile(WifiProfile & profile,
                                   const QString &id,
                                   const QString & key)
{
    QByteArray d = profile.bssid();
    profile.clear();
    profile.setBssid(d);
    profile.defineByUser(true);
    profile.setSsid(id);
    profile.setKeyMgmt("WPA-PSK");
    profile.setProto("WPA");
    profile.setWpa(true);
    QString t;
    if (enc_ccmp_button_.isChecked())
    {
        t.append("CCMP");
    }
    if (enc_tkip_button_.isChecked())
    {
        if (!t.isEmpty())
        {
            t.append(" ");
        }
        t.append("TKIP");
    }
    profile.setPairwise(t);
    profile.setPsk(key);
}

void ApConfigDialogS::setWpa2Profile(WifiProfile & profile,
                                    const QString &id,
                                    const QString & key)
{
    QByteArray d = profile.bssid();
    profile.clear();
    profile.setBssid(d);
    profile.defineByUser(true);
    profile.setSsid(id);
    profile.setKeyMgmt("WPA-PSK");
    profile.setProto("WPA2");
    profile.setWpa2(true);
    QString t;
    if (enc_ccmp_button_.isChecked())
    {
        t.append("CCMP");
    }
    if (enc_tkip_button_.isChecked())
    {
        if (!t.isEmpty())
        {
            t.append(" ");
        }
        t.append("TKIP");
    }
    profile.setPairwise(t);
    profile.setPsk(key);
}

bool ApConfigDialogS::isSsidEmpty()
{
    return profile_.ssid().isEmpty();
}

void ApConfigDialogS::onItemActivated(CatalogView *catalog,
                                   ContentView *item,
                                   int user_data)
{
    OData * item_data = item->data();
    if (item_data->contains(TAG_MENU_TYPE))
    {
        int menu_type = item->data()->value(TAG_MENU_TYPE).toInt();
        if(OnyxKeyboard::KEYBOARD_MENU_OK == menu_type)
        {
            QString ssid = this->getSsidText();
            QString psk = this->getPskText();
            qDebug() << "ssid: " << ssid <<", psk: " << psk;

            if (plain_button_.isChecked())
            {
                setPlainProfile(profile_, ssid);
            }
            else if (wep_button_.isChecked())
            {
                setWepProfile(profile_, ssid, psk);
            }
            else if (wpa_psk_button_.isChecked())
            {
                setWpaProfile(profile_, ssid, psk);
            }
            else if (wpa2_psk_button_.isChecked())
            {
                setWpa2Profile(profile_, ssid, psk);
            }
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

bool ApConfigDialogS::event(QEvent *event)
{
    bool ret = OnyxDialog::event(event);
    if (event->type() == QEvent::UpdateRequest && onyx::screen::instance().isUpdateEnabled())
    {
        onyx::screen::instance().updateWidget(
            this,
            onyx::screen::ScreenProxy::DW,
            false,
            onyx::screen::ScreenCommand::WAIT_NONE);
    }
    return ret;
}

bool ApConfigDialogS::eventFilter(QObject *obj, QEvent *event)
{
    if (QEvent::KeyRelease == event->type())
    {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        int key = key_event->key();

        if ((obj == &plain_button_) || (obj == &wep_button_) ||
                (obj == &wpa_psk_button_) || (obj == &wpa2_psk_button_))
        {
            if (key == Qt::Key_Right) {
                if (obj == &plain_button_) {
                    wep_button_.setFocus();
                }
                else if (obj == &wep_button_) {
                    wpa_psk_button_.setFocus();
                }
                else if (obj == &wpa_psk_button_) {
                    wpa2_psk_button_.setFocus();
                }
                else if (obj == &wpa2_psk_button_) {
                    plain_button_.setFocus();
                }
            }
            else if (key == Qt::Key_Left) {
                if (obj == &plain_button_) {
                    wpa2_psk_button_.setFocus();
                }
                else if (obj == &wep_button_) {
                    plain_button_.setFocus();
                }
                else if (obj == &wpa_psk_button_) {
                    wep_button_.setFocus();
                }
                else if (obj == &wpa2_psk_button_) {
                    wpa_psk_button_.setFocus();
                }
            }
            else if (key == Qt::Key_Up) {
                keyboard_.menu()->visibleSubItems().front()->setFocus();
            }
            else if (key == Qt::Key_Down) {
                if (enc_tkip_button_.isEnabled()) {
                    enc_tkip_button_.setFocus();
                }
                else {
                    edit_view_group_.editList().first()->setFocus();
                }
            }
        }
        else if ((obj == &enc_tkip_button_) || (obj == &enc_ccmp_button_))
        {
            if (key == Qt::Key_Down) {
                edit_view_group_.editList().first()->setFocus();
            }
            else {
                QWidget *wnd = ui::moveFocus(this, key);
                if (wnd)
                {
                    wnd->setFocus();
                }
            }
        }
        else if (obj == edit_view_group_.editList().first())
        {
            if (Qt::Key_Up == key)
            {
                if (enc_tkip_button_.isEnabled())
                {
                    enc_tkip_button_.setFocus();
                }
                else
                {
                    plain_button_.setFocus();
                }
            }
        }
    }
    return OnyxDialog::eventFilter(obj, event);
}

}   // namespace ui

///
/// \example wifi/ap_conf_main.cpp
/// This is an example of how to use the ApConfigDialogS class.
///
