#include "onyx/screen/screen_proxy.h"
#include "proxy_settings_dialog.h"

const QString LABEL_STYLE = "           \
QLabel                                  \
{                                       \
     padding: 0px;                      \
     background: transparent;           \
     font: 16px ;                       \
     color: black;                      \
 }";

const QString LINE_EDIT_STYLE = "       \
QLineEdit                               \
{                                       \
    border: 1px solid dark;             \
    background: white;                  \
    selection-background-color: black;  \
    selection-color: white;             \
    font: 16px bold;                    \
    color: black;                       \
    border-width: 1px;                  \
    border-style: solid;                \
    border-radius: 3;                   \
    padding: 0px;                       \
    min-height: 25px;                   \
}                                       \
QLineEdit:disabled                      \
{                                       \
    border: 1px solid dark;             \
    background: white;                  \
    selection-background-color: black;  \
    selection-color: white;             \
    font: 16px bold;                    \
    color: dark;                       \
    border-width: 1px;                  \
    border-style: solid;                \
    border-radius: 3;                   \
    padding: 0px;                       \
    min-height: 25px;                   \
}";

static const QString CHECK_BOX_STYLE = "                \
QCheckBox {                                             \
    padding : 4px;                                     \
    font-size: 16px;                                    \
    background: white;                                  \
    border-width: 1px;                                  \
    border-color: gray;                                 \
    border-style: solid;                                \
    border-radius: 3;                                   \
 }                                                      \
 QCheckBox:checked {                                    \
    background: #a3a3a3;                                \
 }                                                      \
 QCheckBox:focus {                                      \
    border-width: 2px;                                  \
    border-color: black;                                 \
    border-style: solid;                                \
    border-radius: 3;                                   \
 }                                                      \
 QCheckBox::indicator {                                 \
     width: 25px;                                       \
     height: 25px;                                      \
 }                                                      \
 QCheckBox::indicator:unchecked {                       \
     image: url(:/images/check_box_normal.png);         \
 }                                                      \
 QCheckBox::indicator:unchecked:hover {                 \
     image: url(:/images/check_box_normal.png);         \
 }                                                      \
 QCheckBox::indicator:unchecked:pressed {               \
     image: url(:/images/check_box_normal.png);         \
 }                                                      \
 QCheckBox::indicator:checked {                         \
     image: url(:/images/check_box_checked.png);        \
 }                                                      \
 QCheckBox::indicator:checked:hover {                   \
     image: url(:/images/check_box_checked.png);        \
 }                                                      \
 QCheckBox::indicator:checked:pressed {                 \
     image: url(:/images/check_box_checked.png);        \
 }                                                      \
";

using namespace ui;

