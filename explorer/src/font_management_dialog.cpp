#include "font_management_dialog.h"
#include "onyx/ui/languages.h"
#include "onyx/ui/text_layout.h"
#include "onyx/screen/screen_proxy.h"

namespace explorer
{

namespace view
{

static const char* SCOPE = "DefaultFontPage";

FontManagementDialog::FontManagementDialog(QWidget *parent,  sys::SystemConfig & ref)
: OnyxDialog(parent)
, conf_(ref)
, ok_(QApplication::tr("OK"), 0)
, ver_layout_top_(&content_widget_)
, hor_layout_top_(0)
, ver_layout_left_(0)
, hor_layout_ok_(0)
{
    setModal(true);
    setFixedSize(400, 550);
    createLayout();
}

FontManagementDialog::~FontManagementDialog(void)
{
}

int FontManagementDialog::exec()
{
    updateTitle(QApplication::tr("Default Font"));
    shadows_.show(true);
    show();
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(0, outbounding(parentWidget()), onyx::screen::ScreenProxy::GC, false, onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void FontManagementDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void FontManagementDialog::keyReleaseEvent(QKeyEvent *ke)
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

void FontManagementDialog::changeEvent(QEvent *event)
{
   // if (event->type() == QEvent::FontManagementChange)
   // {
     //   updateTitle(QApplication::tr("FontManagement"));
       // ok_.setText(QApplication::tr("OK"));
 //   }
   // else
    {
        QWidget::changeEvent(event);
    }
}

void FontManagementDialog::createLayout()
{
    // Title
    updateTitleIcon(QPixmap(":/images/small/font_family.png"));
    content_widget_.setBackgroundRole(QPalette::Button);

    ver_layout_left_.setContentsMargins(SPACING, 0, SPACING, 0);
    //ver_layout_right_.setContentsMargins(SPACING, 0, SPACING, 0);

    OnyxCheckBox * btn = 0;
    int index = 0;
    QFontDatabase fonts;
    font_family_list_ = fonts.families();
    for(int row = 0; row < font_family_list_.size(); ++row)
    {
        btn = new OnyxCheckBox(QApplication::translate(SCOPE, font_family_list_[index++].toAscii().constData()), 0);
        FontManagement_buttons_.push_back(btn);
        connect(btn, SIGNAL(clicked(bool)),
                this, SLOT(onFontManagementButtonClicked(bool)));
        ver_layout_left_.addWidget(btn);
    }
    // The current one.
    QString currFont = conf_.defaultFontFamily();
    for(int i = 0; i < font_family_list_.size(); ++i)
    {
        if (currFont == font_family_list_[i])
        {
            FontManagement_buttons_[i]->setChecked(true);
            FontManagement_buttons_[i]->setFocus();
            break;
        }
    }
    text_.setMaximumHeight(80);
    text_.setFocusPolicy(Qt::NoFocus);
    text_.setReadOnly(true);
    text_.setAlignment(Qt::AlignCenter);
    text_.setFontPointSize(20);
    text_.setFontFamily(currFont);
    text_.append("The quick brown fox jumps over the lazy dog.");
    hor_layout_sample_.addWidget(&text_);

    connect(&ok_, SIGNAL(clicked(bool)), this, SLOT(onOkClicked(bool)));
    ok_.useDefaultHeight();
    ok_.setCheckable(false);
    ok_.setFocusPolicy(Qt::TabFocus);
    
    hor_layout_ok_.addStretch(0);
    hor_layout_ok_.addWidget(&ok_);
    hor_layout_top_.addLayout(&ver_layout_left_);
   // hor_layout_top_.addLayout(&ver_layout_right_);
    ver_layout_top_.addLayout(&hor_layout_top_);
    ver_layout_top_.addLayout(&hor_layout_sample_);
    ver_layout_top_.addLayout(&hor_layout_ok_);
}

void FontManagementDialog::onReturn()
{
    size_t count = FontManagement_buttons_.size();
    for(size_t i = 0; i < count; ++i)
    {
        if (FontManagement_buttons_[i]->hasFocus())
        {
            FontManagement_buttons_[i]->setChecked(true);
            QApplication::processEvents();
            onOkClicked(true);
            return;
        }
    }
}

void FontManagementDialog::onFontManagementButtonClicked(bool)
{
    size_t count = FontManagement_buttons_.size();
    for(size_t i = 0; i < count; ++i)
    {
        if (FontManagement_buttons_[i]->isChecked())
        {
            QApplication::setFont(QFont(font_family_list_[i]));
            text_.clear();
            text_.setReadOnly(true);
            text_.setAlignment(Qt::AlignCenter);
            text_.setFontPointSize(20);
            text_.setFontFamily(font_family_list_[i]);
            text_.append("The quick brown fox jumps over the lazy dog.");
            return;
        }
    }
}

void FontManagementDialog::onOkClicked(bool)
{
    size_t count = FontManagement_buttons_.size();
    for(size_t i = 0; i < count; ++i)
    {
        if (FontManagement_buttons_[i]->isChecked())
        {
            conf_.setDefaultFontFamily(font_family_list_[i]);
            break;
        }
    }
    accept();
}

void FontManagementDialog::onCloseClicked()
{
    reject();
}

bool FontManagementDialog::event(QEvent * event)
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

void FontManagementDialog::moveEvent(QMoveEvent *e)
{
    OnyxDialog::moveEvent(e);
}

void FontManagementDialog::resizeEvent(QResizeEvent *e)
{
    OnyxDialog::resizeEvent(e);
}

}   // namespace view

}   // namespace explorer

