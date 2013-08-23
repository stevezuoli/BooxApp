#include "waveform_dialog.h"
#include "onyx/ui/languages.h"
#include "onyx/ui/text_layout.h"
#include "onyx/screen/screen_proxy.h"

namespace explorer
{

namespace view
{

struct WaveformItem
{
    const char * text;
    int gray;
};

// Define all supported languages.
static const char* SCOPE = "waveform";
static const WaveformItem WAVEFORMS[] =
{
    {QT_TRANSLATE_NOOP("waveform", "8 Grayscale"), 8},
    {QT_TRANSLATE_NOOP("waveform", "16 Grayscale"), 16},
};
static const int WAVEFORM_COUNT = sizeof(WAVEFORMS) / sizeof(WAVEFORMS[0]);


WaveformDialog::WaveformDialog(QWidget *parent)
: OnyxDialog(parent)
, ok_(QApplication::tr("OK"), 0)
, ver_layout_(&content_widget_)
, hor_layout_(0)
{
    setModal(true);
    setFixedSize(400, 550);
    createLayout();
}

WaveformDialog::~WaveformDialog(void)
{
}

int WaveformDialog::exec()
{
    updateTitle(QApplication::tr("Waveform Settings"));
    shadows_.show(true);
    show();
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(0, outbounding(parentWidget()), onyx::screen::ScreenProxy::GC, false, onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void WaveformDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void WaveformDialog::keyReleaseEvent(QKeyEvent *ke)
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

void WaveformDialog::createLayout()
{
    // Title
    updateTitleIcon(QPixmap(":/images/small/locale.png"));
    content_widget_.setBackgroundRole(QPalette::Button);

    ver_layout_.setContentsMargins(SPACING, 0, SPACING, 0);

    OnyxCheckBox * btn = 0;
    int index = 0;
    for(int row = 0; row < WAVEFORM_COUNT; ++row)
    {
        btn = new OnyxCheckBox(qApp->translate(SCOPE, WAVEFORMS[index++].text), 0);
        buttons_.push_back(btn);
        connect(btn, SIGNAL(clicked(bool)), this, SLOT(onButtonClicked(bool)));
        ver_layout_.addWidget(btn);
    }

    // The current one.
    int color = sys::SysStatus::instance().grayScale();
    for(int i = 0; i < WAVEFORM_COUNT; ++i)
    {
        if (color == WAVEFORMS[i].gray)
        {
            buttons_[i]->setChecked(true);
            buttons_[i]->setFocus();
            break;
        }
    }

    connect(&ok_, SIGNAL(clicked(bool)), this, SLOT(onOkClicked(bool)));
    ok_.useDefaultHeight();
    ok_.setCheckable(false);
    ok_.setFocusPolicy(Qt::TabFocus);
    hor_layout_.addStretch(0);
    hor_layout_.addWidget(&ok_);
    ver_layout_.addStretch(0);
    ver_layout_.addLayout(&hor_layout_);
}

void WaveformDialog::onReturn()
{
    size_t count = buttons_.size();
    for(size_t i = 0; i < count; ++i)
    {
        if (buttons_[i]->hasFocus())
        {
            buttons_[i]->setChecked(true);
            QApplication::processEvents();
            onOkClicked(true);
            return;
        }
    }
}

void WaveformDialog::onButtonClicked(bool)
{
}

void WaveformDialog::onOkClicked(bool)
{
    size_t count = buttons_.size();
    for(size_t i = 0; i < count; ++i)
    {
        if (buttons_[i]->isChecked())
        {
            sys::SysStatus::instance().setGrayScale(WAVEFORMS[i].gray);
            break;
        }
    }
    accept();
}

void WaveformDialog::onCloseClicked()
{
    reject();
}

bool WaveformDialog::event(QEvent * event)
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

void WaveformDialog::moveEvent(QMoveEvent *e)
{
    OnyxDialog::moveEvent(e);
}

void WaveformDialog::resizeEvent(QResizeEvent *e)
{
    OnyxDialog::resizeEvent(e);
}

}   // namespace view

}   // namespace explorer

