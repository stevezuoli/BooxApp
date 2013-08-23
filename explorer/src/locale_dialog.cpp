#include "locale_dialog.h"
#include "onyx/ui/languages.h"
#include "onyx/ui/text_layout.h"
#include "onyx/screen/screen_proxy.h"

namespace explorer
{

namespace view
{

/// Always use UTF-8.
struct LocaleItem
{
    const char * text;
    QLocale locale;
};

// Define all supported languages.
static const char* SCOPE = "LocalePage";
static const LocaleItem LANGUAGES[] =
{
    {QT_TRANSLATE_NOOP("LocalePage", "Chinese Simplify"), QLocale(QLocale::Chinese, QLocale::China)},
    {QT_TRANSLATE_NOOP("LocalePage", "Chinese Traditional"), QLocale(QLocale::Chinese, QLocale::Taiwan)},
    {QT_TRANSLATE_NOOP("LocalePage", "English"),  QLocale(QLocale::English, QLocale::UnitedStates)},
    // {QT_TRANSLATE_NOOP("LocalePage", "Finnish"),  QLocale(QLocale::Finnish, QLocale::Finland)},
    // {QT_TRANSLATE_NOOP("LocalePage", "French"),  QLocale(QLocale::French, QLocale::France)},
    {QT_TRANSLATE_NOOP("LocalePage", "German"),  QLocale(QLocale::German, QLocale::Germany)},
    {QT_TRANSLATE_NOOP("LocalePage", "Greek"),  QLocale(QLocale::Greek, QLocale::Greece)},
    {QT_TRANSLATE_NOOP("LocalePage", "Korean"),  QLocale(QLocale::Korean, QLocale::RepublicOfKorea)},
    {QT_TRANSLATE_NOOP("LocalePage", "Japanese"),  QLocale(QLocale::Japanese, QLocale::Japan)},
    {QT_TRANSLATE_NOOP("LocalePage", "Netherlands"),  QLocale(QLocale::Dutch, QLocale::Netherlands)},
    {QT_TRANSLATE_NOOP("LocalePage", "Russian"),  QLocale(QLocale::Russian, QLocale::RussianFederation)},
    {QT_TRANSLATE_NOOP("LocalePage", "Poland"),  QLocale(QLocale::Polish, QLocale::Poland)},
    {QT_TRANSLATE_NOOP("LocalePage", "Spanish"),  QLocale(QLocale::Spanish, QLocale::Spain)},
    {QT_TRANSLATE_NOOP("LocalePage", "Swedish"),  QLocale(QLocale::Swedish, QLocale::Sweden)},
    {QT_TRANSLATE_NOOP("LocalePage", "Denmark"),  QLocale(QLocale::Danish, QLocale::Denmark)},
    {QT_TRANSLATE_NOOP("LocalePage", "Italian"),  QLocale(QLocale::Italian, QLocale::Italy)},
    {QT_TRANSLATE_NOOP("LocalePage", "Hungarian"),  QLocale(QLocale::Hungarian, QLocale::Hungary)},
    {QT_TRANSLATE_NOOP("LocalePage", "Portuguese"),  QLocale(QLocale::Portuguese, QLocale::Portugal)},
    {QT_TRANSLATE_NOOP("LocalePage", "Norwegian"),  QLocale(QLocale::Norwegian, QLocale::Norway)},
    {QT_TRANSLATE_NOOP("LocalePage", "Hebrew"),  QLocale(QLocale::Hebrew, QLocale::Israel)},
};
static const int LANGUAGE_COUNT = sizeof(LANGUAGES) / sizeof(LANGUAGES[0]);

static QString current_locale;

LocaleDialog::LocaleDialog(QWidget *parent,  sys::SystemConfig & ref)
: OnyxDialog(parent)
, conf(ref)
, ok_(QApplication::tr("OK"), 0)
, ver_layout_top_(&content_widget_)
, hor_layout_top_(0)
, ver_layout_left_(0)
, ver_layout_right_(0)
, hor_layout_ok_(0)
{
    setModal(true);
    setFixedSize(500, 600);
    createLayout();
}

LocaleDialog::~LocaleDialog(void)
{
}

int LocaleDialog::exec()
{
    updateTitle(QApplication::tr("Locale Settings"));
    shadows_.show(true);
    show();
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(0, outbounding(parentWidget()), onyx::screen::ScreenProxy::GC, false, onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void LocaleDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void LocaleDialog::keyReleaseEvent(QKeyEvent *ke)
{
    QWidget * wnd = 0;
    // Check the current selected type.
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
    case Qt::Key_Right:
    case Qt::Key_Down:
        wnd = ui::moveFocus(&content_widget_, ke->key());
        if (wnd)
        {
            wnd->setFocus();
        }
        break;
    case Qt::Key_Return:
        onReturn();
        break;
    case Qt::Key_Escape:
        reject();
        break;
    }
}

void LocaleDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        updateTitle(QApplication::tr("Locale"));
        ok_.setText(QApplication::tr("OK"));
    }
    else
    {
        QWidget::changeEvent(event);
    }
}

void LocaleDialog::createLayout()
{
    // Title
    updateTitleIcon(QPixmap(":/images/small/locale.png"));
    content_widget_.setBackgroundRole(QPalette::Button);

    ver_layout_left_.setContentsMargins(SPACING, 0, SPACING, 0);
    ver_layout_right_.setContentsMargins(SPACING, 0, SPACING, 0);

    OnyxCheckBox * btn = 0;
    int index = 0;
    int row = 0;
    for(; row < (LANGUAGE_COUNT + 1)/2; ++row)
    {
        btn = new OnyxCheckBox(qApp->translate(SCOPE, LANGUAGES[index++].text), 0);
        language_buttons_.push_back(btn);
        connect(btn, SIGNAL(clicked(bool)),
                this, SLOT(onLanguageButtonClicked(bool)));

        ver_layout_left_.addWidget(btn);
    }
    for(; row < LANGUAGE_COUNT; ++row)
    {
        btn = new OnyxCheckBox(qApp->translate(SCOPE, LANGUAGES[index++].text), 0);
        language_buttons_.push_back(btn);
        connect(btn, SIGNAL(clicked(bool)),
                this, SLOT(onLanguageButtonClicked(bool)));

        ver_layout_right_.addWidget(btn);
    }
    // The current one.
    QLocale locale = conf.locale();
    current_locale = locale.name();
    qDebug("locale %s", qPrintable(current_locale));
    for(int i = 0; i < LANGUAGE_COUNT; ++i)
    {
        qDebug("locale %s", qPrintable(LANGUAGES[i].locale.name()));

        if (locale.name() == LANGUAGES[i].locale.name())
        {
            language_buttons_[i]->setChecked(true);
            language_buttons_[i]->setFocus();
            break;
        }
    }

    connect(&ok_, SIGNAL(clicked(bool)), this, SLOT(onOkClicked(bool)));
    ok_.useDefaultHeight();
    ok_.setCheckable(false);
    ok_.setFocusPolicy(Qt::TabFocus);
    
    hor_layout_ok_.addStretch(0);
    hor_layout_ok_.addWidget(&ok_);
    hor_layout_top_.addLayout(&ver_layout_left_);
    hor_layout_top_.addLayout(&ver_layout_right_);
    ver_layout_top_.addLayout(&hor_layout_top_);
    ver_layout_top_.addLayout(&hor_layout_ok_);
}

void LocaleDialog::onReturn()
{
    size_t count = language_buttons_.size();
    for(size_t i = 0; i < count; ++i)
    {
        if (language_buttons_[i]->hasFocus())
        {
            language_buttons_[i]->setChecked(true);
            QApplication::processEvents();
            onOkClicked(true);
            return;
        }
    }
}

void LocaleDialog::onLanguageButtonClicked(bool)
{
    size_t count = language_buttons_.size();
    for(size_t i = 0; i < count; ++i)
    {
        if (language_buttons_[i]->isChecked())
        {
            ui::loadTranslator(LANGUAGES[i].locale.name());
            return;
        }
    }
}

void LocaleDialog::onOkClicked(bool)
{
    size_t count = language_buttons_.size();
    for(size_t i = 0; i < count; ++i)
    {
        if (language_buttons_[i]->isChecked())
        {
            conf.setLocale(LANGUAGES[i].locale);
            break;
        }
    }
    accept();
}

void LocaleDialog::onCloseClicked()
{
    ui::loadTranslator(current_locale);
    reject();
}

bool LocaleDialog::event(QEvent * event)
{
    bool ret = OnyxDialog::event(event);
    if (event->type() == QEvent::UpdateRequest && onyx::screen::instance().isUpdateEnabled())
    {
        // onyx::screen::instance().sync(&shadows_.hor_shadow());
        // onyx::screen::instance().sync(&shadows_.ver_shadow());
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW);
    }
    return ret;
}

void LocaleDialog::moveEvent(QMoveEvent *e)
{
    OnyxDialog::moveEvent(e);
}

void LocaleDialog::resizeEvent(QResizeEvent *e)
{
    OnyxDialog::resizeEvent(e);
}

}   // namespace view

}   // namespace explorer

