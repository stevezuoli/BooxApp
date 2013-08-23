#include "onyx/screen/screen_proxy.h"
#include "startup_dialog.h"
#include "onyx/ui/keyboard_navigator.h"

namespace explorer
{

namespace view
{

static const QString KEY_FOR_MISC_CONF = "startup_setting_type";
static const char* SCOPE = "startup_setting";
struct SettingType
{
    const char * title;
    int type;
};

/// Define all startup types.
static const SettingType STARTUP_ITEMS[] =
{
    {QT_TRANSLATE_NOOP("startup_setting","open most-recent document"), 0},
    {QT_TRANSLATE_NOOP("startup_setting","start at home screen"), 1},
};
static const int STARTUP_COUNT = sizeof(STARTUP_ITEMS) / sizeof(STARTUP_ITEMS[0]);

StartupDialog::StartupDialog(QWidget *parent, sys::SystemConfig & ref)
    : OnyxDialog(parent)
    , conf(ref)
    , ver_layout_(&content_widget_)
    , startup_layout_(0)
    , startup_group_(0)
    , hor_layout_(0)
    , ok_(QApplication::tr("OK"), 0)
    , sys_startup_type_(START_AT_HOME_SCREEN)
    , startup_type_(START_AT_HOME_SCREEN)
{
    setModal(true);
    resize(400, 200);
    createLayout();
}

StartupDialog::~StartupDialog(void)
{
}

int StartupDialog::exec()
{
    shadows_.show(true);
    show();
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(0, outbounding(parentWidget()),
            onyx::screen::ScreenProxy::GC, false,
            onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void StartupDialog::itemSelected()
{
    if (startup_type_ != sys_startup_type_)
    {
        QString type = QString::number(startup_type_);
        conf.setMiscValue(KEY_FOR_MISC_CONF, type);
    }
    accept();
}

void StartupDialog::setStartupType()
{
    int count = static_cast<int> (startup_buttons_.size());
    int i = 0;
    for (; i < count; ++i)
    {
        if (startup_buttons_[i]->isChecked())
        {
            startup_type_
                    = (StartupDialog::StartupSettingType) STARTUP_ITEMS[i].type;
            return;
        }
    }
    startup_type_ = (StartupDialog::StartupSettingType)0;
}

void StartupDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void StartupDialog::keyReleaseEvent(QKeyEvent *ke)
{
    QWidget *wnd = 0;
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
        wnd = ui::moveFocus(&content_widget_, ke->key());
        if (wnd)
        {
            wnd->setFocus();
        }
        break;
    case Qt::Key_Return:
    {
        onReturn();
        break;
    }
    case Qt::Key_Escape:
        reject();
        break;
    }
}

void StartupDialog::onReturn()
{
    size_t count = startup_buttons_.size();
    for (size_t i = 0; i < count; ++i)
    {
        if (startup_buttons_[i]->hasFocus())
        {
            startup_buttons_[i]->setChecked(true);
            QApplication::processEvents();
            setStartupType();
            onOkClicked(true);
            return;
        }
    }
}

void StartupDialog::onStartupButtonChanged(bool state)
{
    setStartupType();
}

QString StartupDialog::getKeyForMiscConf()
{
    return KEY_FOR_MISC_CONF;
}

StartupDialog::StartupSettingType StartupDialog::getStartupSettingType(const QString type)
{
    const int DEFAULT_STARTUP_SETTING = 0;
    bool ok;
    int typeInt = type.toInt(&ok, 10);
    if (!ok)
    {
        typeInt = DEFAULT_STARTUP_SETTING;
    }
    return (StartupDialog::StartupSettingType)typeInt;
}

void StartupDialog::createLayout()
{
    content_widget_.setBackgroundRole(QPalette::Button);

    // Retrieve the values from system status.
    sys_startup_type_ = getStartupSettingType(conf.miscValue(KEY_FOR_MISC_CONF));
    startup_type_ = sys_startup_type_;

    updateTitle(QApplication::tr("Startup Setting"));
    updateTitleIcon(QPixmap(":/images/startup.png"));
    // The big layout.
    ver_layout_.setContentsMargins(SPACING, 0, SPACING, 0);
    ver_layout_.addSpacing(3);

    // Startup layout
    startup_text_label_.setText(QApplication::tr("After Boox starts up:"));
    startup_text_label_.setWordWrap(true);
    startup_layout_.setContentsMargins(10, 0, 0, 0);
    startup_layout_.addWidget(&startup_text_label_);
    startup_layout_.addStretch(0);

    OnyxCheckBox * btn = 0;
    int index = 0;
    bool set_intial_focus = true;
    for (int row = 0; row < STARTUP_COUNT; ++row, ++index)
    {
        btn = new OnyxCheckBox(qApp->translate(SCOPE,
                STARTUP_ITEMS[index].title), 0);
        startup_group_.addButton(btn);
        startup_buttons_.push_back(btn);
        if (sys_startup_type_ == STARTUP_ITEMS[index].type)
        {
            btn->setFocus();
            btn->setChecked(true);
            set_intial_focus = false;
        }
        connect(btn, SIGNAL(clicked(bool)), this,
                SLOT(onStartupButtonChanged(bool)), Qt::QueuedConnection);
        startup_layout_.addWidget(btn);
    }

    if (set_intial_focus)
    {
        startup_buttons_[0]->setFocus();
    }

    ver_layout_.addLayout(&startup_layout_);

    // OK cancel buttons.
    connect(&ok_, SIGNAL(clicked(bool)), this, SLOT(onOkClicked(bool)));

    ok_.useDefaultHeight();
    ok_.setCheckable(false);
    ok_.setFocusPolicy(Qt::TabFocus);
    hor_layout_.addStretch(0);
    hor_layout_.addWidget(&ok_);

    ver_layout_.addStretch(0);
    ver_layout_.addLayout(&hor_layout_);
}

bool StartupDialog::event(QEvent *qe)
{
    bool ret = QDialog::event(qe);
    if (qe->type() == QEvent::UpdateRequest
            && onyx::screen::instance().isUpdateEnabled())
    {
        onyx::screen::instance().updateWidget(this,
                onyx::screen::ScreenProxy::DW);
    }
    return ret;
}

void StartupDialog::onOkClicked(bool)
{
    itemSelected();
}

}   // namespace view

}   // namespace explorer