namespace network_service
{

ProxySettingsDialog::ProxySettingsDialog(QWidget *parent)
    : OnyxDialog(parent)
    , content_vbox_(&content_widget_)
    , grid_layout_(0)
    , form_layout_(0)
    , exception_layout_(0)
    , hor_layout_(0)
    , use_proxy_button_(QApplication::tr("Use Proxy"), 0)
    , type_label_(QApplication::tr("Type"), 0)
    , host_label_(QApplication::tr("Host"), 0)
    , port_label_(QApplication::tr("Port"), 0)
    , user_name_label_(QApplication::tr("User Name"), 0)
    , password_label_(QApplication::tr("Password"), 0)
    , exceptions_label1_(QApplication::tr("Do not use proxy server for addresses beginning with:"), 0)
    , exceptions_label2_(QApplication::tr("Use semicolons(;) to seperate entries"), 0)
    , socks5_button_(QApplication::tr("Socks5"), 0)
    , http_button_(QApplication::tr("Http"), 0)
    , use_exceptions_button_(QApplication::tr("Exceptions"), 0)
    , host_edit_("", 0)
    , port_edit_("", 0)
    , user_name_edit_("", 0)
    , password_edit_("", 0)
    , exceptions_edit_("", 0)
    , ok_button_(QApplication::tr("OK"), 0)
    , keyboard_(this)
    , validator_(this)
    , keyboard_receiver_(0)
{
#ifndef Q_WS_QWS
    resize(600, 800);
#else
    resize(qApp->desktop()->screenGeometry().size());
#endif

    type_label_.setStyleSheet(LABEL_STYLE);
    host_label_.setStyleSheet(LABEL_STYLE);
    port_label_.setStyleSheet(LABEL_STYLE);
    user_name_label_.setStyleSheet(LABEL_STYLE);
    password_label_.setStyleSheet(LABEL_STYLE);
    exceptions_label1_.setStyleSheet(LABEL_STYLE);
    exceptions_label2_.setStyleSheet(LABEL_STYLE);

    //host_edit_.setStyleSheet(LINE_EDIT_STYLE);
    //port_edit_.setStyleSheet(LINE_EDIT_STYLE);
    //user_name_edit_.setStyleSheet(LINE_EDIT_STYLE);
    //password_edit_.setStyleSheet(LINE_EDIT_STYLE);
    //exceptions_edit_.setStyleSheet(LINE_EDIT_STYLE);

    socks5_button_.setStyleSheet(CHECK_BOX_STYLE);
    socks5_button_.setFixedHeight(32);
    http_button_.setStyleSheet(CHECK_BOX_STYLE);
    http_button_.setFixedHeight(32);
    use_proxy_button_.setStyleSheet(CHECK_BOX_STYLE);
    use_proxy_button_.setFixedHeight(32);
    use_exceptions_button_.setStyleSheet(CHECK_BOX_STYLE);
    use_exceptions_button_.setFixedHeight(32);

    createLayout();
    ok_button_.setFixedSize(QSize(42, 34));

    loadFromSettings();

    // Widget attributes.
    setModal(true);
    setFocusPolicy(Qt::NoFocus);
    content_widget_.setBackgroundRole(QPalette::Button);
    content_widget_.setFixedHeight(290);

    updateTitle(tr("Proxy Configuration"));
}

ProxySettingsDialog::~ProxySettingsDialog(void)
{
}

int ProxySettingsDialog::popup()
{
    onUseProxyStateChanged(use_proxy_button_.checkState());
    onUseExceptionStateChanged(use_proxy_button_.checkState());

    shadows_.show(false);
    show();
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(
        0,
        outbounding(parentWidget()),
        onyx::screen::ScreenProxy::GC,
        false,
        onyx::screen::ScreenCommand::WAIT_ALL);
    return OnyxDialog::exec();
}

void ProxySettingsDialog::loadFromSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("proxy"));

    use_proxy_button_.setChecked(settings.value(QLatin1String("enabled"), false).toBool());

    int type = settings.value(QLatin1String("type"), 0).toInt();
    if (type == 0)
    {
        socks5_button_.setChecked(true);
    }
    else
    {
        http_button_.setChecked(true);
    }

    host_edit_.setText(settings.value(QLatin1String("hostName")).toString());

    validator_.setRange(1, 65535);
    port_edit_.setValidator(&validator_);
    QString port_str;
    port_str.setNum(settings.value(QLatin1String("port"), 1080).toInt());
    port_edit_.setText(port_str);

    user_name_edit_.setText(settings.value(QLatin1String("userName")).toString());

    password_edit_.setText(settings.value(QLatin1String("password")).toString());

    use_exceptions_button_.setChecked(settings.value(QLatin1String("useExceptions"), false).toBool());
    exceptions_edit_.setText(settings.value(QLatin1String("exceptions")).toString());
    settings.endGroup();
}

void ProxySettingsDialog::saveToSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("proxy"));
    settings.setValue(QLatin1String("enabled"), use_proxy_button_.isChecked());

    int type = socks5_button_.isChecked() ? 0 : 1;
    settings.setValue(QLatin1String("type"), type);

    settings.setValue(QLatin1String("hostName"), host_edit_.text());
    settings.setValue(QLatin1String("port"), port_edit_.text());
    settings.setValue(QLatin1String("userName"), user_name_edit_.text());
    settings.setValue(QLatin1String("password"), password_edit_.text());
    settings.setValue(QLatin1String("useExceptions"), use_exceptions_button_.isChecked());
    settings.setValue(QLatin1String("Exceptions"), exceptions_edit_.text());
    settings.endGroup();
}

