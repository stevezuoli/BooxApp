#include "onyx/screen/screen_proxy.h"
#include "screen_update_dialog.h"
#include "onyx/ui/keyboard_navigator.h"
#include "onyx/sys/platform.h"

namespace explorer
{

namespace view
{

const QString ScreenUpdateDialog::SCREEN_UPDATE_KEY = "screen_update_setting";
const char* ScreenUpdateDialog::SCREEN_UPDATE_SCOPE = "screen_update_setting";

/// Define all screen update setting type
static const ScreenUpdateDialog::SettingType SCREEN_UPDATE_ITEMS[] =
{
    {QT_TRANSLATE_NOOP("screen_update_setting", "every 3 pages"), 3},
    {QT_TRANSLATE_NOOP("screen_update_setting", "every 5 pages"), 5},
    {QT_TRANSLATE_NOOP("screen_update_setting", "every 7 pages"), 7},
    {QT_TRANSLATE_NOOP("screen_update_setting", "every 9 pages"), 9},
    {QT_TRANSLATE_NOOP("screen_update_setting", "always"), 1},
};
const int ScreenUpdateDialog::SCREEN_UPDATE_COUNT = sizeof(SCREEN_UPDATE_ITEMS)
        / sizeof(SCREEN_UPDATE_ITEMS[0]);


const QString ScreenUpdateDialog::GRAY_SCALE_KEY = "gray_scale_setting";
const char* ScreenUpdateDialog::GRAY_SCALE_SCOPE = "gray_scale_setting";

/// Define all gray scale setting type
static const ScreenUpdateDialog::SettingType GRAY_SCALE_ITEMS[] =
{
    {QT_TRANSLATE_NOOP("gray_scale_setting", "speed first"), 8},
    {QT_TRANSLATE_NOOP("gray_scale_setting", "quality first"), 16},

};
const int ScreenUpdateDialog::GRAY_SCALE_COUNT = sizeof(GRAY_SCALE_ITEMS)
        / sizeof(GRAY_SCALE_ITEMS[0]);


ScreenUpdateDialog::ScreenUpdateDialog(QWidget *parent, sys::SystemConfig & ref)
    : OnyxDialog(parent)
    , conf(ref)
    , ver_layout_(&content_widget_)
    , screen_update_layout_(0)
    , screen_update_group_(0)
    , gray_scale_layout_(0)
    , gray_scale_group_(0)
    , hor_layout_(0)
    , ok_(QApplication::tr("OK"), 0)
    , sys_screen_update_setting_(ALWAYS)
    , screen_update_setting_(ALWAYS)
    , sys_gray_scale_setting_(SPEED_FIRST)
    , gray_scale_setting_(SPEED_FIRST)
{
    setModal(true);
    if (sys::is166E())
    {
        resize(400, 500);
    }
    else
    {
        resize(400, 380);
    }
    createLayout();
}

ScreenUpdateDialog::~ScreenUpdateDialog()
{
}

int ScreenUpdateDialog::exec()
{
    shadows_.show(true);
    show();
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(0, outbounding(parentWidget()),
            onyx::screen::ScreenProxy::GC, false,
            onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void ScreenUpdateDialog::setScreenUpdateSetting()
{
    int count = static_cast<int> (screen_update_buttons_.size());
    ScreenUpdateDialog::ScreenUpdateSettingType type = ALWAYS;
    int i = 0;
    for (; i < count; ++i)
    {
        if (screen_update_buttons_[i]->isChecked())
        {
            type = (ScreenUpdateDialog::ScreenUpdateSettingType) SCREEN_UPDATE_ITEMS[i].type;
            break;
        }
    }
    screen_update_setting_ = type;
}

void ScreenUpdateDialog::setGrayscaleSetting()
{
    int count = static_cast<int> (gray_scale_buttons_.size());
    ScreenUpdateDialog::GrayScaleSettingType type = SPEED_FIRST;
    int i = 0;
    for (; i < count; ++i)
    {
        if (gray_scale_buttons_[i]->isChecked())
        {
            type = (ScreenUpdateDialog::GrayScaleSettingType) GRAY_SCALE_ITEMS[i].type;
            break;
        }
    }
    gray_scale_setting_ = type;
}

void ScreenUpdateDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void ScreenUpdateDialog::keyReleaseEvent(QKeyEvent *ke)
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

ScreenUpdateDialog::ScreenUpdateSettingType ScreenUpdateDialog::getScreenUpdateSettingType(
        const QString type)
{
    const int DEFAULT_SCREEN_UPDATE_SETTING = ALWAYS;
    bool ok;
    int typeInt = type.toInt(&ok, 10);
    if (!ok)
    {
        typeInt = DEFAULT_SCREEN_UPDATE_SETTING;
    }
    return (ScreenUpdateDialog::ScreenUpdateSettingType)typeInt;
}

ScreenUpdateDialog::GrayScaleSettingType ScreenUpdateDialog::getGrayScaleSettingType(
        const QString type)
{
    const int DEFAULT_GRAY_SCALE_SETTING = SPEED_FIRST;
    bool ok;
    int typeInt = type.toInt(&ok, 10);
    if (!ok)
    {
        typeInt = DEFAULT_GRAY_SCALE_SETTING;
    }
    return (ScreenUpdateDialog::GrayScaleSettingType)typeInt;
}

QString ScreenUpdateDialog::getKeyForMiscConf()
{
    return SCREEN_UPDATE_KEY;
}

QString ScreenUpdateDialog::getGrayScaleSettingKey()
{
    return GRAY_SCALE_KEY;
}

void ScreenUpdateDialog::createScreenUpdateLayout()
{
    // Retrieve the values from system status.
    sys_screen_update_setting_ = getScreenUpdateSettingType(conf.miscValue(
            SCREEN_UPDATE_KEY));
    screen_update_setting_ = sys_screen_update_setting_;

    // Screen update layout
    screen_update_text_label_.setText(QApplication::tr("Fully refresh the screen:"));
    screen_update_text_label_.setWordWrap(true);
    screen_update_layout_.setContentsMargins(10, 0, 0, 0);
    screen_update_layout_.addWidget(&screen_update_text_label_);
    screen_update_layout_.addStretch(0);

    OnyxCheckBox * btn = 0;
    int index = 0;
    bool set_intial_focus = true;
    for (int row = 0; row < SCREEN_UPDATE_COUNT; ++row, ++index)
    {
        btn = new OnyxCheckBox(qApp->translate(SCREEN_UPDATE_SCOPE,
                SCREEN_UPDATE_ITEMS[index].title), 0);
        screen_update_group_.addButton(btn);
        screen_update_buttons_.push_back(btn);
        if (sys_screen_update_setting_ == SCREEN_UPDATE_ITEMS[index].type)
        {
            btn->setFocus();
            btn->setChecked(true);
            set_intial_focus = false;
        }
        connect(btn, SIGNAL(clicked(bool)), this,
                SLOT(onScreenUpdateButtonChanged(bool)), Qt::QueuedConnection);
        screen_update_layout_.addWidget(btn);
    }

    if (set_intial_focus)
    {
        screen_update_buttons_[0]->setFocus();
    }

    ver_layout_.addLayout(&screen_update_layout_);
}

void ScreenUpdateDialog::createGrayScaleLayout()
{
    // Retrieve the values from system status.
    sys_gray_scale_setting_ = getGrayScaleSettingType(conf.miscValue(
            GRAY_SCALE_KEY));
    gray_scale_setting_ = sys_gray_scale_setting_;

    // gray scale layout
    gray_scale_text_label_.setText(QApplication::tr(
            "Refresh the screen by:"));
    gray_scale_text_label_.setWordWrap(true);
    gray_scale_layout_.setContentsMargins(10, 0, 0, 0);
    gray_scale_layout_.addWidget(&gray_scale_text_label_);
    gray_scale_layout_.addStretch(0);

    OnyxCheckBox * btn = 0;
    int index = 0;
    bool set_intial_focus = true;
    for (int row = 0; row < GRAY_SCALE_COUNT; ++row, ++index)
    {
        btn = new OnyxCheckBox(qApp->translate(GRAY_SCALE_SCOPE,
                GRAY_SCALE_ITEMS[index].title), 0);
        gray_scale_group_.addButton(btn);
        gray_scale_buttons_.push_back(btn);
        if (sys_gray_scale_setting_ == GRAY_SCALE_ITEMS[index].type)
        {
            btn->setFocus();
            btn->setChecked(true);
            set_intial_focus = false;
        }
        connect(btn, SIGNAL(clicked(bool)), this, SLOT(onGrayScaleButtonChanged(bool)),
                Qt::QueuedConnection);
        gray_scale_layout_.addWidget(btn);
    }

    if (set_intial_focus)
    {
        gray_scale_buttons_[0]->setFocus();
    }

    ver_layout_.addLayout(&gray_scale_layout_);
}

void ScreenUpdateDialog::createLayout()
{
    content_widget_.setBackgroundRole(QPalette::Button);
    updateTitle(QApplication::tr("Screen Update"));
    updateTitleIcon(QPixmap(":/images/screen_update_setting.png"));

    // The big layout.
    ver_layout_.setContentsMargins(SPACING, 0, SPACING, 0);
    ver_layout_.addSpacing(5);

    createScreenUpdateLayout();
    if (sys::is166E())
    {
        createGrayScaleLayout();
    }

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

bool ScreenUpdateDialog::event(QEvent *qe)
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

void ScreenUpdateDialog::onReturn()
{
    size_t count = screen_update_buttons_.size();
    for (size_t i = 0; i < count; ++i)
    {
        if (screen_update_buttons_[i]->hasFocus())
        {
            screen_update_buttons_[i]->setChecked(true);
            QApplication::processEvents();
            setScreenUpdateSetting();
            onOkClicked(true);
            break;
        }
    }

    size_t gray_scale_count = gray_scale_buttons_.size();
    for (size_t i = 0; i < gray_scale_count; ++i)
    {
        if (gray_scale_buttons_[i]->hasFocus())
        {
            gray_scale_buttons_[i]->setChecked(true);
            QApplication::processEvents();
            setGrayscaleSetting();
            onOkClicked(true);
            break;
        }
    }
    return;
}

void ScreenUpdateDialog::onScreenUpdateButtonChanged(bool state)
{
    setScreenUpdateSetting();
}

void ScreenUpdateDialog::onGrayScaleButtonChanged(bool state)
{
    setGrayscaleSetting();
}

void ScreenUpdateDialog::onOkClicked(bool)
{
    if (screen_update_setting_ != sys_screen_update_setting_)
    {
        QString type = QString::number(screen_update_setting_);
        conf.setMiscValue(SCREEN_UPDATE_KEY, type);
    }
    if (sys::is166E())
    {
        if (gray_scale_setting_ != sys_gray_scale_setting_)
        {
            QString type = QString::number(gray_scale_setting_);
            conf.setMiscValue(GRAY_SCALE_KEY, type);
        }
    }
    accept();
}

}   // namespace view

}   // namespace explorer