void ProxySettingsDialog::createLayout()
{
    // Title
    title_icon_label_.setPixmap(QPixmap(":/images/network_connection.png"));
    content_vbox_.setContentsMargins(SPACING * 4, SPACING, SPACING * 4, SPACING);
    content_vbox_.setSpacing(SPACING * 2);

    hor_layout_.setContentsMargins(0, 0, 0, 0);
    hor_layout_.setSpacing(SPACING * 2);

    grid_layout_.setContentsMargins(0, 0, 0, 0);
    grid_layout_.setSpacing(SPACING * 2);

    form_layout_.setContentsMargins(0, 0, 0, 0);
    form_layout_.setSpacing(SPACING * 2);

    exception_layout_.setContentsMargins(0, 0, 0, 0);
    exception_layout_.setSpacing(SPACING * 2);

    // use proxy?
    use_proxy_button_.selectOnClicked(false);
    ok_button_.useDefaultHeight();
    ok_button_.setCheckable(false);
    ok_button_.setFocusPolicy(Qt::TabFocus);
    hor_layout_.addWidget(&use_proxy_button_);
    hor_layout_.addWidget(&ok_button_);

    //exception_layout_.addStretch(0);
    content_vbox_.addLayout(&hor_layout_);

    // grid
    grid_layout_.addWidget(&type_label_, 0, 0);
    grid_layout_.addWidget(&socks5_button_, 0, 1);
    grid_layout_.addWidget(&http_button_, 0, 2);
    grid_layout_.addWidget(&host_label_, 1, 0);
    grid_layout_.addWidget(&host_edit_, 1, 1);
    grid_layout_.addWidget(&port_label_, 1, 2);
    grid_layout_.addWidget(&port_edit_, 1, 3);
    content_vbox_.addLayout(&grid_layout_);
    types_button_group_.addButton(&socks5_button_);
    types_button_group_.addButton(&http_button_);

    // form
    form_layout_.addRow(&user_name_label_, &user_name_edit_);
    form_layout_.addRow(&password_label_, &password_edit_);
    content_vbox_.addLayout(&form_layout_);

    // exception
    exception_layout_.addWidget(&use_exceptions_button_);
    exception_layout_.addWidget(&exceptions_label1_);
    exception_layout_.addWidget(&exceptions_edit_);
    //exception_layout_.addWidget(&exceptions_label2_);
    use_exceptions_button_.selectOnClicked(false);
    content_vbox_.addLayout(&exception_layout_);

    // keyboard
    keyboard_.attachReceiver(this);
    vbox_.addWidget(&keyboard_);

    // connect edit
    connect(&host_edit_, SIGNAL(getFocus(OnyxLineEdit *)), this, SLOT(onGetFocus(OnyxLineEdit *)));
    connect(&port_edit_, SIGNAL(getFocus(OnyxLineEdit *)), this, SLOT(onGetFocus(OnyxLineEdit *)));
    connect(&user_name_edit_, SIGNAL(getFocus(OnyxLineEdit *)), this, SLOT(onGetFocus(OnyxLineEdit *)));
    connect(&password_edit_, SIGNAL(getFocus(OnyxLineEdit *)), this, SLOT(onGetFocus(OnyxLineEdit *)));
    connect(&exceptions_edit_, SIGNAL(getFocus(OnyxLineEdit *)), this, SLOT(onGetFocus(OnyxLineEdit *)));

    // connect check buttons
    connect(&use_proxy_button_, SIGNAL(stateChanged(int)), this, SLOT(onUseProxyStateChanged(int)), Qt::QueuedConnection);
    connect(&use_exceptions_button_, SIGNAL(stateChanged(int)), this, SLOT(onUseExceptionStateChanged(int)), Qt::QueuedConnection);

    // OK cancel buttons.
    connect(&ok_button_, SIGNAL(clicked(bool)), this, SLOT(onOkClicked(bool)));
}

void ProxySettingsDialog::onGetFocus(OnyxLineEdit *edit)
{
    keyboard_.attachReceiver(edit);
}

void ProxySettingsDialog::onUseProxyStateChanged(int state)
{
    bool use = !(state == Qt::Unchecked);
    socks5_button_.setEnabled(use);
    http_button_.setEnabled(use);
    use_exceptions_button_.setEnabled(use);
    host_edit_.setEnabled(use);
    port_edit_.setEnabled(use);
    user_name_edit_.setEnabled(use);
    password_edit_.setEnabled(use);
    exceptions_edit_.setEnabled(use);
}

void ProxySettingsDialog::onUseExceptionStateChanged(int state)
{
    bool use = !(state == Qt::Unchecked);
    exceptions_edit_.setEnabled(use);
}

void ProxySettingsDialog::onOkClicked(bool ok)
{
    saveToSettings();
    done(QDialog::Accepted);
}

void ProxySettingsDialog::mouseMoveEvent(QMouseEvent *me)
{
    me->accept();
}

void ProxySettingsDialog::mousePressEvent(QMouseEvent *me)
{
    me->accept();
}

void ProxySettingsDialog::mouseReleaseEvent(QMouseEvent *me)
{
    me->accept();
}

void ProxySettingsDialog::keyReleaseEvent(QKeyEvent *ke)
{
    int key = ke->key();
    if (key == Qt::Key_Escape || key == ui::Device_Menu_Key)
    {
        onCloseClicked();
        return;
    }
}

/// The keyPressEvent could be sent from virtual keyboard.
void ProxySettingsDialog::keyPressEvent(QKeyEvent * ke)
{
    ke->accept();
    if (ke->key() == Qt::Key_Enter)
    {
        return;
    }
    else if (ke->key() == Qt::Key_Shift || ke->key() == Qt::Key_CapsLock)
    {
        return;
    }

    // Disable the parent widget to update screen.
    QKeyEvent * key_event = new QKeyEvent(ke->type(), ke->key(), ke->modifiers(), ke->text());
    QApplication::postEvent(receiver(), key_event);

    // Can not use flush here, could be caused by the keyboard event.
    onyx::screen::instance().enableUpdate(false);
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW);
}

bool ProxySettingsDialog::event(QEvent * event)
{
    bool ret = OnyxDialog::event(event);
    if (event->type() == QEvent::UpdateRequest && onyx::screen::instance().isUpdateEnabled())
    {
        onyx::screen::instance().sync(&shadows_.hor_shadow());
        onyx::screen::instance().sync(&shadows_.ver_shadow());
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
    }
    return ret;
}

void ProxySettingsDialog::moveEvent(QMoveEvent *e)
{
    OnyxDialog::moveEvent(e);
}

void ProxySettingsDialog::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    shadows_.show(false);
    update();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

}
